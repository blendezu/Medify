#include "ProcessManager.h"
#include <QImageReader>
#include <QImageWriter>
#include <QFileInfo>
#include <QDir>
#include <QThread>
#include <QBuffer>
#include <QPdfWriter>
#include <QPainter>
#include <QImage>

ProcessManager::ProcessManager(ToolMode mode, const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings, QObject *parent)
    : QObject(parent), m_mode(mode), m_inputPaths(inputPaths), m_outputPath(outputPath), m_settings(settings), m_process(nullptr), m_canceled(false) {
}

ProcessManager::~ProcessManager() {
    if (m_process) {
        m_process->kill();
        m_process->waitForFinished();
    }
}

void ProcessManager::start() {
    emit started();
    if (m_canceled) return;

    switch (m_mode) {
        case ToolMode::CompressMedia: processCompressMedia(); break;
        case ToolMode::PdfToImage: processPdfToImage(); break;
        case ToolMode::ImagesToPdf: processImagesToPdf(); break;
        case ToolMode::SplitPdf: processSplitPdf(); break;
        case ToolMode::ExtractAudio: processExtractAudio(); break;
        case ToolMode::AudioConverter: processAudioConverter(); break;
        case ToolMode::VideoConverter: processVideoConverter(); break;
        default: emit finished(false, "Unknown tool mode");
    }
}

void ProcessManager::cancel() {
    m_canceled = true;
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
    }
    emit canceled();
}

// ----------------------------------------------------
// Helpers
// ----------------------------------------------------
double ProcessManager::getMediaDuration(const QString& path) {
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

QString ProcessManager::mapSizeToPdfSetting(qint64 targetBytes) {
    if (targetBytes <= 1024 * 1024) return "/screen";       // < 1MB
    if (targetBytes <= 5 * 1024 * 1024) return "/ebook";    // < 5MB
    if (targetBytes <= 20 * 1024 * 1024) return "/printer"; // < 20MB
    return "/prepress";
}

// ----------------------------------------------------
// Process Jobs
// ----------------------------------------------------
void ProcessManager::processCompressMedia() {
    if (m_inputPaths.isEmpty()) { emit finished(false, "No input file"); return; }
    QString inputPath = m_inputPaths.first();
    qint64 targetBytes = m_settings.value("targetBytes", 0).toLongLong();

    if (inputPath.endsWith(".pdf", Qt::CaseInsensitive)) {
        m_process = new QProcess(this);
        connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &ProcessManager::onProcessFinished);
        connect(m_process, &QProcess::errorOccurred, this, &ProcessManager::onProcessError);
        
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
        connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &ProcessManager::onProcessFinished);
        connect(m_process, &QProcess::errorOccurred, this, &ProcessManager::onProcessError);
        
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
        connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &ProcessManager::onProcessFinished);
        connect(m_process, &QProcess::errorOccurred, this, &ProcessManager::onProcessError);
        
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

void ProcessManager::processPdfToImage() {
    if (m_inputPaths.isEmpty()) { emit finished(false, "No input file"); return; }
    
    m_process = new QProcess(this);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &ProcessManager::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred, this, &ProcessManager::onProcessError);
    
    QString gsCommand = "gs";
    #ifdef Q_OS_WIN
    gsCommand = "gswin64c";
    #endif
    
    QString dpi = m_settings.value("dpi", "300").toString();
    QString pageRange = m_settings.value("pageRange", "").toString();
    
    QStringList args;
    args << "-dSAFER" << "-dBATCH" << "-dNOPAUSE" 
         << "-sDEVICE=png16m" << "-r" + dpi;
         
    if (!pageRange.isEmpty() && pageRange.contains("-")) {
        QStringList parts = pageRange.split("-");
        if (parts.size() == 2) {
            args << "-dFirstPage=" + parts[0].trimmed();
            args << "-dLastPage=" + parts[1].trimmed();
        }
    }
    // For Ghostscript multi-page to images, use %03d format
    QString finalOutputPath = m_outputPath;
    if (!finalOutputPath.contains("%")) {
        QFileInfo fi(finalOutputPath);
        finalOutputPath = fi.path() + "/" + fi.completeBaseName() + "_%03d." + fi.suffix();
    }
    args << "-sOutputFile=" + finalOutputPath << m_inputPaths.first();
    m_process->start(gsCommand, args);
}

void ProcessManager::processImagesToPdf() {
    if (m_inputPaths.isEmpty()) { emit finished(false, "No input files"); return; }
    
    QPdfWriter pdfWriter(m_outputPath);
    pdfWriter.setResolution(300);
    pdfWriter.setCreator("Media Toolkit via Qt");
    
    QPainter painter;
    if (!painter.begin(&pdfWriter)) {
        emit finished(false, "Could not begin painting on PDF.");
        return;
    }
    
    for (int i = 0; i < m_inputPaths.size(); ++i) {
        if (m_canceled) {
            painter.end();
            return;
        }
        
        QImage img(m_inputPaths[i]);
        if (img.isNull()) continue;
        
        if (i > 0) pdfWriter.newPage();
        
        QRect targetRect(0, 0, pdfWriter.width(), pdfWriter.height());
        painter.drawImage(targetRect, img);
    }
    
    painter.end();
    emit finished(true, "");
}

void ProcessManager::processSplitPdf() {
    if (m_inputPaths.isEmpty()) { emit finished(false, "No input file"); return; }
    
    m_process = new QProcess(this);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &ProcessManager::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred, this, &ProcessManager::onProcessError);
    
    QString gsCommand = "gs";
    #ifdef Q_OS_WIN
    gsCommand = "gswin64c";
    #endif
    
    QString pageRange = m_settings.value("pageRange", "").toString();
    QStringList args;
    args << "-sDEVICE=pdfwrite" << "-dNOPAUSE" << "-dBATCH";
    
    if (!pageRange.isEmpty() && pageRange.contains("-")) {
        QStringList parts = pageRange.split("-");
        if (parts.size() == 2) {
            args << "-dFirstPage=" + parts[0].trimmed();
            args << "-dLastPage=" + parts[1].trimmed();
        }
    }
    args << "-sOutputFile=" + m_outputPath << m_inputPaths.first();
    m_process->start(gsCommand, args);
}

void ProcessManager::processExtractAudio() {
    if (m_inputPaths.isEmpty()) { emit finished(false, "No input file"); return; }
    
    m_process = new QProcess(this);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &ProcessManager::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred, this, &ProcessManager::onProcessError);
    
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

void ProcessManager::processAudioConverter() {
    processExtractAudio(); 
}

void ProcessManager::processVideoConverter() {
    if (m_inputPaths.isEmpty()) { emit finished(false, "No input file"); return; }
    
    m_process = new QProcess(this);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &ProcessManager::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred, this, &ProcessManager::onProcessError);
    
    QStringList args;
    args << "-y" << "-i" << m_inputPaths.first();
    args << "-c:v" << "libx264" << "-crf" << "21" << "-c:a" << "aac" << "-b:a" << "128k";
    args << m_outputPath;
    
    m_process->start("ffmpeg", args);
}

// ----------------------------------------------------
// Process Callbacks
// ----------------------------------------------------
void ProcessManager::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    if (m_canceled) return;
    if (exitStatus == QProcess::CrashExit) {
        emit finished(false, "Process crashed");
    } else if (exitCode != 0) {
        emit finished(false, QString("Process failed with exit code %1. Check the inputs.").arg(exitCode));
    } else {
        emit finished(true, "");
    }
}

void ProcessManager::onProcessError(QProcess::ProcessError error) {
    if (m_canceled) return;
    if (error == QProcess::Crashed && m_canceled) return;
    emit finished(false, "Failed to start process: " + m_process->errorString());
}
