#include "ImagesToPdfWorker.h"
#include <QPdfWriter>
#include <QPainter>
#include <QImage>

ImagesToPdfWorker::ImagesToPdfWorker(const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings, QObject *parent)
    : BaseWorker(inputPaths, outputPath, settings, parent) {}

void ImagesToPdfWorker::start() {
    emit started();
    if (m_canceled || m_inputPaths.isEmpty()) { emit finished(false, "No input"); return; }
    
    QPdfWriter pdfWriter(m_outputPath);
    pdfWriter.setResolution(300);
    pdfWriter.setCreator("Medify Native Engine");
    
    QPainter painter;
    if (!painter.begin(&pdfWriter)) {
        emit finished(false, "Could not begin painting on PDF.");
        return;
    }
    
    for (int i = 0; i < m_inputPaths.size(); ++i) {
        if (m_canceled) { painter.end(); return; }
        QImage img(m_inputPaths[i]);
        if (img.isNull()) continue;
        if (i > 0) pdfWriter.newPage();
        QRect targetRect(0, 0, pdfWriter.width(), pdfWriter.height());
        painter.drawImage(targetRect, img);
    }
    
    painter.end();
    emit finished(true, "");
}
