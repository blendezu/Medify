#include "VideoConverterWorker.h"

VideoConverterWorker::VideoConverterWorker(const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings, QObject *parent)
    : BaseWorker(inputPaths, outputPath, settings, parent) {}

void VideoConverterWorker::start() {
    emit started();
    if (m_canceled || m_inputPaths.isEmpty()) { emit finished(false, "No input"); return; }
    
    m_process = new QProcess(this);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &BaseWorker::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred, this, &BaseWorker::onProcessError);
    
    QStringList args;
    args << "-y" << "-i" << m_inputPaths.first();
    args << "-c:v" << "libx264" << "-crf" << "21" << "-c:a" << "aac" << "-b:a" << "128k";
    args << m_outputPath;
    
    m_process->start("ffmpeg", args);
}
