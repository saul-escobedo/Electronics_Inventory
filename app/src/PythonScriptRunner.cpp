#include "integration/PythonScriptRunner.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonParseError>
#include <QProcess>
#include <QTemporaryFile>

#include <utility>

namespace {
	QString _trimmedStdErr(const QString& stdErr) {
		return stdErr.trimmed().isEmpty() ? QString("Unknown script execution error") : stdErr.trimmed();
	}
}

namespace ecim {
	PythonScriptRunner::PythonScriptRunner(QString pythonExecutable, QString projectRoot)
		: m_pythonExecutable(std::move(pythonExecutable)),
		  m_projectRoot(std::move(projectRoot)) {}

	PythonScriptRunner PythonScriptRunner::fromBuildConfig() {
#ifdef ECIM_PYTHON_EXECUTABLE
		const QString pythonExecutable = QString::fromUtf8(ECIM_PYTHON_EXECUTABLE);
#else
		const QString pythonExecutable = QString("python3");
#endif

#ifdef ECIM_PROJECT_ROOT
		const QString projectRoot = QString::fromUtf8(ECIM_PROJECT_ROOT);
#else
		const QString projectRoot = QDir::currentPath();
#endif
		return PythonScriptRunner(pythonExecutable, projectRoot);
	}

	ScriptExecutionResult PythonScriptRunner::executeScript(
		const QString& scriptPath,
		const QStringList& arguments,
		int timeoutMs
	) const {
		ScriptExecutionResult result;
		QProcess process;
		QStringList finalArgs;
		finalArgs << _resolveScriptPath(scriptPath);
		finalArgs << arguments;
		process.setWorkingDirectory(m_projectRoot);

		process.start(m_pythonExecutable, finalArgs);
		if (!process.waitForStarted()) {
			result.stdErr = QString("Failed to start Python interpreter: %1").arg(m_pythonExecutable);
			return result;
		}

		result.started = true;
		if (!process.waitForFinished(timeoutMs)) {
			result.timedOut = true;
			process.kill();
			process.waitForFinished();
		}

		result.exitCode = process.exitCode();
		result.stdOut = QString::fromUtf8(process.readAllStandardOutput());
		result.stdErr = QString::fromUtf8(process.readAllStandardError());
		if (result.timedOut && result.stdErr.trimmed().isEmpty()) {
			result.stdErr = QString("Python process timed out");
		}

		return result;
	}

	ScriptExecutionResult PythonScriptRunner::executeScriptWithFiles(
		const QString& scriptPath,
		const QString& inputFilePath,
		const QString& outputFilePath,
		const QStringList& extraArguments,
		int timeoutMs
	) const {
		QStringList arguments;
		arguments << "--input-file" << inputFilePath;
		arguments << "--output-file" << outputFilePath;
		arguments << extraArguments;
		return executeScript(scriptPath, arguments, timeoutMs);
	}

	ScriptExecutionResult PythonScriptRunner::executeScriptWithJson(
		const QString& scriptPath,
		const QJsonDocument& requestJson,
		QJsonDocument& responseJson,
		int timeoutMs
	) const {
		ScriptExecutionResult result;

		QTemporaryFile inputFile(QDir::tempPath() + "/ecim_request_XXXXXX.json");
		QTemporaryFile outputFile(QDir::tempPath() + "/ecim_response_XXXXXX.json");
		if (!inputFile.open() || !outputFile.open()) {
			result.stdErr = QString("Failed to create temporary files for JSON communication");
			return result;
		}

		inputFile.write(requestJson.toJson(QJsonDocument::Compact));
		inputFile.flush();
		inputFile.close();
		outputFile.close();

		result = executeScriptWithFiles(scriptPath, inputFile.fileName(), outputFile.fileName(), {}, timeoutMs);
		if (!result.ok()) {
			if (result.stdErr.trimmed().isEmpty()) {
				result.stdErr = _trimmedStdErr(result.stdErr);
			}
			return result;
		}

		QFile jsonOutput(outputFile.fileName());
		if (!jsonOutput.open(QIODevice::ReadOnly)) {
			result.exitCode = -1;
			result.stdErr = QString("Could not open JSON output file produced by script");
			return result;
		}

		QJsonParseError parseError;
		const QJsonDocument parsed = QJsonDocument::fromJson(jsonOutput.readAll(), &parseError);
		if (parseError.error != QJsonParseError::NoError) {
			result.exitCode = -1;
			result.stdErr = QString("Invalid JSON output: %1").arg(parseError.errorString());
			return result;
		}

		responseJson = parsed;
		return result;
	}

	QString PythonScriptRunner::_resolveScriptPath(const QString& scriptPath) const {
		const QFileInfo fileInfo(scriptPath);
		if (fileInfo.isAbsolute()) {
			return scriptPath;
		}

		return QDir(m_projectRoot).filePath(scriptPath);
	}
}
