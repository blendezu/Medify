#include "AudioConverterWorker.h"

AudioConverterWorker::AudioConverterWorker(const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings, QObject *parent)
    : BaseWorker(inputPaths, outputPath, settings, parent) {}

void AudioConverterWorker::start() {
    emit started();
    if (m_canceled || m_inputPaths.isEmpty()) { emit finished(false, "No input"); return; }
    
    m_process = new QProcess(this);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &BaseWorker::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred, this, &BaseWorker::onProcessError);
    
    QString format = m_settings.value("format", "mp3").toString(); 
    QStringList args;
    args << "-y" << "-i" << m_inputPaths.first() << "-vn";
    if (format.toLower() == "wav") {
        args << "-acodec" << "pcm_s16le";
    } else {
        args << "-acodec" << "libmp3lame" << "-q:a" << "2"; 
    }
    args << m_outputPath;
    m_process->start("ffmpeg", args);
}
