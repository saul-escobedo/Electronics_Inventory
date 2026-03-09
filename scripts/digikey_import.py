#!/usr/bin/env python3
"""
digikey_import.py

Retrieves part information from Digi-Key using their v4 REST API and converts
it into a structured JSON format compatible with the ECIM application.

Usage:
    python digikey_import.py part  <PART_NUMBER>
    python digikey_import.py search <KEYWORD> [--limit N]

Credentials are read from environment variables:
    DIGIKEY_CLIENT_ID      - Your Digi-Key API client ID   (required)
    DIGIKEY_CLIENT_SECRET  - Your Digi-Key API client secret (required)
    DIGIKEY_SANDBOX        - Set to "1" to target the sandbox API (optional)

Output is a JSON object (single part) or JSON array (keyword search) whose
fields map directly onto the ECIM ElectronicComponent data model.
"""

import argparse
import json
import os
import re
import sys

import requests

# ─── API constants ────────────────────────────────────────────────────────────

_DIGIKEY_API_BASE     = "https://api.digikey.com"
_DIGIKEY_SANDBOX_BASE = "https://sandbox-api.digikey.com"
_TOKEN_PATH           = "/v1/oauth2/token"
_DETAILS_PATH         = "/products/v4/search/{part_number}/productdetails"
_KEYWORD_PATH         = "/products/v4/search/keyword"

# ─── SI / unit parsing ────────────────────────────────────────────────────────

_SI_PREFIX = {
    "p": 1e-12,
    "n": 1e-9,
    "µ": 1e-6,
    "u": 1e-6,
    "m": 1e-3,
    "k": 1e3,
    "M": 1e6,
    "G": 1e9,
}


def parse_value(text: str) -> float:
    """Parse a numeric string that may carry an SI prefix and/or unit suffix.

    Converts the result to the corresponding base SI unit value.

    Examples::

        parse_value("1 kOhms")  →  1000.0
        parse_value("100 nF")   →  1e-7
        parse_value("47 µH")    →  4.7e-5
        parse_value("5%")       →  0.05
        parse_value("3.3 V")    →  3.3
    """
    if not text:
        return 0.0

    text = text.strip()

    # Handle bare percentages.
    if text.endswith("%"):
        try:
            return float(text[:-1].strip()) / 100.0
        except ValueError:
            return 0.0

    # Match: optional sign, number, optional SI prefix, optional unit letters.
    m = re.match(
        r"^([+-]?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)\s*([pnµumkMG]?)\s*[a-zA-ZΩ°]*",
        text,
    )
    if not m:
        return 0.0

    number     = float(m.group(1))
    multiplier = _SI_PREFIX.get(m.group(2), 1.0)
    return number * multiplier


# ─── Digi-Key API client ──────────────────────────────────────────────────────


class DigiKeyClient:
    """Thin wrapper around the Digi-Key v4 REST API."""

    def __init__(self, client_id: str, client_secret: str, sandbox: bool = False) -> None:
        self._client_id     = client_id
        self._client_secret = client_secret
        self._base_url      = _DIGIKEY_SANDBOX_BASE if sandbox else _DIGIKEY_API_BASE
        self._token: str | None = None

    # ── Authentication ────────────────────────────────────────────────────────

    def _fetch_token(self) -> str:
        """Obtain an OAuth2 access token via the client-credentials flow."""
        url  = self._base_url + _TOKEN_PATH
        data = {
            "client_id"    : self._client_id,
            "client_secret": self._client_secret,
            "grant_type"   : "client_credentials",
        }
        resp = requests.post(url, data=data, timeout=30)
        resp.raise_for_status()
        payload = resp.json()
        if "access_token" not in payload:
            raise RuntimeError(f"Failed to obtain access token: {payload}")
        return payload["access_token"]

    @property
    def _access_token(self) -> str:
        if self._token is None:
            self._token = self._fetch_token()
        return self._token

    def _auth_headers(self) -> dict:
        return {
            "Authorization"      : f"Bearer {self._access_token}",
            "X-DIGIKEY-Client-Id": self._client_id,
            "Content-Type"       : "application/json",
        }

    # ── Public API calls ──────────────────────────────────────────────────────

    def get_part_details(self, part_number: str) -> dict:
        """Retrieve full details for a manufacturer or Digi-Key part number."""
        url    = self._base_url + _DETAILS_PATH.format(part_number=part_number)
        params = {
            "includes": (
                "DigiKeyPartNumber,ManufacturerPartNumber,ProductDescription,"
                "Manufacturer,Parameters,Category,QuantityAvailable,UnitPrice"
            )
        }
        resp = requests.get(url, headers=self._auth_headers(), params=params, timeout=30)
        resp.raise_for_status()
        return resp.json()

    def search_keyword(self, keyword: str, limit: int = 10) -> dict:
        """Search Digi-Key for parts that match *keyword*."""
        url  = self._base_url + _KEYWORD_PATH
        body = {
            "Keywords"      : keyword,
            "RecordCount"   : limit,
            "RecordStartPos": 0,
            "Filters"       : {},
            "Sort"          : {
                "SortOption"     : "SortByUnitPrice",
                "Direction"      : "Ascending",
                "SortParameterId": 0,
            },
            "RequestedQuantity"          : 1,
            "SearchOptions"              : [],
            "ExcludeMarketPlaceProducts" : False,
        }
        resp = requests.post(url, headers=self._auth_headers(), json=body, timeout=30)
        resp.raise_for_status()
        return resp.json()


# ─── ECIM component-type detection ───────────────────────────────────────────

# Ordered list of (keywords, ecim_type).  The first match wins.
_CATEGORY_TO_TYPE: list[tuple[list[str], str]] = [
    (["resistor"],                                                   "Resistor"),
    (["capacitor"],                                                  "Capacitor"),
    (["inductor", "coil", "choke", "ferrite bead"],                  "Inductor"),
    (["led", "zener", "schottky", "diode", "rectifier", "varistor"], "Diode"),
    (["bjt", "bipolar", "transistor - bipolar"],                     "BJTransistor"),
    (["mosfet", "jfet", "fet", "transistor - fet"],                  "FETransistor"),
]
# Default fallback
_IC_TYPE = "IntegratedCircuit"

# Capacitor sub-type keyword map (longest / most-specific first)
_CAP_TYPE_MAP: list[tuple[str, str]] = [
    ("aluminum polymer",    "AluminumPolymer"),
    ("aluminum electrolytic", "AluminumElectrolytic"),
    ("supercapacitor",      "ElectricDoubleLayer"),
    ("double layer",        "ElectricDoubleLayer"),
    ("thin film",           "ThinFilm"),
    ("ac motor",            "ACMotor"),
    ("lithium",             "LithiumHybrid"),
    ("niobium",             "NiobiumOxide"),
    ("tantalum",            "Tantalum"),
    ("silicon",             "Silicon"),
    ("ceramic",             "Ceramic"),
    ("film",                "Film"),
    ("mica",                "Mica"),
    ("ptfe",                "PTFE"),
]

# Diode sub-type keyword map
_DIODE_TYPE_MAP: list[tuple[str, str]] = [
    ("zener",    "Zener"),
    ("schottky", "Schottky"),
    ("led",      "LED"),
]


# ─── Mapping helpers ──────────────────────────────────────────────────────────


def _detect_type(category_name: str, description: str) -> str:
    """Determine the ECIM component type from Digi-Key category and description."""
    text = (category_name + " " + description).lower()
    for keywords, ecim_type in _CATEGORY_TO_TYPE:
        if any(kw in text for kw in keywords):
            return ecim_type
    return _IC_TYPE


def _params_dict(parameters: list) -> dict:
    """Convert the Digi-Key Parameters list to a {name: value} mapping."""
    return {p["Parameter"]: p["Value"] for p in (parameters or [])}


def _first_match(params: dict, *substrings: str) -> str:
    """Return the value of the first parameter whose name contains any substring."""
    for key, val in params.items():
        key_lower = key.lower()
        if any(s in key_lower for s in substrings):
            return val
    return "0"


def _map_resistor(params: dict, base: dict) -> dict:
    tolerance = parse_value(params.get("Tolerance", "0"))
    base["ratings"]["power"]     = parse_value(params.get("Power (Watts)", "0"))
    base["ratings"]["voltage"]   = parse_value(params.get("Voltage Rating - DC", "0"))
    base["ratings"]["tolerance"] = tolerance
    base["properties"] = {
        "resistance"    : parse_value(params.get("Resistance", "0")),
        "tolerance_band": tolerance,
    }
    return base


def _map_capacitor(params: dict, category_name: str, base: dict) -> dict:
    cap_type_raw = (params.get("Type", "") + " " + category_name).lower()
    cap_type = "Ceramic"
    for keyword, ecim_cap_type in _CAP_TYPE_MAP:
        if keyword in cap_type_raw:
            cap_type = ecim_cap_type
            break

    tolerance = parse_value(params.get("Tolerance", "0"))
    base["ratings"]["voltage"]   = parse_value(params.get("Voltage - Rated", "0"))
    base["ratings"]["tolerance"] = tolerance
    base["properties"] = {
        "capacitance"   : parse_value(params.get("Capacitance", "0")),
        "capacitor_type": cap_type,
    }
    return base


def _map_inductor(params: dict, base: dict) -> dict:
    tolerance = parse_value(params.get("Tolerance", "0"))
    base["ratings"]["current"]   = parse_value(_first_match(params, "current rating"))
    base["ratings"]["tolerance"] = tolerance
    base["properties"] = {
        "inductance": parse_value(params.get("Inductance", "0")),
    }
    return base


def _map_diode(params: dict, category_name: str, description: str, base: dict) -> dict:
    text = (category_name + " " + description).lower()

    diode_type = "Regular"
    for keyword, dtype in _DIODE_TYPE_MAP:
        if keyword in text:
            diode_type = dtype
            break

    # Forward voltage may appear under several parameter names.
    fwd_val = _first_match(params, "forward", "vf", "vz")
    base["ratings"]["voltage"] = parse_value(
        _first_match(params, "reverse voltage", "vr (max)", "peak reverse")
    )
    base["ratings"]["current"] = parse_value(_first_match(params, "rectified", "io)"))
    base["properties"] = {
        "forward_voltage": parse_value(fwd_val),
        "diode_type"     : diode_type,
    }
    return base


def _map_bjt(params: dict, base: dict) -> dict:
    base["ratings"]["voltage"] = parse_value(_first_match(params, "vceo", "collector-emitter"))
    base["ratings"]["current"] = parse_value(_first_match(params, "collector (ic)"))
    base["ratings"]["power"]   = parse_value(params.get("Power - Max", "0"))
    base["properties"] = {
        "gain": parse_value(_first_match(params, "hfe", "dc current gain")),
    }
    return base


def _map_fet(params: dict, base: dict) -> dict:
    base["ratings"]["voltage"] = parse_value(_first_match(params, "vdss", "drain to source voltage"))
    base["ratings"]["current"] = parse_value(_first_match(params, "continuous drain", "id @"))
    base["ratings"]["power"]   = parse_value(params.get("Power - Max", "0"))
    base["properties"] = {
        "threshold_voltage": parse_value(_first_match(params, "vgs(th)", "threshold")),
    }
    return base


def _map_ic(params: dict, base: dict) -> dict:
    pin_count = 0
    for key, val in params.items():
        if "pin" in key.lower() or "number of" in key.lower():
            digits = re.sub(r"[^\d]", "", val or "")
            if digits:
                pin_count = int(digits)
                break
    base["properties"] = {
        "pin_count": pin_count,
        "width"    : 0.0,
        "height"   : 0.0,
        "length"   : 0.0,
    }
    return base


# ─── Public conversion function ───────────────────────────────────────────────

_MAPPERS = {
    "Resistor"         : _map_resistor,
    "Capacitor"        : _map_capacitor,
    "Inductor"         : _map_inductor,
    "Diode"            : _map_diode,
    "BJTransistor"     : _map_bjt,
    "FETransistor"     : _map_fet,
    "IntegratedCircuit": _map_ic,
}


def product_to_ecim(product: dict) -> dict:
    """Convert a Digi-Key v4 product dict to an ECIM-compatible JSON object.

    The returned dict mirrors the ``ElectronicComponent::BaseConfig`` struct
    plus a ``properties`` sub-dict for component-specific fields.
    """
    manufacturer_obj = product.get("Manufacturer") or {}
    manufacturer = (
        manufacturer_obj.get("Name", "Unknown")
        if isinstance(manufacturer_obj, dict)
        else str(manufacturer_obj)
    )

    category_obj  = product.get("Category") or {}
    category_name = (
        category_obj.get("Name", "")
        if isinstance(category_obj, dict)
        else str(category_obj)
    )

    description = product.get("ProductDescription") or product.get("Description", "")
    part_number = (
        product.get("ManufacturerProductNumber")
        or product.get("ManufacturerPartNumber")
        or product.get("DigiKeyPartNumber", "")
    )
    quantity   = product.get("QuantityAvailable", 0)
    parameters = product.get("Parameters") or []
    params     = _params_dict(parameters)
    ecim_type  = _detect_type(category_name, description)

    base: dict = {
        "type"        : ecim_type,
        "name"        : part_number,
        "manufacturer": manufacturer,
        "part_number" : part_number,
        "description" : description,
        # Inventory quantity starts at zero; 'digikey_quantity_available'
        # records what Digi-Key currently has in stock for reference.
        "quantity"    : 0,
        "digikey_quantity_available": quantity,
        "ratings"     : {
            "voltage"  : 0.0,
            "current"  : 0.0,
            "power"    : 0.0,
            "tolerance": 0.0,
        },
        "properties"  : {},
    }

    mapper = _MAPPERS.get(ecim_type)
    if mapper is None:
        return base

    # Each mapper takes (params, base) except the ones that also need
    # category_name / description for sub-type detection.
    if ecim_type == "Capacitor":
        return mapper(params, category_name, base)
    if ecim_type == "Diode":
        return mapper(params, category_name, description, base)
    return mapper(params, base)


# ─── CLI ──────────────────────────────────────────────────────────────────────


def _build_client() -> DigiKeyClient:
    client_id     = os.environ.get("DIGIKEY_CLIENT_ID", "")
    client_secret = os.environ.get("DIGIKEY_CLIENT_SECRET", "")
    sandbox       = os.environ.get("DIGIKEY_SANDBOX", "0") == "1"

    if not client_id or not client_secret:
        sys.exit(
            "Error: DIGIKEY_CLIENT_ID and DIGIKEY_CLIENT_SECRET environment "
            "variables must be set.\n"
            "Obtain API credentials at https://developer.digikey.com/."
        )
    return DigiKeyClient(client_id, client_secret, sandbox)


def _cmd_part(args: argparse.Namespace) -> None:
    client  = _build_client()
    raw     = client.get_part_details(args.part_number)
    product = raw.get("Product") or raw
    print(json.dumps(product_to_ecim(product), indent=2))


def _cmd_search(args: argparse.Namespace) -> None:
    client   = _build_client()
    raw      = client.search_keyword(args.keyword, limit=args.limit)
    products = raw.get("Products") or []
    print(json.dumps([product_to_ecim(p) for p in products], indent=2))


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Fetch Digi-Key part data and output ECIM-compatible JSON.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Environment variables:
  DIGIKEY_CLIENT_ID      Digi-Key API client ID         (required)
  DIGIKEY_CLIENT_SECRET  Digi-Key API client secret     (required)
  DIGIKEY_SANDBOX        Set to '1' to use sandbox API  (optional)

Examples:
  python digikey_import.py part CRCW04021K00FKED
  python digikey_import.py search "1k resistor 0402" --limit 5
""",
    )
    sub = parser.add_subparsers(dest="command", required=True)

    # 'part' sub-command
    part_p = sub.add_parser("part", help="Fetch details for a single part number.")
    part_p.add_argument("part_number", help="Manufacturer or Digi-Key part number.")
    part_p.set_defaults(func=_cmd_part)

    # 'search' sub-command
    search_p = sub.add_parser("search", help="Search Digi-Key by keyword.")
    search_p.add_argument("keyword", help="Search keyword.")
    search_p.add_argument(
        "--limit",
        type=int,
        default=10,
        help="Maximum number of results to return (default: 10).",
    )
    search_p.set_defaults(func=_cmd_search)

    args = parser.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
