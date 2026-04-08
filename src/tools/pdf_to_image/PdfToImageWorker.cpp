#include "PdfToImageWorker.h"
#include <QFileInfo>

PdfToImageWorker::PdfToImageWorker(const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings, QObject *parent)
    : BaseWorker(inputPaths, outputPath, settings, parent) {}

void PdfToImageWorker::start() {
    emit started();
    if (m_canceled || m_inputPaths.isEmpty()) { emit finished(false, "No input"); return; }
    
    m_process = new QProcess(this);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &BaseWorker::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred, this, &BaseWorker::onProcessError);
    
    QString gsCommand = "gs";
    #ifdef Q_OS_WIN
    gsCommand = "gswin64c";
    #endif
    
    QString dpi = m_settings.value("dpi", "300").toString();
    QString pageRange = m_settings.value("pageRange", "").toString();
    
    QStringList args;
    args << "-dSAFER" << "-dBATCH" << "-dNOPAUSE" << "-sDEVICE=png16m" << "-r" + dpi;
    if (!pageRange.isEmpty() && pageRange.contains("-")) {
        QStringList parts = pageRange.split("-");
        if (parts.size() == 2) {
            args << "-dFirstPage=" + parts[0].trimmed();
            args << "-dLastPage=" + parts[1].trimmed();
        }
    }
    QString finalOutputPath = m_outputPath;
    if (!finalOutputPath.contains("%")) {
        QFileInfo fi(finalOutputPath);
        finalOutputPath = fi.path() + "/" + fi.completeBaseName() + "_%03d." + fi.suffix();
    }
    args << "-sOutputFile=" + finalOutputPath << m_inputPaths.first();
    m_process->start(gsCommand, args);
}
