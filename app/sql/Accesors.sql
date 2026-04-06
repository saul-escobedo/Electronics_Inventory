-- This file contains a list of accessor statements that is used by the
-- database backend to modify and access defined objects in a SQL database.
-- These are kept in a separate file to allow syntax highlighting/corrections
-- (in an IDE) and to draw the line between the backend and the SQL engine
-- (like sqlite).

-- How it works: Each accessor statement must have a comment before it that
-- tells the accessor's name in Pacal Case. This name will be used to identify
-- each statement when it's embedded in a header file. The semicolon at the end
-- of each statement is the delimiter. The "?" symbol is a placeholder for a
-- value that will be filled at runtime.

--AddComponent
INSERT INTO ElectronicComponents
(Name, Type, Manufacturer, PartNumber, Description, Quantity, VoltageRating,
CurrentRating, PowerRating)
VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?);

--AddResistor
INSERT INTO Resistors
(ComponentID, Resistance, ToleranceBand)
VALUES(?, ?, ?);

--AddCapacitor
INSERT INTO Capacitors
(ComponentID, Type, Capacitance)
VALUES(?, ?, ?);

--AddInductor
INSERT INTO Inductors
(ComponentID, Inductance)
VALUES(?, ?);

--AddBjTransistor
INSERT INTO BJTransistors
(ComponentID, Gain)
VALUES(?, ?);

--AddFeTransistor
INSERT INTO FETransistors
(ComponentID, ThresholdVoltage)
VALUES(?, ?);

--AddIntegratedCircuit
INSERT INTO IntegratedCircuits
(ComponentID, PinCout, Width, Height, Length)
VALUES(?, ?, ?, ?, ?);
