#!/usr/bin/env python3
"""Fetch parts from Digi-Key API and convert them to ECIM-compatible JSON.

This script uses Digi-Key OAuth2 client credentials to obtain an access token,
queries part data, then normalizes each item into a structure that mirrors the
ECIM component model (base fields + type-specific properties).
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Dict, Iterable, List, Optional
from urllib import error, parse, request


DIGIKEY_TOKEN_URL = "https://api.digikey.com/v1/oauth2/token"
DIGIKEY_SEARCH_URL = "https://api.digikey.com/products/v4/search/keyword"
DEFAULT_TIMEOUT_SECONDS = 30
ECIM_IMPORT_SCHEMA_ID = "ecim.import.v1"
ECIM_IMPORT_SCHEMA_VERSION = "1.0.0"


class DigiKeyError(RuntimeError):
    """Raised for Digi-Key API errors."""


@dataclass
class DigiKeyCredentials:
    client_id: str
    client_secret: str


class DigiKeyClient:
    def __init__(
        self,
        credentials: DigiKeyCredentials,
        token_url: str = DIGIKEY_TOKEN_URL,
        search_url: str = DIGIKEY_SEARCH_URL,
        timeout: int = DEFAULT_TIMEOUT_SECONDS,
    ) -> None:
        self._credentials = credentials
        self._token_url = token_url
        self._search_url = search_url
        self._timeout = timeout
        self._access_token: Optional[str] = None
        self._token_expiry_epoch: float = 0.0

    def _post_form(self, url: str, fields: Dict[str, str], headers: Dict[str, str]) -> Dict[str, Any]:
        data = parse.urlencode(fields).encode("utf-8")
        req = request.Request(url, data=data, headers=headers, method="POST")
        return self._json_request(req)

    def _post_json(self, url: str, payload: Dict[str, Any], headers: Dict[str, str]) -> Dict[str, Any]:
        data = json.dumps(payload).encode("utf-8")
        req = request.Request(url, data=data, headers=headers, method="POST")
        return self._json_request(req)

    def _json_request(self, req: request.Request) -> Dict[str, Any]:
        try:
            with request.urlopen(req, timeout=self._timeout) as response:
                body = response.read().decode("utf-8")
                if not body:
                    return {}
                return json.loads(body)
        except error.HTTPError as exc:
            detail = exc.read().decode("utf-8", errors="replace")
            raise DigiKeyError(f"HTTP {exc.code} for {req.full_url}: {detail}") from exc
        except error.URLError as exc:
            raise DigiKeyError(f"Network error for {req.full_url}: {exc.reason}") from exc
        except json.JSONDecodeError as exc:
            raise DigiKeyError(f"Invalid JSON response from {req.full_url}: {exc}") from exc

    def _ensure_token(self) -> str:
        if self._access_token and time.time() < self._token_expiry_epoch:
            return self._access_token

        fields = {
            "client_id": self._credentials.client_id,
            "client_secret": self._credentials.client_secret,
            "grant_type": "client_credentials",
        }
        headers = {"Content-Type": "application/x-www-form-urlencoded"}
        payload = self._post_form(self._token_url, fields, headers)

        token = payload.get("access_token")
        if not token:
            raise DigiKeyError("Token response missing 'access_token'")

        expires_in = int(payload.get("expires_in", 1800))
        # Renew a little earlier to avoid edge-case expiration mid-request.
        self._token_expiry_epoch = time.time() + max(60, expires_in - 60)
        self._access_token = token
        return token

    def keyword_search(self, keyword: str, limit: int = 25) -> List[Dict[str, Any]]:
        token = self._ensure_token()
        headers = {
            "Authorization": f"Bearer {token}",
            "X-DIGIKEY-Client-Id": self._credentials.client_id,
            "Content-Type": "application/json",
            "Accept": "application/json",
        }
        payload = {
            "Keywords": keyword,
            "RecordCount": max(1, min(limit, 50)),
            "RecordStartPosition": 0,
        }
        result = self._post_json(self._search_url, payload, headers)

        products = _first_list(result, ["Products", "products", "Items", "items"])
        if not products:
            return []
        return [p for p in products if isinstance(p, dict)]


def _first_value(data: Dict[str, Any], keys: Iterable[str], default: Any = None) -> Any:
    for key in keys:
        if key in data and data[key] is not None:
            return data[key]
    return default


def _first_list(data: Dict[str, Any], keys: Iterable[str]) -> List[Any]:
    value = _first_value(data, keys, default=[])
    return value if isinstance(value, list) else []


def _text(value: Any) -> str:
    if value is None:
        return ""
    if isinstance(value, (dict, list)):
        return json.dumps(value, ensure_ascii=False)
    return str(value).strip()


def _to_float(value: Any, default: float = 0.0) -> float:
    if value is None:
        return default
    if isinstance(value, (int, float)):
        return float(value)
    text = _text(value)
    if not text:
        return default

    normalized = text.replace(",", "").replace("\u00b5", "u").replace("\u03bc", "u")
    match = re.search(
        r"(-?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)\s*([pnumkKMGT]?)\s*[A-Za-z%\u03a9\u03c9]*",
        normalized,
    )
    if not match:
        return default

    number = match.group(1)
    prefix = match.group(2)
    multiplier = {
        "p": 1e-12,
        "n": 1e-9,
        "u": 1e-6,
        "m": 1e-3,
        "k": 1e3,
        "K": 1e3,
        "M": 1e6,
        "G": 1e9,
        "T": 1e12,
    }.get(prefix, 1.0)

    try:
        return float(number) * multiplier
    except ValueError:
        return default


def _to_int(value: Any, default: int = 0) -> int:
    return int(round(_to_float(value, default=default)))


def _normalize_key(name: str) -> str:
    return re.sub(r"[^a-z0-9]+", "", name.lower())


def _flatten_parameters(product: Dict[str, Any]) -> Dict[str, Any]:
    out: Dict[str, Any] = {}
    params = _first_list(product, ["Parameters", "parameters", "ParametricData", "parametricData"])
    for item in params:
        if not isinstance(item, dict):
            continue
        raw_name = _first_value(item, ["ParameterText", "Name", "parameterText", "name"], default="")
        if not raw_name:
            continue
        value = _first_value(
            item,
            ["ValueText", "Value", "valueText", "value", "DisplayValue", "displayValue"],
            default="",
        )
        out[_normalize_key(_text(raw_name))] = value
    return out


def _guess_type(product: Dict[str, Any], params: Dict[str, Any]) -> str:
    category = _text(_first_value(product, ["Category", "category", "Family", "family"], default=""))
    family_name = ""
    if isinstance(_first_value(product, ["Family", "family"], default={}), dict):
        family_name = _text(
            _first_value(_first_value(product, ["Family", "family"], default={}), ["Name", "name"], default="")
        )
    text_blob = f"{category} {family_name} {_text(_first_value(product, ['Description', 'description'], default=''))}".lower()

    if any(k in text_blob for k in ["resistor", "resistance"]):
        return "Resistor"
    if any(k in text_blob for k in ["capacitor", "capacitance"]):
        return "Capacitor"
    if any(k in text_blob for k in ["inductor", "inductance"]):
        return "Inductor"
    if "diode" in text_blob:
        return "Diode"
    if any(k in text_blob for k in ["mosfet", "fet", "field effect"]):
        return "FETransistor"
    if any(k in text_blob for k in ["bjt", "bipolar transistor", "transistor (bjt)"]):
        return "BJTransistor"
    if any(k in text_blob for k in ["integrated circuit", "ic", "microcontroller", "amplifier"]):
        return "IntegratedCircuit"

    # Parameter fallback when category text is sparse.
    if "resistance" in params:
        return "Resistor"
    if "capacitance" in params:
        return "Capacitor"
    if "inductance" in params:
        return "Inductor"
    if "forwardvoltage" in params:
        return "Diode"
    if "thresholdvoltage" in params:
        return "FETransistor"
    if "dccurrentgainhfe" in params or "currentgain" in params:
        return "BJTransistor"

    return "IntegratedCircuit"


def _diode_type(description: str) -> str:
    text = description.lower()
    if "schottky" in text:
        return "Schottky"
    if "zener" in text:
        return "Zener"
    if "led" in text or "light emitting" in text:
        return "LED"
    return "Regular"


def _capacitor_type(description: str) -> str:
    text = description.lower()
    mapping = [
        ("aluminum polymer", "AluminumPolymer"),
        ("electrolytic", "AluminumElectrolytic"),
        ("ceramic", "Ceramic"),
        ("supercapacitor", "ElectricDoubleLayer"),
        ("double layer", "ElectricDoubleLayer"),
        ("film", "Film"),
        ("mica", "Mica"),
        ("ptfe", "PTFE"),
        ("niobium", "NiobiumOxide"),
        ("silicon", "Silicon"),
        ("tantalum", "Tantalum"),
        ("thin film", "ThinFilm"),
        ("ac motor", "ACMotor"),
        ("lithium", "LithiumHybrid"),
    ]
    for needle, mapped in mapping:
        if needle in text:
            return mapped
    return "Ceramic"


def _rating_block(params: Dict[str, Any]) -> Dict[str, float]:
    return {
        "voltage": _to_float(
            _first_value(
                params,
                ["voltagerated", "voltage", "voltagedc", "ratedvoltage"],
                default=0,
            )
        ),
        "current": _to_float(
            _first_value(params, ["current", "currentrated", "ratedcurrent"], default=0)
        ),
        "power": _to_float(
            _first_value(params, ["powerrating", "powerwatts", "power", "ratedpower"], default=0)
        ),
        "tolerance": _to_float(
            _first_value(params, ["tolerance", "resistancetolerance", "capacitancetolerance"], default=0)
        ),
    }


def _type_properties(component_type: str, params: Dict[str, Any], description: str) -> Dict[str, Any]:
    if component_type == "Resistor":
        return {
            "resistance": _to_float(_first_value(params, ["resistance", "resistanceohms"], default=0)),
            "toleranceBand": _to_float(_first_value(params, ["tolerance", "resistancetolerance"], default=0)),
        }
    if component_type == "Capacitor":
        return {
            "capacitance": _to_float(_first_value(params, ["capacitance"], default=0)),
            "capacitorType": _capacitor_type(description),
        }
    if component_type == "Inductor":
        return {
            "inductance": _to_float(_first_value(params, ["inductance"], default=0)),
        }
    if component_type == "Diode":
        return {
            "forwardVoltage": _to_float(_first_value(params, ["forwardvoltage", "voltageforwardvfmaxif"], default=0)),
            "diodeType": _diode_type(description),
        }
    if component_type == "BJTransistor":
        return {
            "gain": _to_float(_first_value(params, ["dccurrentgainhfe", "gain", "currentgain"], default=0)),
        }
    if component_type == "FETransistor":
        return {
            "thresholdVoltage": _to_float(
                _first_value(params, ["gatesourcethresholdvoltagevgs th maxid", "thresholdvoltage"], default=0)
            ),
        }

    # IntegratedCircuit
    return {
        "pinCount": max(1, _to_int(_first_value(params, ["numberofpins", "pins", "pincount"], default=1))),
        "width": _to_float(_first_value(params, ["width", "packagecase"], default=0)),
        "height": _to_float(_first_value(params, ["height"], default=0)),
        "length": _to_float(_first_value(params, ["length", "size dimension"], default=0)),
    }


def normalize_product(product: Dict[str, Any], default_quantity: int = 1) -> Dict[str, Any]:
    params = _flatten_parameters(product)

    part_number = _text(
        _first_value(
            product,
            [
                "ManufacturerPartNumber",
                "manufacturerPartNumber",
                "DigiKeyPartNumber",
                "digiKeyPartNumber",
                "PartNumber",
                "partNumber",
            ],
            default="",
        )
    )
    manufacturer = _text(
        _first_value(
            product,
            ["Manufacturer", "manufacturer", "ManufacturerName", "manufacturerName"],
            default="Unknown",
        )
    )
    if isinstance(_first_value(product, ["Manufacturer", "manufacturer"], default=None), dict):
        manufacturer_dict = _first_value(product, ["Manufacturer", "manufacturer"], default={})
        manufacturer = _text(_first_value(manufacturer_dict, ["Name", "name"], default=manufacturer))

    description = _text(
        _first_value(
            product,
            ["Description", "description", "ProductDescription", "productDescription"],
            default="",
        )
    )
    if isinstance(_first_value(product, ["Description", "description"], default=None), dict):
        desc_dict = _first_value(product, ["Description", "description"], default={})
        description = _text(_first_value(desc_dict, ["ProductDescription", "DetailedDescription", "description"], default=""))

    component_type = _guess_type(product, params)
    name = _text(_first_value(product, ["ProductName", "productName", "Name", "name"], default="")) or part_number

    ecim_item = {
        "type": component_type,
        "name": name or "Unnamed",
        "manufacturer": manufacturer or "Unknown",
        "partNumber": part_number,
        "description": description,
        "quantity": max(0, int(default_quantity)),
        "rating": _rating_block(params),
        "properties": _type_properties(component_type, params, description),
        # Keep original payload for troubleshooting and future parser refinement.
        "source": {
            "provider": "DigiKey",
            "raw": product,
        },
    }
    return ecim_item


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Fetch Digi-Key parts and convert to ECIM-compatible JSON."
    )
    parser.add_argument(
        "--keywords",
        nargs="+",
        help="One or more search terms (e.g. NE555 1k resistor IRLZ44N).",
    )
    parser.add_argument(
        "--input-file",
        help="Path to JSON request file (bridge mode).",
    )
    parser.add_argument(
        "--output-file",
        help="Path to JSON response file (bridge mode).",
    )
    parser.add_argument(
        "--output",
        default="ecim_parts.json",
        help="Output JSON file path (default: ecim_parts.json).",
    )
    parser.add_argument(
        "--limit-per-keyword",
        type=int,
        default=None,
        help="Maximum Digi-Key records per keyword (1-50, default: 10).",
    )
    parser.add_argument(
        "--quantity",
        type=int,
        default=None,
        help="Default quantity assigned to each exported part (default: 1).",
    )
    parser.add_argument(
        "--client-id",
        default=None,
        help="Digi-Key API client ID (or set DIGIKEY_CLIENT_ID).",
    )
    parser.add_argument(
        "--client-secret",
        default=None,
        help="Digi-Key API client secret (or set DIGIKEY_CLIENT_SECRET).",
    )
    parser.add_argument(
        "--token-url",
        default=None,
        help="OAuth token URL override.",
    )
    parser.add_argument(
        "--search-url",
        default=None,
        help="Search API URL override.",
    )
    parser.add_argument(
        "--pretty",
        action="store_true",
        default=None,
        help="Pretty-print JSON output.",
    )
    return parser


def _load_bridge_request(input_file: Optional[str]) -> Dict[str, Any]:
    if not input_file:
        return {}

    try:
        raw_data = Path(input_file).read_text(encoding="utf-8")
        parsed = json.loads(raw_data)
    except OSError as exc:
        raise DigiKeyError(f"Could not read input file '{input_file}': {exc}") from exc
    except json.JSONDecodeError as exc:
        raise DigiKeyError(f"Input file is not valid JSON: {exc}") from exc

    if not isinstance(parsed, dict):
        raise DigiKeyError("Input file JSON must be an object")
    return parsed


def _list_from_payload(value: Any) -> List[str]:
    if value is None:
        return []
    if isinstance(value, str):
        return [value]
    if isinstance(value, list):
        return [str(item) for item in value if str(item).strip()]
    return []


def _pick_value(cli_value: Any, payload: Dict[str, Any], keys: List[str], default: Any = None) -> Any:
    if cli_value not in (None, ""):
        return cli_value
    return _first_value(payload, keys, default)


def _to_bool(value: Any, default: bool = False) -> bool:
    if value is None:
        return default
    if isinstance(value, bool):
        return value
    if isinstance(value, (int, float)):
        return value != 0
    text = str(value).strip().lower()
    if text in {"1", "true", "yes", "y", "on"}:
        return True
    if text in {"0", "false", "no", "n", "off"}:
        return False
    return default


def main(argv: Optional[List[str]] = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)

    bridge_payload = _load_bridge_request(args.input_file)
    keywords = _list_from_payload(args.keywords)
    if not keywords:
        keywords = _list_from_payload(_first_value(bridge_payload, ["keywords", "searchTerms"], default=[]))

    if not keywords:
        parser.error("Missing keywords. Use --keywords or provide them in --input-file JSON.")

    limit_per_keyword = int(
        _pick_value(args.limit_per_keyword, bridge_payload, ["limitPerKeyword", "limit_per_keyword"], default=10)
    )
    quantity = int(_pick_value(args.quantity, bridge_payload, ["quantity"], default=1))
    client_id = _pick_value(
        args.client_id,
        bridge_payload,
        ["clientId", "client_id"],
        default=os.getenv("DIGIKEY_CLIENT_ID", ""),
    )
    client_secret = _pick_value(
        args.client_secret,
        bridge_payload,
        ["clientSecret", "client_secret"],
        default=os.getenv("DIGIKEY_CLIENT_SECRET", ""),
    )
    token_url = _pick_value(args.token_url, bridge_payload, ["tokenUrl", "token_url"], default=DIGIKEY_TOKEN_URL)
    search_url = _pick_value(args.search_url, bridge_payload, ["searchUrl", "search_url"], default=DIGIKEY_SEARCH_URL)
    pretty = _to_bool(_pick_value(args.pretty, bridge_payload, ["pretty"], default=False), default=False)
    output_path = _pick_value(args.output_file, bridge_payload, ["outputFile", "output_file"], default="")
    if not output_path:
        output_path = args.output

    if not client_id or not client_secret:
        parser.error("Missing Digi-Key credentials. Set --client-id/--client-secret or env vars.")

    credentials = DigiKeyCredentials(client_id=client_id, client_secret=client_secret)
    client = DigiKeyClient(
        credentials=credentials,
        token_url=token_url,
        search_url=search_url,
    )

    all_products: List[Dict[str, Any]] = []
    for keyword in keywords:
        items = client.keyword_search(keyword=keyword, limit=limit_per_keyword)
        all_products.extend(items)

    # De-duplicate by part number while preserving first seen order.
    unique: Dict[str, Dict[str, Any]] = {}
    for product in all_products:
        part_number = _text(
            _first_value(
                product,
                ["ManufacturerPartNumber", "manufacturerPartNumber", "DigiKeyPartNumber", "digiKeyPartNumber", "PartNumber", "partNumber"],
                default="",
            )
        )
        key = part_number or json.dumps(product, sort_keys=True)
        if key not in unique:
            unique[key] = product

    normalized = [normalize_product(p, default_quantity=quantity) for p in unique.values()]

    output_payload = {
        "schema": ECIM_IMPORT_SCHEMA_ID,
        "schemaVersion": ECIM_IMPORT_SCHEMA_VERSION,
        "generatedBy": "scripts/digikey_to_ecim.py",
        "items": normalized,
    }

    with open(output_path, "w", encoding="utf-8") as outfile:
        if pretty:
            json.dump(output_payload, outfile, indent=2, ensure_ascii=False)
            outfile.write("\n")
        else:
            json.dump(output_payload, outfile, separators=(",", ":"), ensure_ascii=False)

    print(f"Exported {len(normalized)} component(s) to {output_path}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except DigiKeyError as exc:
        print(f"Digi-Key request failed: {exc}", file=sys.stderr)
        raise SystemExit(1)
