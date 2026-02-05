# Electronics Inventory — Project Objective (Spring 2026)

Project objective
-----------------
This semester (Spring 2026) our team is building an Electronics Inventory system for hobbyists who maintain large collections of small SMD components. The system runs locally (no internet required) and provides a fast, native UI and optional barcode-scanner integration to update inventory in the workshop.

Key capabilities
----------------
- Track components by attributes such as package size, resistance, capacitance, inductance, and other metadata.
- Fast lookup to see what is in stock and what needs ordering while designing circuits.
- Local-first operation: the app stores and queries data on the local machine; barcode scanning accelerates input.

Quality-of-life (QOL) features
-----------------------------
- Pull component specifications (wattage, tolerance, ESR, etc.) from distributor APIs or CSVs (e.g., LCSC, Digi-Key) when available, to populate part metadata.
- Voltage-divider designer that suggests the best divider using resistors currently in inventory.
