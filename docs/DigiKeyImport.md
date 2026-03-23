# Digi-Key Import Script

This guide describes how to fetch part information from Digi-Key and convert it into JSON records compatible with ECIM.

## Script Location

- `scripts/digikey_to_ecim.py`

## Prerequisites

1. Digi-Key API credentials (`client_id` and `client_secret`)
2. Python 3.9+

## Credentials

Set your Digi-Key credentials as environment variables:

```bash
export DIGIKEY_CLIENT_ID="your_client_id"
export DIGIKEY_CLIENT_SECRET="your_client_secret"
```

You may also pass credentials directly with CLI flags (`--client-id`, `--client-secret`).

## Example Usage

```bash
python3 scripts/digikey_to_ecim.py \
  --keywords "NE555" "1k resistor" "IRLZ44N" \
  --limit-per-keyword 5 \
  --quantity 10 \
  --output ecim_parts.json \
  --pretty
```

## Bridge Mode (Input/Output Files)

The script also supports the ECIM bridge contract used by the C++ runner:

```bash
python3 scripts/digikey_to_ecim.py \
  --input-file request.json \
  --output-file ecim_parts.json
```

Example `request.json`:

```json
{
  "keywords": ["NE555", "1k resistor"],
  "limitPerKeyword": 5,
  "quantity": 10,
  "pretty": true,
  "clientId": "your_client_id",
  "clientSecret": "your_client_secret"
}
```

CLI flags take precedence over values provided in the input JSON file.

## Output Format

The script writes JSON in this envelope:

```json
{
  "schema": "ecim.import.v1",
  "schemaVersion": "1.0.0",
  "generatedBy": "scripts/digikey_to_ecim.py",
  "items": [
    {
      "type": "Resistor",
      "name": "...",
      "manufacturer": "...",
      "partNumber": "...",
      "description": "...",
      "quantity": 10,
      "rating": {
        "voltage": 50.0,
        "current": 0.0,
        "power": 0.25,
        "tolerance": 5.0
      },
      "properties": {
        "resistance": 1000.0,
        "toleranceBand": 5.0
      },
      "source": {
        "provider": "DigiKey",
        "raw": {}
      }
    }
  ]
}
```

The full canonical schema is defined in `schemas/ecim.distributor-import.schema.v1.json`.

## Type Mapping

The script infers ECIM component type from category/description and parametric fields:

- `Resistor`
- `Capacitor`
- `Inductor`
- `Diode`
- `BJTransistor`
- `FETransistor`
- `IntegratedCircuit` (fallback)

## Notes

- Numeric values are parsed from Digi-Key text fields with SI prefixes (for example, `"1k Ohms" -> 1000`).
- The original Digi-Key product payload is preserved under `source.raw` for traceability.
- If Digi-Key API response fields differ by endpoint version, this script attempts multiple known key variants.
