#include "BaseWorker.h"

BaseWorker::BaseWorker(const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings, QObject *parent)
    : QObject(parent), m_inputPaths(inputPaths), m_outputPath(outputPath), m_settings(settings), m_process(nullptr), m_canceled(false) {
}

BaseWorker::~BaseWorker() {
    if (m_process) {
        m_process->kill();
        m_process->waitForFinished();
    }
}

void BaseWorker::cancel() {
    m_canceled = true;
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
    }
    emit canceled();
}

void BaseWorker::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    if (m_canceled) return;
    if (exitStatus == QProcess::CrashExit) {
        emit finished(false, "Process crashed");
    } else if (exitCode != 0) {
        emit finished(false, QString("Process failed with exit code %1").arg(exitCode));
    } else {
        emit finished(true, "");
    }
}

void BaseWorker::onProcessError(QProcess::ProcessError error) {
    if (m_canceled) return;
    if (error == QProcess::Crashed && m_canceled) return;
    emit finished(false, "Failed to start process: " + m_process->errorString());
}

double BaseWorker::getMediaDuration(const QString& path) {
    QProcess process;
    QStringList args;
    args << "-v" << "error" << "-show_entries" << "format=duration" 
         << "-of" << "default=noprint_wrappers=1:nokey=1" << path;
    process.start("ffprobe", args);
    if (!process.waitForFinished(10000)) return 0.0;
    bool ok;
    double duration = process.readAllStandardOutput().trimmed().toDouble(&ok);
    return ok ? duration : 0.0;
}
