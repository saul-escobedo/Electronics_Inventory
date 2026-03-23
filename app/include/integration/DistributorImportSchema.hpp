#pragma once

#include <QJsonDocument>
#include <QString>

namespace ecim {
	inline constexpr const char* kDistributorImportSchemaId = "ecim.import.v1";
	inline constexpr const char* kDistributorImportSchemaVersion = "1.0.0";

	bool validateDistributorImportPayload(
		const QJsonDocument& payload,
		QString* errorMessage = nullptr
	);
}
