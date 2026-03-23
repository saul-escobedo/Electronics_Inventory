#include "Application.hpp"
#include "integration/PythonScriptRunner.hpp"

#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>

int execECIMApplication(int argCount, char** argValues) {
	ecim::PythonScriptRunner pythonRunner = ecim::PythonScriptRunner::fromBuildConfig();
	QJsonObject requestObject;
	requestObject["action"] = "inventory-bridge-health-check";
	requestObject["origin"] = "ECIM C++ app";
	requestObject["version"] = "1.0";

	QJsonDocument responseJson;
	const ecim::ScriptExecutionResult bridgeResult = pythonRunner.executeScriptWithJson(
		"scripts/ecim_bridge_echo.py",
		QJsonDocument(requestObject),
		responseJson,
		5000
	);

	QString bridgeStatusMessage;
	if (bridgeResult.ok()) {
		const QJsonObject responseObject = responseJson.object();
		const QString mode = responseObject.value("mode").toString("unknown");
		const int keyCount = responseObject.value("summary").toObject().value("keyCount").toInt(0);
		bridgeStatusMessage = QString("Python bridge status: OK (mode=%1, keys=%2)").arg(mode).arg(keyCount);
	}
	else {
		bridgeStatusMessage = QString("Python bridge status: FAILED (%1)")
			.arg(bridgeResult.stdErr.trimmed().isEmpty() ? "unknown error" : bridgeResult.stdErr.trimmed());
	}

	QApplication app(argCount, argValues);
	QLabel* label = new QLabel(
		QString("Yo wassup, welcome to Electronics Components Invetory Manager\n%1").arg(bridgeStatusMessage)
	);

	label->setWindowTitle("Electronics Components Inventory Manager");
	label->resize(600, 400);
	label->setWordWrap(true);
	label->show();

	return app.exec();
}
