"""Tests for digikey_import.py — covers parse_value and product_to_ecim."""

import sys
import os

# Allow importing the script without installing it as a package.
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

import pytest
from digikey_import import parse_value, product_to_ecim


# ─── parse_value ─────────────────────────────────────────────────────────────


class TestParseValue:
    def test_bare_integer(self):
        assert parse_value("1000") == pytest.approx(1000.0)

    def test_bare_float(self):
        assert parse_value("3.3") == pytest.approx(3.3)

    def test_kilo_prefix(self):
        assert parse_value("1 kOhms") == pytest.approx(1000.0)

    def test_mega_prefix(self):
        assert parse_value("1 MOhms") == pytest.approx(1e6)

    def test_milli_prefix(self):
        assert parse_value("100 mW") == pytest.approx(0.1)

    def test_nano_prefix(self):
        assert parse_value("100 nF") == pytest.approx(1e-7)

    def test_micro_prefix_symbol(self):
        assert parse_value("47 µH") == pytest.approx(47e-6)

    def test_micro_prefix_u(self):
        assert parse_value("10 uF") == pytest.approx(10e-6)

    def test_pico_prefix(self):
        assert parse_value("10 pF") == pytest.approx(10e-12)

    def test_percentage(self):
        assert parse_value("5%") == pytest.approx(0.05)

    def test_percentage_decimal(self):
        assert parse_value("0.1%") == pytest.approx(0.001)

    def test_empty_string(self):
        assert parse_value("") == pytest.approx(0.0)

    def test_non_numeric(self):
        assert parse_value("N/A") == pytest.approx(0.0)

    def test_scientific_notation(self):
        assert parse_value("1e3") == pytest.approx(1000.0)

    def test_voltage_no_prefix(self):
        assert parse_value("50 V") == pytest.approx(50.0)

    def test_giga_prefix(self):
        assert parse_value("1 GHz") == pytest.approx(1e9)


# ─── product_to_ecim — helper ────────────────────────────────────────────────


def _make_product(
    category: str,
    description: str,
    manufacturer: str = "ACME Corp",
    part_number: str = "TEST-001",
    parameters: list | None = None,
    quantity: int = 500,
) -> dict:
    """Build a minimal Digi-Key product dict for testing."""
    return {
        "ManufacturerProductNumber": part_number,
        "ProductDescription"       : description,
        "Manufacturer"             : {"Name": manufacturer},
        "Category"                 : {"Name": category},
        "Parameters"               : parameters or [],
        "QuantityAvailable"        : quantity,
    }


# ─── product_to_ecim — base fields ───────────────────────────────────────────


class TestProductToEcimBaseFields:
    def test_part_number_propagated(self):
        result = product_to_ecim(_make_product("Resistors", "1k Resistor"))
        assert result["part_number"] == "TEST-001"

    def test_manufacturer_propagated(self):
        result = product_to_ecim(_make_product("Resistors", "1k Resistor", manufacturer="Vishay"))
        assert result["manufacturer"] == "Vishay"

    def test_description_propagated(self):
        result = product_to_ecim(_make_product("Resistors", "1k Resistor"))
        assert result["description"] == "1k Resistor"

    def test_inventory_quantity_zero(self):
        result = product_to_ecim(_make_product("Resistors", "1k Resistor", quantity=9999))
        assert result["quantity"] == 0

    def test_digikey_quantity_recorded(self):
        result = product_to_ecim(_make_product("Resistors", "1k Resistor", quantity=250))
        assert result["digikey_quantity_available"] == 250

    def test_ratings_keys_present(self):
        result = product_to_ecim(_make_product("Resistors", "1k Resistor"))
        assert set(result["ratings"]) >= {"voltage", "current", "power", "tolerance"}

    def test_manufacturer_dict_missing_name(self):
        product = _make_product("Resistors", "1k Resistor")
        product["Manufacturer"] = {}
        result = product_to_ecim(product)
        assert result["manufacturer"] == "Unknown"

    def test_manufacturer_string_fallback(self):
        product = _make_product("Resistors", "1k Resistor")
        product["Manufacturer"] = "VisMfg"
        result = product_to_ecim(product)
        assert result["manufacturer"] == "VisMfg"


# ─── product_to_ecim — Resistor ──────────────────────────────────────────────


class TestResistorMapping:
    def _resistor(self, resistance="1 kOhms", tolerance="5%", power="0.25 W", voltage="50 V"):
        params = [
            {"Parameter": "Resistance",       "Value": resistance},
            {"Parameter": "Tolerance",        "Value": tolerance},
            {"Parameter": "Power (Watts)",    "Value": power},
            {"Parameter": "Voltage Rating - DC", "Value": voltage},
        ]
        return product_to_ecim(_make_product("Resistors", "Resistor", parameters=params))

    def test_type(self):
        assert self._resistor()["type"] == "Resistor"

    def test_resistance(self):
        assert self._resistor()["properties"]["resistance"] == pytest.approx(1000.0)

    def test_tolerance_band(self):
        assert self._resistor()["properties"]["tolerance_band"] == pytest.approx(0.05)

    def test_power_rating(self):
        assert self._resistor()["ratings"]["power"] == pytest.approx(0.25)

    def test_voltage_rating(self):
        assert self._resistor()["ratings"]["voltage"] == pytest.approx(50.0)


# ─── product_to_ecim — Capacitor ─────────────────────────────────────────────


class TestCapacitorMapping:
    def _cap(self, capacitance="100 nF", voltage="10 V", cap_type="Ceramic"):
        params = [
            {"Parameter": "Capacitance",   "Value": capacitance},
            {"Parameter": "Voltage - Rated", "Value": voltage},
            {"Parameter": "Type",          "Value": cap_type},
        ]
        return product_to_ecim(_make_product("Capacitors", "Capacitor", parameters=params))

    def test_type(self):
        assert self._cap()["type"] == "Capacitor"

    def test_capacitance(self):
        assert self._cap()["properties"]["capacitance"] == pytest.approx(100e-9)

    def test_voltage_rating(self):
        assert self._cap()["ratings"]["voltage"] == pytest.approx(10.0)

    def test_ceramic_type(self):
        assert self._cap(cap_type="Ceramic")["properties"]["capacitor_type"] == "Ceramic"

    def test_tantalum_type(self):
        assert self._cap(cap_type="Tantalum")["properties"]["capacitor_type"] == "Tantalum"

    def test_film_type(self):
        assert self._cap(cap_type="Film")["properties"]["capacitor_type"] == "Film"

    def test_aluminum_electrolytic_type(self):
        result = self._cap(cap_type="Aluminum Electrolytic")
        assert result["properties"]["capacitor_type"] == "AluminumElectrolytic"


# ─── product_to_ecim — Inductor ──────────────────────────────────────────────


class TestInductorMapping:
    def _inductor(self, inductance="47 µH", current="1 A"):
        params = [
            {"Parameter": "Inductance",          "Value": inductance},
            {"Parameter": "Current Rating (Amps)", "Value": current},
        ]
        return product_to_ecim(_make_product("Inductors, Coils, Chokes", "Inductor", parameters=params))

    def test_type(self):
        assert self._inductor()["type"] == "Inductor"

    def test_inductance(self):
        assert self._inductor()["properties"]["inductance"] == pytest.approx(47e-6)

    def test_current_rating(self):
        assert self._inductor()["ratings"]["current"] == pytest.approx(1.0)


# ─── product_to_ecim — Diode ─────────────────────────────────────────────────


class TestDiodeMapping:
    def _diode(self, category="Diodes - Rectifiers", desc="Diode", fwd_v="0.7 V"):
        params = [
            {"Parameter": "Voltage - Forward (Vf) (Typ)", "Value": fwd_v},
        ]
        return product_to_ecim(_make_product(category, desc, parameters=params))

    def test_type(self):
        assert self._diode()["type"] == "Diode"

    def test_forward_voltage(self):
        assert self._diode()["properties"]["forward_voltage"] == pytest.approx(0.7)

    def test_regular_diode(self):
        assert self._diode()["properties"]["diode_type"] == "Regular"

    def test_zener_detected_from_category(self):
        result = self._diode(category="Diodes - Zener", desc="Zener Diode")
        assert result["properties"]["diode_type"] == "Zener"

    def test_schottky_detected_from_description(self):
        result = self._diode(category="Diodes", desc="Schottky Barrier Rectifier")
        assert result["properties"]["diode_type"] == "Schottky"

    def test_led_detected_from_category(self):
        result = self._diode(category="LED Indication - Discrete", desc="Green LED")
        assert result["properties"]["diode_type"] == "LED"


# ─── product_to_ecim — BJTransistor ──────────────────────────────────────────


class TestBJTMapping:
    def _bjt(self):
        params = [
            {"Parameter": "DC Current Gain (hFE) (Min) @ If, Vce", "Value": "100"},
            {"Parameter": "Collector-Emitter Voltage (Vceo)",       "Value": "40 V"},
            {"Parameter": "Current - Collector (Ic) (Max)",         "Value": "600 mA"},
            {"Parameter": "Power - Max",                             "Value": "625 mW"},
        ]
        return product_to_ecim(
            _make_product("Transistors - Bipolar (BJT) - Single", "NPN BJT", parameters=params)
        )

    def test_type(self):
        assert self._bjt()["type"] == "BJTransistor"

    def test_gain(self):
        assert self._bjt()["properties"]["gain"] == pytest.approx(100.0)

    def test_voltage_rating(self):
        assert self._bjt()["ratings"]["voltage"] == pytest.approx(40.0)

    def test_current_rating(self):
        assert self._bjt()["ratings"]["current"] == pytest.approx(0.6)

    def test_power_rating(self):
        assert self._bjt()["ratings"]["power"] == pytest.approx(0.625)


# ─── product_to_ecim — FETransistor ──────────────────────────────────────────


class TestFETMapping:
    def _fet(self):
        params = [
            {"Parameter": "Vgs(th) (Max) @ Id",           "Value": "2 V"},
            {"Parameter": "Drain to Source Voltage (Vdss)", "Value": "60 V"},
            {"Parameter": "Current - Continuous Drain (Id) @ 25°C", "Value": "30 A"},
            {"Parameter": "Power - Max",                   "Value": "100 W"},
        ]
        return product_to_ecim(
            _make_product("Transistors - FETs, MOSFETs - Single", "N-Ch MOSFET", parameters=params)
        )

    def test_type(self):
        assert self._fet()["type"] == "FETransistor"

    def test_threshold_voltage(self):
        assert self._fet()["properties"]["threshold_voltage"] == pytest.approx(2.0)

    def test_voltage_rating(self):
        assert self._fet()["ratings"]["voltage"] == pytest.approx(60.0)

    def test_current_rating(self):
        assert self._fet()["ratings"]["current"] == pytest.approx(30.0)

    def test_power_rating(self):
        assert self._fet()["ratings"]["power"] == pytest.approx(100.0)


# ─── product_to_ecim — IntegratedCircuit ─────────────────────────────────────


class TestICMapping:
    def _ic(self, pin_count="8"):
        params = [
            {"Parameter": "Number of Pins", "Value": pin_count},
        ]
        return product_to_ecim(
            _make_product("Integrated Circuits (ICs)", "Op-Amp", parameters=params)
        )

    def test_type(self):
        assert self._ic()["type"] == "IntegratedCircuit"

    def test_pin_count(self):
        assert self._ic()["properties"]["pin_count"] == 8

    def test_default_dimensions_zero(self):
        result = self._ic()
        assert result["properties"]["width"]  == pytest.approx(0.0)
        assert result["properties"]["height"] == pytest.approx(0.0)
        assert result["properties"]["length"] == pytest.approx(0.0)


# ─── product_to_ecim — type detection edge cases ─────────────────────────────


class TestTypeDetection:
    def test_fallback_to_ic(self):
        product = _make_product("Unknown Category", "Some random part")
        assert product_to_ecim(product)["type"] == "IntegratedCircuit"

    def test_coil_mapped_to_inductor(self):
        product = _make_product("Fixed Inductors", "Ferrite bead 100 Ohm")
        assert product_to_ecim(product)["type"] == "Inductor"

    def test_empty_parameters_handled_gracefully(self):
        product = _make_product("Resistors", "Resistor", parameters=[])
        result = product_to_ecim(product)
        assert result["type"] == "Resistor"
        assert result["properties"]["resistance"] == pytest.approx(0.0)
