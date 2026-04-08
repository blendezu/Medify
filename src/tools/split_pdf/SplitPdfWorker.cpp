#include "SplitPdfWorker.h"
#include <QFileInfo>

SplitPdfWorker::SplitPdfWorker(const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings, QObject *parent)
    : BaseWorker(inputPaths, outputPath, settings, parent) {}

void SplitPdfWorker::start() {
    emit started();
    if (m_canceled || m_inputPaths.isEmpty()) { emit finished(false, "No input"); return; }

    // Resolve gs binary
    QString gsCommand = "gs";
    for (const QString& p : {"/opt/homebrew/bin/gs", "/usr/local/bin/gs", "gs"}) {
        if (QFileInfo::exists(p)) { gsCommand = p; break; }
    }

    QVariantList ranges = m_settings.value("ranges").toList();
    QString baseName   = m_settings.value("baseName", "split").toString();
    QString outputDir  = m_outputPath;

    if (ranges.isEmpty()) { emit finished(false, "No ranges defined"); return; }

    for (const QVariant& v : ranges) {
        if (m_canceled) return;

        QVariantMap r = v.toMap();
        int from = r.value("from", 1).toInt();
        int to   = r.value("to",   1).toInt();

        QString outputFile = outputDir + "/" + baseName + "_" +
                             QString::number(from) + "-" + QString::number(to) + ".pdf";

        QProcess proc;
        proc.setProcessChannelMode(QProcess::SeparateChannels);
        QStringList args;
        args << "-sDEVICE=pdfwrite" << "-dNOPAUSE" << "-dBATCH"
             << "-dCompatibilityLevel=1.4"
             << ("-dFirstPage=" + QString::number(from))
             << ("-dLastPage="  + QString::number(to))
             << ("-sOutputFile=" + outputFile)
             << m_inputPaths.first();

        proc.start(gsCommand, args);
        if (!proc.waitForFinished(120000)) {
            emit finished(false, "Timeout on range " + QString::number(from) + "-" + QString::number(to));
            return;
        }
        if (proc.exitCode() != 0) {
            QString err = proc.readAllStandardError().trimmed();
            emit finished(false, "Failed on range " + QString::number(from) + "-" +
                          QString::number(to) + ":\n" + err);
            return;
        }
    }

    emit finished(true, "");
}
