-- ECIM Database Schema Version 1

BEGIN;

CREATE TABLE IF NOT EXISTS ElectronicComponents(
    ComponentID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    Name TEXT NOT NULL,
    Type INTEGER NOT NULL, --ElectronicComponent::Type enum
    Manufacturer TEXT NOT NULL,
    PartNumber TEXT,
    Description TEXT,
    Quantity NOT NULL DEFAULT 0,
    VoltageRating DOUBLE DEFAULT 0,
    CurrentRating DOUBLE DEFAULT 0,
    PowerRating DOUBLE DEFAULT 0
);

CREATE TABLE IF NOT EXISTS Resistors(
    ResistorID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    ComponentID INTEGER,
    Resistance DOUBLE DEFAULT 0, --Ohms
    ToleranceBand DOUBLE DEFAULT 0,
    FOREIGN KEY (ComponentID) REFERENCES ElectronicComponents(ComponentID)
);

CREATE TABLE IF NOT EXISTS Capacitors(
    CapacitorID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    ComponentID INTEGER,
    Type INTEGER NOT NULL, --Capacitor::Type enum
    Capacitance DOUBLE DEFAULT 0, --Farads
    FOREIGN KEY (ComponentID) REFERENCES ElectronicComponents(ComponentID)
);

CREATE TABLE IF NOT EXISTS Inductors(
    InductorID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    ComponentID INTEGER,
    Inductance DOUBLE DEFAULT 0, --Henrys
    FOREIGN KEY (ComponentID) REFERENCES ElectronicComponents(ComponentID)
);

CREATE TABLE IF NOT EXISTS BJTransistors(
    TransistorID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    ComponentID INTEGER,
    Gain DOUBLE DEFAULT 0, --hFE
    FOREIGN KEY (ComponentID) REFERENCES ElectronicComponents(ComponentID)
);

CREATE TABLE IF NOT EXISTS FETransistors(
    TransistorID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    ComponentID INTEGER,
    ThresholdVoltage DOUBLE DEFAULT 0, --Volts
    FOREIGN KEY (ComponentID) REFERENCES ElectronicComponents(ComponentID)
);

CREATE TABLE IF NOT EXISTS Diodes(
    DiodeID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    ComponentID INTEGER,
    Type INTEGER NOT NULL, --Diode::Type enum
    ForwardVoltage DOUBLE DEFAULT 0, --Volts
    FOREIGN KEY (ComponentID) REFERENCES ElectronicComponents(ComponentID)
);

CREATE TABLE IF NOT EXISTS IntegratedCircuits(
    ChipID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    ComponentID INTEGER,
    PinCount INTEGER DEFAULT 0,
    Width DOUBLE DEFAULT 0, --milimeters
    Height DOUBLE DEFAULT 0, --milimeters
    Length DOUBLE DEFAULT 0, --milimeters
    FOREIGN KEY (ComponentID) REFERENCES ElectronicComponents(ComponentID)
);

COMMIT;
