#include "PdfToImageWorker.h"
#include <QFileInfo>

PdfToImageWorker::PdfToImageWorker(const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings, QObject *parent)
    : BaseWorker(inputPaths, outputPath, settings, parent) {}

void PdfToImageWorker::start() {
    emit started();
    if (m_canceled || m_inputPaths.isEmpty()) { emit finished(false, "No input"); return; }

    // Find gs (Ghostscript) – check Homebrew paths on macOS first
    QString gsCommand = "gs";
    QStringList gsCandidates = {
        "/opt/homebrew/bin/gs",
        "/usr/local/bin/gs",
        "gs"
    };
#ifdef Q_OS_WIN
    gsCandidates = {"gswin64c", "gswin32c"};
#endif
    for (const QString& candidate : gsCandidates) {
        if (QFileInfo::exists(candidate)) { gsCommand = candidate; break; }
    }

    QString dpi = m_settings.value("dpi", "150").toString();
    QVariantList selectedPages = m_settings.value("selectedPages").toList();
    QString baseName = m_settings.value("baseName", "page").toString();

    QFileInfo outInfo(m_outputPath);
    QString outputDir = (!outInfo.suffix().isEmpty() && !outInfo.isDir())
                        ? outInfo.path()
                        : m_outputPath;

    if (baseName == "page" && !outInfo.suffix().isEmpty() && !outInfo.isDir()) {
        baseName = outInfo.completeBaseName();
    }

    QStringList baseArgs;
    baseArgs << "-dSAFER" << "-dBATCH" << "-dNOPAUSE"
             << "-sDEVICE=png16m" << ("-r" + dpi);

    if (selectedPages.isEmpty()) {
        // Export all pages at once — Ghostscript numbers them sequentially
        m_process = new QProcess(this);
        m_process->setProcessChannelMode(QProcess::SeparateChannels);
        connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &BaseWorker::onProcessFinished);
        connect(m_process, &QProcess::errorOccurred, this, &BaseWorker::onProcessError);

        QStringList args = baseArgs;
        args << ("-sOutputFile=" + outputDir + "/" + baseName + "_%03d.png");
        args << m_inputPaths.first();
        m_process->start(gsCommand, args);

    } else {
        // Export each selected page individually so the real page number ends up in the filename
        // e.g. page 3 → baseName_3.png
        for (const QVariant& v : selectedPages) {
            if (m_canceled) return;

            int pageNum = v.toInt();
            QString outputFile = outputDir + "/" + baseName + "_" + QString::number(pageNum) + ".png";

            QProcess proc;
            proc.setProcessChannelMode(QProcess::SeparateChannels);
            QStringList args = baseArgs;
            args << ("-dFirstPage=" + QString::number(pageNum));
            args << ("-dLastPage=" + QString::number(pageNum));
            args << ("-sOutputFile=" + outputFile);
            args << m_inputPaths.first();

            proc.start(gsCommand, args);
            if (!proc.waitForFinished(60000)) {
                emit finished(false, "Timeout exporting page " + QString::number(pageNum));
                return;
            }
            if (proc.exitCode() != 0) {
                QString err = proc.readAllStandardError().trimmed();
                emit finished(false, "Failed on page " + QString::number(pageNum) + ":\n" + err);
                return;
            }
        }
        emit finished(true, "");
    }
}

