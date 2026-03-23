#include "integration/DistributorImportSchema.hpp"

#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>

namespace {
	bool _fail(QString* errorMessage, const QString& message) {
		if (errorMessage != nullptr) {
			*errorMessage = message;
		}
		return false;
	}

	bool _requireObjectField(const QJsonObject& object, const QString& fieldName, QString* errorMessage) {
		if (!object.contains(fieldName) || !object.value(fieldName).isObject()) {
			return _fail(errorMessage, QString("Missing object field '%1'").arg(fieldName));
		}
		return true;
	}

	bool _requireArrayField(const QJsonObject& object, const QString& fieldName, QString* errorMessage) {
		if (!object.contains(fieldName) || !object.value(fieldName).isArray()) {
			return _fail(errorMessage, QString("Missing array field '%1'").arg(fieldName));
		}
		return true;
	}

	bool _requireStringField(
		const QJsonObject& object,
		const QString& fieldName,
		QString* errorMessage,
		bool allowEmpty = false
	) {
		if (!object.contains(fieldName) || !object.value(fieldName).isString()) {
			return _fail(errorMessage, QString("Missing string field '%1'").arg(fieldName));
		}
		if (!allowEmpty && object.value(fieldName).toString().trimmed().isEmpty()) {
			return _fail(errorMessage, QString("Field '%1' cannot be empty").arg(fieldName));
		}
		return true;
	}

	bool _requireNonNegativeNumberField(const QJsonObject& object, const QString& fieldName, QString* errorMessage) {
		if (!object.contains(fieldName) || !object.value(fieldName).isDouble()) {
			return _fail(errorMessage, QString("Missing numeric field '%1'").arg(fieldName));
		}
		if (object.value(fieldName).toDouble() < 0.0) {
			return _fail(errorMessage, QString("Field '%1' must be >= 0").arg(fieldName));
		}
		return true;
	}

	bool _validateRating(const QJsonObject& rating, QString* errorMessage) {
		return _requireNonNegativeNumberField(rating, "voltage", errorMessage)
			&& _requireNonNegativeNumberField(rating, "current", errorMessage)
			&& _requireNonNegativeNumberField(rating, "power", errorMessage)
			&& _requireNonNegativeNumberField(rating, "tolerance", errorMessage);
	}

	bool _validateSource(const QJsonObject& source, QString* errorMessage) {
		if (!_requireStringField(source, "provider", errorMessage)) {
			return false;
		}
		if (!source.contains("raw")) {
			return _fail(errorMessage, "Missing field 'source.raw'");
		}
		return true;
	}

	bool _validateTypeProperties(const QString& type, const QJsonObject& properties, QString* errorMessage) {
		if (type == "Resistor") {
			return _requireNonNegativeNumberField(properties, "resistance", errorMessage)
				&& _requireNonNegativeNumberField(properties, "toleranceBand", errorMessage);
		}
		if (type == "Capacitor") {
			if (!_requireNonNegativeNumberField(properties, "capacitance", errorMessage)
				|| !_requireStringField(properties, "capacitorType", errorMessage)) {
				return false;
			}
			const QString value = properties.value("capacitorType").toString();
			static const QStringList allowed = {
				"AluminumPolymer", "AluminumElectrolytic", "Ceramic", "ElectricDoubleLayer",
				"Film", "Mica", "PTFE", "NiobiumOxide", "Silicon", "Tantalum", "ThinFilm",
				"ACMotor", "LithiumHybrid"
			};
			if (!allowed.contains(value)) {
				return _fail(errorMessage, QString("Invalid capacitorType '%1'").arg(value));
			}
			return true;
		}
		if (type == "Inductor") {
			return _requireNonNegativeNumberField(properties, "inductance", errorMessage);
		}
		if (type == "Diode") {
			if (!_requireNonNegativeNumberField(properties, "forwardVoltage", errorMessage)
				|| !_requireStringField(properties, "diodeType", errorMessage)) {
				return false;
			}
			const QString value = properties.value("diodeType").toString();
			static const QStringList allowed = {"Regular", "Schottky", "Zener", "LED"};
			if (!allowed.contains(value)) {
				return _fail(errorMessage, QString("Invalid diodeType '%1'").arg(value));
			}
			return true;
		}
		if (type == "BJTransistor") {
			return _requireNonNegativeNumberField(properties, "gain", errorMessage);
		}
		if (type == "FETransistor") {
			return _requireNonNegativeNumberField(properties, "thresholdVoltage", errorMessage);
		}
		if (type == "IntegratedCircuit") {
			if (!properties.contains("pinCount") || !properties.value("pinCount").isDouble()) {
				return _fail(errorMessage, "Missing numeric field 'pinCount'");
			}
			if (properties.value("pinCount").toInt() <= 0) {
				return _fail(errorMessage, "Field 'pinCount' must be > 0");
			}
			return _requireNonNegativeNumberField(properties, "width", errorMessage)
				&& _requireNonNegativeNumberField(properties, "height", errorMessage)
				&& _requireNonNegativeNumberField(properties, "length", errorMessage);
		}

		return _fail(errorMessage, QString("Unsupported component type '%1'").arg(type));
	}

	bool _validateItem(const QJsonObject& item, QString* errorMessage) {
		if (!_requireStringField(item, "type", errorMessage)
			|| !_requireStringField(item, "name", errorMessage)
			|| !_requireStringField(item, "manufacturer", errorMessage)
			|| !_requireStringField(item, "description", errorMessage, true)
			|| !_requireStringField(item, "partNumber", errorMessage, true)
			|| !_requireObjectField(item, "rating", errorMessage)
			|| !_requireObjectField(item, "properties", errorMessage)
			|| !_requireObjectField(item, "source", errorMessage)) {
			return false;
		}

		if (!item.contains("quantity") || !item.value("quantity").isDouble()) {
			return _fail(errorMessage, "Missing numeric field 'quantity'");
		}
		if (item.value("quantity").toInt() < 0) {
			return _fail(errorMessage, "Field 'quantity' must be >= 0");
		}

		const QString type = item.value("type").toString();
		if (!_validateRating(item.value("rating").toObject(), errorMessage)
			|| !_validateSource(item.value("source").toObject(), errorMessage)
			|| !_validateTypeProperties(type, item.value("properties").toObject(), errorMessage)) {
			return false;
		}

		return true;
	}
}

namespace ecim {
	bool validateDistributorImportPayload(const QJsonDocument& payload, QString* errorMessage) {
		if (!payload.isObject()) {
			return _fail(errorMessage, "Payload root must be a JSON object");
		}

		const QJsonObject root = payload.object();
		if (!_requireStringField(root, "schema", errorMessage)
			|| !_requireStringField(root, "schemaVersion", errorMessage)
			|| !_requireStringField(root, "generatedBy", errorMessage)
			|| !_requireArrayField(root, "items", errorMessage)) {
			return false;
		}

		if (root.value("schema").toString() != kDistributorImportSchemaId) {
			return _fail(errorMessage, QString("Unsupported schema '%1'").arg(root.value("schema").toString()));
		}

		const QJsonArray items = root.value("items").toArray();
		for (qsizetype index = 0; index < items.size(); ++index) {
			if (!items.at(index).isObject()) {
				return _fail(errorMessage, QString("items[%1] must be an object").arg(index));
			}
			QString validationError;
			if (!_validateItem(items.at(index).toObject(), &validationError)) {
				return _fail(errorMessage, QString("items[%1]: %2").arg(index).arg(validationError));
			}
		}

		return true;
	}
}
