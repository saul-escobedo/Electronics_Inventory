#include <gtest/gtest.h>

#include "integration/DistributorImportSchema.hpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

TEST(DistributorImportSchema, ValidPayloadPasses) {
	const QByteArray json = R"({
		"schema": "ecim.import.v1",
		"schemaVersion": "1.0.0",
		"generatedBy": "scripts/example.py",
		"items": [
			{
				"type": "Resistor",
				"name": "R1",
				"manufacturer": "Vishay",
				"partNumber": "R-1K",
				"description": "1k resistor",
				"quantity": 10,
				"rating": {
					"voltage": 50,
					"current": 0,
					"power": 0.25,
					"tolerance": 5
				},
				"properties": {
					"resistance": 1000,
					"toleranceBand": 5
				},
				"source": {
					"provider": "DigiKey",
					"raw": {}
				}
			}
		]
	})";

	const QJsonDocument payload = QJsonDocument::fromJson(json);
	QString error;
	EXPECT_TRUE(ecim::validateDistributorImportPayload(payload, &error)) << error.toStdString();
}

TEST(DistributorImportSchema, MissingFieldFails) {
	const QByteArray json = R"({
		"schema": "ecim.import.v1",
		"schemaVersion": "1.0.0",
		"generatedBy": "scripts/example.py",
		"items": [
			{
				"type": "Diode",
				"name": "D1",
				"manufacturer": "Onsemi",
				"partNumber": "D-123",
				"description": "signal diode",
				"quantity": 1,
				"rating": {
					"voltage": 100,
					"current": 1,
					"power": 1,
					"tolerance": 0
				},
				"properties": {
					"forwardVoltage": 0.7
				},
				"source": {
					"provider": "DigiKey",
					"raw": {}
				}
			}
		]
	})";

	const QJsonDocument payload = QJsonDocument::fromJson(json);
	QString error;
	EXPECT_FALSE(ecim::validateDistributorImportPayload(payload, &error));
	EXPECT_FALSE(error.isEmpty());
}
