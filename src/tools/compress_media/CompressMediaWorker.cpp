#include "CompressMediaWorker.h"
#include <QImageReader>
#include <QImageWriter>
#include <QBuffer>
#include <QFile>

CompressMediaWorker::CompressMediaWorker(const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings, QObject *parent)
    : BaseWorker(inputPaths, outputPath, settings, parent) {}

QString CompressMediaWorker::mapSizeToPdfSetting(qint64 targetBytes) {
    if (targetBytes <= 1024 * 1024) return "/screen";
    if (targetBytes <= 5 * 1024 * 1024) return "/ebook";
    if (targetBytes <= 20 * 1024 * 1024) return "/printer";
    return "/prepress";
}

void CompressMediaWorker::start() {
    emit started();
    if (m_canceled || m_inputPaths.isEmpty()) {
        emit finished(false, "Canceled or no input");
        return;
    }
    
    QString inputPath = m_inputPaths.first();
    qint64 targetBytes = m_settings.value("targetBytes", 0).toLongLong();

    if (inputPath.endsWith(".pdf", Qt::CaseInsensitive)) {
        m_process = new QProcess(this);
        connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &BaseWorker::onProcessFinished);
        connect(m_process, &QProcess::errorOccurred, this, &BaseWorker::onProcessError);
        
        QString gsCommand = "gs";
        #ifdef Q_OS_WIN
        gsCommand = "gswin64c";
        #endif
        
        QStringList args;
        args << "-sDEVICE=pdfwrite" << "-dCompatibilityLevel=1.4" 
             << "-dPDFSETTINGS=" + mapSizeToPdfSetting(targetBytes) 
             << "-dNOPAUSE" << "-dBATCH" 
             << "-sOutputFile=" + m_outputPath << inputPath;
        m_process->start(gsCommand, args);
        return;
    } 
    
    // Video handling
    if (inputPath.endsWith(".mp4", Qt::CaseInsensitive) || inputPath.endsWith(".mkv", Qt::CaseInsensitive)) {
        m_process = new QProcess(this);
        connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &BaseWorker::onProcessFinished);
        connect(m_process, &QProcess::errorOccurred, this, &BaseWorker::onProcessError);
        
        double duration = getMediaDuration(inputPath);
        if (duration <= 0) duration = 1.0;
        
        qint64 targetBits = targetBytes * 8;
        qint64 totalBitrate = targetBits / duration;
        
        qint64 audioBitrate = 128000;
        qint64 videoBitrate = totalBitrate - audioBitrate;
        
        if (videoBitrate < 50000) {
            audioBitrate = totalBitrate / 3;
            videoBitrate = totalBitrate - audioBitrate;
            if (videoBitrate < 1000) videoBitrate = 1000;
        }
        
        QStringList args;
        args << "-y" << "-i" << inputPath 
             << "-c:v" << "libx264" << "-b:v" << QString::number(videoBitrate) 
             << "-maxrate" << QString::number(videoBitrate * 2) << "-bufsize" << QString::number(videoBitrate * 4) 
             << "-c:a" << "aac" << "-b:a" << QString::number(audioBitrate) << m_outputPath;
        m_process->start("ffmpeg", args);
        return;
    }

    // Audio handling
    if (inputPath.endsWith(".mp3", Qt::CaseInsensitive) || inputPath.endsWith(".wav", Qt::CaseInsensitive)) {
        m_process = new QProcess(this);
        connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &BaseWorker::onProcessFinished);
        connect(m_process, &QProcess::errorOccurred, this, &BaseWorker::onProcessError);
        
        double duration = getMediaDuration(inputPath);
        if (duration <= 0) duration = 1.0;
        
        qint64 targetBits = targetBytes * 8;
        qint64 targetBitrate = targetBits / duration;
        if (targetBitrate < 16000) targetBitrate = 16000;
        
        QStringList args;
        args << "-y" << "-i" << inputPath << "-b:a" << QString::number(targetBitrate) << m_outputPath;
        m_process->start("ffmpeg", args);
        return;
    }

    // Image handling
    QImageReader reader(inputPath);
    QImage image = reader.read();
    if (image.isNull()) { emit finished(false, "Failed to read image"); return; }

    qint64 bestDiff = -1;
    QByteArray bestData;
    int low = 1, high = 100;
    
    while (low <= high && !m_canceled) {
        int mid = low + (high - low) / 2;
        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        QImageWriter writer(&buffer, "jpeg");
        writer.setQuality(mid);
        writer.write(image);
        
        qint64 currentSize = ba.size();
        qint64 diff = qAbs(currentSize - targetBytes);
        
        if (bestDiff == -1 || diff < bestDiff) {
            bestDiff = diff;
            bestData = ba;
        }
        if (currentSize == targetBytes) break;
        if (currentSize < targetBytes) { low = mid + 1; }
        else { high = mid - 1; }
    }
    
    if (m_canceled) return;
    QFile outFile(m_outputPath);
    if (!outFile.open(QIODevice::WriteOnly)) { emit finished(false, "Failed to write output image"); return; }
    outFile.write(bestData);
    outFile.close();
    emit finished(true, "");
}
