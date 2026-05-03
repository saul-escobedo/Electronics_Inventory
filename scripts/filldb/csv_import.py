import csv
import argparse
import re
import sqlite3

from enum import Enum

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

    # Handle percentages.
    if text.endswith("%"):
        try:
            m = re.match(r"^[±=-]?(\d)%", text)
            return float(m.group(1)) / 100.0
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

def parse_pins(package: str) -> int:
    if not package:
        return 0

    # Match a pattern where a hyphen is followed by a number (e.g., "-8" in "SOIC-8")
    match = re.search(r"-(\d+)", package)
    if match:
        try:
            return int(match.group(1))
        except ValueError:
            return 0

    return 0

def find_column_indicies(headers):
    result = {}

    relevant_headers = {
        "manufacturer": "manufacturer",
        "part_number": "mpn part number",
        "quantity": "availability quantity",
        "voltage_rating": "voltage rating drain to source voltage dc reverse voltage(vr) maximum power supply range (vdd-vss)",
        "current_rating": "max current - collector(ic) current rating current - continuous drain(id)",
        "power_rating": "power rating power(watts) pd - power dissipation",
        "resistance": "resistance",
        "capacitance": "capacitance",
        "inductance": "inductance",
        "tolerance": "tolerance",
        "name": "name",
        "forward_voltage": "voltage - forward(vf@if) forward voltage forward",
        "gain": "dc current gain hfe",
        "threshold_voltage": "gate threshold voltage (vgs(th))",
        "pins": "package"
    }

    i = 0
    for header in headers:
        for h, keywords in relevant_headers.items():
            if keywords.rfind(header.lower()) != -1 and h not in result:
                result[h] = i
        i += 1

    return result

class ComponentType(Enum):
    resistor = 'resistor'
    capacitor = 'capacitor'
    inductor = 'inductor'
    diode = 'diode'
    bjt = 'bjt'
    fet = 'fet'
    ic = 'ic'

    def __str__(self):
        return self.value

def add_base(row, col_indicies, type, db):
    sql = """
    INSERT INTO ElectronicComponents
    (Name, Type, Manufacturer, PartNumber, Description, Quantity, VoltageRating,
    CurrentRating, PowerRating)
    VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)
    """

    name = ""

    if type == 'resistor':
        type_id = 1
    elif type == 'capacitor':
        type_id = 2
    elif type == 'inductor':
        type_id = 3
    elif type == 'diode':
        type_id = 4
    elif type == 'bjt':
        type_id = 5
    elif type == 'fet':
        type_id = 6
    elif type == 'ic':
        type_id = 7
    else:
        raise ValueError('Unrecognized component type: ' + str(type))

    manufacturer = ""
    part_number = ""
    description = ""
    quantity = 0
    voltage_rating = 0
    current_rating = 0
    power_rating = 0

    if "manufacturer" in col_indicies:
        manufacturer = row[col_indicies["manufacturer"]]
    if "part_number" in col_indicies:
        part_number = row[col_indicies["part_number"]]
    if "quantity" in col_indicies:
        quantity = parse_value(row[col_indicies["quantity"]])
    if "voltage_rating" in col_indicies:
        voltage_rating = parse_value(row[col_indicies["voltage_rating"]])
    if "current_rating" in col_indicies:
        current_rating = parse_value(row[col_indicies["current_rating"]])
    if "power_rating" in col_indicies:
        power_rating = parse_value(row[col_indicies["power_rating"]])

    name = manufacturer + ' ' + part_number

    db.execute(sql, (name, type_id, manufacturer, part_number,
        description, quantity, voltage_rating, current_rating, power_rating))

    return db.lastrowid


def add_resistor_dataset(data, db):
    col_indicies = find_column_indicies(data[0])

    sql = """
    INSERT INTO Resistors
    (ComponentID, Resistance, ToleranceBand)
    VALUES(?, ?, ?)
    """

    for row in range(1, len(data)):
        component_id = add_base(data[row], col_indicies, 'resistor', db)
        resistance = 0
        tolerance = 0

        if "resistance" in col_indicies:
            resistance = parse_value(data[row][col_indicies["resistance"]])

        if "tolerance" in  col_indicies:
            tolerance = parse_value(data[row][col_indicies["tolerance"]])

        db.execute(sql, (component_id, resistance, tolerance))

def add_capacitor_dataset(data, db):
    col_indicies = find_column_indicies(data[0])

    sql = """
    INSERT INTO Capacitors
    (ComponentID, Type, Capacitance)
    VALUES(?, ?, ?);
    """

    for row in range(1, len(data)):
        component_id = add_base(data[row], col_indicies, 'capacitor', db)
        capacitance = 0
        type = 2 # Let's always infer that its a cermic capacitor for now

        if "capacitance" in col_indicies:
            capacitance = parse_value(data[row][col_indicies["capacitance"]])

        db.execute(sql, (component_id, type, capacitance))

def add_inductor_dataset(data, db):
    col_indicies = find_column_indicies(data[0])

    sql = """
    INSERT INTO Inductors
    (ComponentID, Inductance)
    VALUES(?, ?);
    """

    for row in range(1, len(data)):
        component_id = add_base(data[row], col_indicies, 'inductor', db)
        inductance = 0

        if "inductance" in col_indicies:
            inductance = parse_value(data[row][col_indicies["inductance"]])

        db.execute(sql, (component_id, inductance))

def add_diode_dataset(data, db):
    col_indicies = find_column_indicies(data[0])

    sql = """
    INSERT INTO Diodes
    (ComponentID, Type, ForwardVoltage)
    VALUES(?, ?, ?)
    """

    for row in range(1, len(data)):
        component_id = add_base(data[row], col_indicies, 'diode', db)
        forward_voltage = 0
        type = 0 # Let's always infer that its a regular diode for now

        if "forward_voltage" in col_indicies:
            forward_voltage = parse_value(data[row][col_indicies["forward_voltage"]])

        db.execute(sql, (component_id, type, forward_voltage))

def add_bjt_dataset(data, db):
    col_indicies = find_column_indicies(data[0])

    sql = """
    INSERT INTO BJTransistors
    (ComponentID, Gain)
    VALUES(?, ?)
    """

    for row in range(1, len(data)):
        component_id = add_base(data[row], col_indicies, 'bjt', db)
        gain = 0

        if "gain" in col_indicies:
            gain = parse_value(data[row][col_indicies["gain"]])

        db.execute(sql, (component_id, gain))

def add_fet_dataset(data, db):
    col_indicies = find_column_indicies(data[0])

    sql = """
    INSERT INTO FETransistors
    (ComponentID, ThresholdVoltage)
    VALUES(?, ?);
    """

    for row in range(1, len(data)):
        component_id = add_base(data[row], col_indicies, 'fet', db)
        threshold_voltage = 0

        if "threshold_voltage" in col_indicies:
            threshold_voltage = parse_value(data[row][col_indicies["threshold_voltage"]])

        db.execute(sql, (component_id, threshold_voltage))

def add_ic_dataset(data, db):
    col_indicies = find_column_indicies(data[0])

    sql = """
    INSERT INTO IntegratedCircuits
    (ComponentID, PinCount, Width, Height, Length)
    VALUES(?, ?, ?, ?, ?);
    """

    for row in range(1, len(data)):
        component_id = add_base(data[row], col_indicies, 'ic', db)
        pin_count = 0
        # Assume all dimensions are fixed to 10mm
        width = 0.01
        height = 0.01
        length = 0.01

        if "pins" in col_indicies:
            pin_count = parse_pins(data[row][col_indicies["pins"]])

        db.execute(sql, (component_id, pin_count, width, height, length))

def main():
    parser = argparse.ArgumentParser(description='Import a list of electronic parts from a csv file into an sqlite database')
    parser.add_argument('type', type=ComponentType, choices=list(ComponentType))
    parser.add_argument('input_file', type=str, help='Path to the input CSV file')
    parser.add_argument('output_file', type=str, help='Path to the output file')
    args = parser.parse_args()

    # Read the input CSV file
    with open(args.input_file, mode='r', newline='', encoding='utf-8') as infile:
        reader = csv.reader(infile)
        data = list(reader)

    db = sqlite3.connect(args.output_file)

    with open('../../app/sql/Schema.sql', mode='r') as schemafile:
        schema = schemafile.read()
        db.executescript(schema)

    if args.type == ComponentType.resistor:
        add_resistor_dataset(data, db.cursor())
    elif args.type == ComponentType.capacitor:
        add_capacitor_dataset(data, db.cursor())
    elif args.type == ComponentType.inductor:
        add_inductor_dataset(data, db.cursor())
    elif args.type == ComponentType.diode:
        add_diode_dataset(data, db.cursor())
    elif args.type == ComponentType.bjt:
        add_bjt_dataset(data, db.cursor())
    elif args.type == ComponentType.fet:
        add_fet_dataset(data, db.cursor())
    elif args.type == ComponentType.ic:
        add_ic_dataset(data, db.cursor())

    db.commit()

    db.close()

if __name__ == '__main__':
    main()
