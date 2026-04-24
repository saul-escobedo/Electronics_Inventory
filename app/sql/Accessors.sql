-- This file contains a list of accessor statements that is used by the
-- database backend to modify and access defined objects in a SQL database.
-- These are kept in a separate file to allow syntax highlighting/corrections
-- (in an IDE) and to draw the line between the backend and the SQL engine
-- (like sqlite).

-- How it works: Each accessor statement must have a comment before it that
-- tells the accessor's name in PacalCase. This name will be used to identify
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

--AddDiode
INSERT INTO Diodes
(ComponentID, Type, ForwardVoltage)
VALUES(?, ?, ?);

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
(ComponentID, PinCount, Width, Height, Length)
VALUES(?, ?, ?, ?, ?);



--GetComponent
SELECT * FROM ElectronicComponents WHERE ComponentID = ?;

--GetResistor
SELECT * FROM Resistors WHERE ComponentID = ?;

--GetCapacitor
SELECT * FROM Capacitors WHERE ComponentID = ?;

--GetInductor
SELECT * FROM Inductors WHERE ComponentID = ?;

--GetDiode
SELECT * FROM Diodes WHERE ComponentID = ?;

--GetBjTransistor
SELECT * FROM BJTransistors WHERE ComponentID = ?;

--GetFeTransistor
SELECT * FROM FETransistors WHERE ComponentID = ?;

--GetIntegratedCircuit
SELECT * FROM IntegratedCircuits WHERE ComponentID = ?;



--RemoveComponent
DELETE FROM ElectronicComponents WHERE ComponentID = ?;

--RemoveResistor
DELETE FROM Resistors WHERE ComponentID = ?;

--RemoveCapacitor
DELETE FROM Capacitors WHERE ComponentID = ?;

--RemoveInductor
DELETE FROM Inductors WHERE ComponentID = ?;

--RemoveDiode
DELETE FROM Diodes WHERE ComponentID = ?;

--RemoveBjTransistor
DELETE FROM BJTransistors WHERE ComponentID = ?;

--RemoveFeTransistor
DELETE FROM FETransistors WHERE ComponentID = ?;

--RemoveIntegratedCircuit
DELETE FROM IntegratedCircuits WHERE ComponentID = ?;



--EditComponent
UPDATE ElectronicComponents
SET Name = ?, Manufacturer = ?, PartNumber = ?, Description = ?, Quantity = ?,
VoltageRating = ?, CurrentRating = ?, PowerRating = ?
WHERE ComponentID = ?;

--EditResistor
UPDATE Resistors
SET Resistance = ?, ToleranceBand = ?
WHERE ComponentID = ?;

--EditCapacitor
UPDATE Capacitors
SET Type = ?, Capacitance = ?
WHERE ComponentID = ?;

--EditInductor
UPDATE Inductors
SET Inductance = ?
WHERE ComponentID = ?;

--EditDiode
UPDATE Diodes
SET Type = ?, ForwardVoltage = ?
WHERE ComponentID = ?;

--EditBjTransistor
UPDATE BJTransistors
SET Gain = ?
WHERE ComponentID = ?;

--EditFeTransistor
UPDATE FETransistors
SET ThresholdVoltage = ?
WHERE ComponentID = ?;

--EditIntegratedCircuit
UPDATE IntegratedCircuits
SET PinCount = ?, Width = ?, Height = ?, Length = ?
WHERE ComponentID = ?;
