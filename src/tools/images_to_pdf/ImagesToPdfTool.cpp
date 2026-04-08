#include "ImagesToPdfTool.h"
#include "ImagesToPdfWorker.h"
#include <QVBoxLayout>
#include <QLabel>

ImagesToPdfTool::ImagesToPdfTool(QObject* parent) : QObject(parent) {}

void ImagesToPdfTool::buildSettingsUi(QWidget* container) {
    if (!container->layout()) new QVBoxLayout(container);
    QLabel *lbl = new QLabel("Upload multiple images. They will be ordered sequentially.");
    container->layout()->addWidget(lbl);
}

QVariantMap ImagesToPdfTool::getSettings() const { return QVariantMap(); }
BaseWorker* ImagesToPdfTool::createWorker(const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings) {
    return new ImagesToPdfWorker(inputPaths, outputPath, settings);
}
QString ImagesToPdfTool::getOutputSuggestion(const QString& firstInputPath, int filterMode) const {
    return "MergedImages.pdf";
}
