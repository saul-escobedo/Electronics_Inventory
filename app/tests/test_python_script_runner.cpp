#include <gtest/gtest.h>

#include "integration/PythonScriptRunner.hpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryFile>
#include <QTextStream>

using ecim::PythonScriptRunner;
using ecim::ScriptExecutionResult;

TEST(PythonScriptRunner, ExecuteScriptWithJsonSuccess) {
	PythonScriptRunner runner = PythonScriptRunner::fromBuildConfig();

	QJsonObject request;
	request["action"] = "test";
	request["component"] = "resistor";

	QJsonDocument response;
	const ScriptExecutionResult result = runner.executeScriptWithJson(
		"scripts/ecim_bridge_echo.py",
		QJsonDocument(request),
		response,
		5000
	);

	ASSERT_TRUE(result.ok()) << result.stdErr.toStdString();
	ASSERT_TRUE(response.isObject());
	const QJsonObject responseObject = response.object();
	EXPECT_TRUE(responseObject.value("ok").toBool(false));
	EXPECT_EQ(responseObject.value("mode").toString().toStdString(), "file");
	EXPECT_EQ(
		responseObject.value("summary").toObject().value("keyCount").toInt(-1),
		2
	);
}

TEST(PythonScriptRunner, ExecuteScriptWithFilesSuccess) {
	PythonScriptRunner runner = PythonScriptRunner::fromBuildConfig();

	QTemporaryFile inputFile;
	QTemporaryFile outputFile;
	ASSERT_TRUE(inputFile.open());
	ASSERT_TRUE(outputFile.open());

	const QByteArray payload = R"({"source":"gtest","value":42})";
	ASSERT_EQ(inputFile.write(payload), payload.size());
	inputFile.flush();
	inputFile.close();
	outputFile.close();

	const ScriptExecutionResult result = runner.executeScriptWithFiles(
		"scripts/ecim_bridge_echo.py",
		inputFile.fileName(),
		outputFile.fileName(),
		{},
		5000
	);

	ASSERT_TRUE(result.ok()) << result.stdErr.toStdString();

	ASSERT_TRUE(outputFile.open());
	const QJsonDocument parsed = QJsonDocument::fromJson(outputFile.readAll());
	ASSERT_TRUE(parsed.isObject());
	EXPECT_EQ(parsed.object().value("mode").toString().toStdString(), "file");
	EXPECT_EQ(parsed.object().value("summary").toObject().value("keyCount").toInt(-1), 2);
}

TEST(PythonScriptRunner, ExecuteScriptTimeout) {
	PythonScriptRunner runner = PythonScriptRunner::fromBuildConfig();

	QTemporaryFile slowScript;
	ASSERT_TRUE(slowScript.open());

	QTextStream out(&slowScript);
	out << "import time\n";
	out << "time.sleep(2)\n";
	out << "print('{\\\"ok\\\": true}')\n";
	out.flush();
	slowScript.flush();
	slowScript.close();

	const ScriptExecutionResult result = runner.executeScript(slowScript.fileName(), {}, 100);
	EXPECT_TRUE(result.started);
	EXPECT_TRUE(result.timedOut);
	EXPECT_FALSE(result.ok());
}
