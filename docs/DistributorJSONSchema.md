# Distributor JSON Schema Standard

This document defines the canonical JSON contract that all distributor import scripts must output before data is consumed by the C++ application.

## Canonical Schema

- JSON Schema file: `schemas/ecim.distributor-import.schema.v1.json`
- Schema identifier (`schema`): `ecim.import.v1`
- Schema version (`schemaVersion`): `1.0.0`

## Envelope

Top-level JSON object fields:

- `schema` (string, required)
- `schemaVersion` (string, required)
- `generatedBy` (string, required)
- `items` (array, required)

## Item Contract

Each entry in `items` must include:

- `type`: `Resistor | Capacitor | Inductor | Diode | BJTransistor | FETransistor | IntegratedCircuit`
- `name` (string, non-empty)
- `manufacturer` (string, non-empty)
- `partNumber` (string, can be empty)
- `description` (string, can be empty)
- `quantity` (integer, >= 0)
- `rating` object with non-negative numbers:
  - `voltage`, `current`, `power`, `tolerance`
- `properties` object (type-specific fields)
- `source` object with:
  - `provider` (string, non-empty)
  - `raw` (original distributor payload, any JSON)

## Type-specific `properties`

- `Resistor`: `resistance`, `toleranceBand`
- `Capacitor`: `capacitance`, `capacitorType`
- `Inductor`: `inductance`
- `Diode`: `forwardVoltage`, `diodeType`
- `BJTransistor`: `gain`
- `FETransistor`: `thresholdVoltage`
- `IntegratedCircuit`: `pinCount (> 0)`, `width`, `height`, `length`

## C++ Validation Hook

C++ validation API:

- Header: `app/include/integration/DistributorImportSchema.hpp`
- Function: `validateDistributorImportPayload(...)`

Use this validation before converting JSON items into C++ component classes.
