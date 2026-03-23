#pragma once

#include <QJsonDocument>
#include <QString>
#include <QStringList>

namespace ecim {
	struct ScriptExecutionResult {
		int exitCode = -1;
		QString stdOut;
		QString stdErr;
		bool timedOut = false;
		bool started = false;

		bool ok() const {
			return started && !timedOut && exitCode == 0;
		}
	};

	class PythonScriptRunner {
		public:
		explicit PythonScriptRunner(QString pythonExecutable, QString projectRoot);

		static PythonScriptRunner fromBuildConfig();

		ScriptExecutionResult executeScript(
			const QString& scriptPath,
			const QStringList& arguments = {},
			int timeoutMs = 30000
		) const;

		ScriptExecutionResult executeScriptWithFiles(
			const QString& scriptPath,
			const QString& inputFilePath,
			const QString& outputFilePath,
			const QStringList& extraArguments = {},
			int timeoutMs = 30000
		) const;

		ScriptExecutionResult executeScriptWithJson(
			const QString& scriptPath,
			const QJsonDocument& requestJson,
			QJsonDocument& responseJson,
			int timeoutMs = 30000
		) const;

		private:
		QString _resolveScriptPath(const QString& scriptPath) const;

		QString m_pythonExecutable;
		QString m_projectRoot;
	};
}
