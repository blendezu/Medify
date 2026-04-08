#include "VideoConverterWorker.h"

VideoConverterWorker::VideoConverterWorker(const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings, QObject *parent)
    : BaseWorker(inputPaths, outputPath, settings, parent) {}

void VideoConverterWorker::start() {
    emit started();
    if (m_canceled || m_inputPaths.isEmpty()) { emit finished(false, "No input"); return; }
    
    m_process = new QProcess(this);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &BaseWorker::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred, this, &BaseWorker::onProcessError);
    
    QString quality = m_settings.value("quality", "Medium").toString();
    QString crf = "23"; // Medium
    if (quality == "Low")  crf = "28";
    if (quality == "High") crf = "18";

    QStringList args;
    args << "-y" << "-i" << m_inputPaths.first();
    args << "-c:v" << "libx264" << "-crf" << crf << "-preset" << "medium"
         << "-c:a" << "aac" << "-b:a" << "128k";
    args << m_outputPath;
    m_process->start("ffmpeg", args);
}
