#include "VideoConverterTool.h"
#include "VideoConverterWorker.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QFileInfo>

VideoConverterTool::VideoConverterTool(QObject* parent) : QObject(parent), m_formatBox(nullptr) {}

void VideoConverterTool::buildSettingsUi(QWidget* container) {
    if (!container->layout()) new QVBoxLayout(container);
    QHBoxLayout *h = new QHBoxLayout();
    QLabel *lbl = new QLabel("Convert To Container:");
    m_formatBox = new QComboBox();
    m_formatBox->addItems({"mp4", "mkv", "avi", "mov"});
    h->addWidget(lbl); h->addWidget(m_formatBox); h->addStretch();
    QWidget *w = new QWidget(); w->setLayout(h);
    container->layout()->addWidget(w);
}

QVariantMap VideoConverterTool::getSettings() const {
    QVariantMap map;
    if (m_formatBox) map["format"] = m_formatBox->currentText();
    return map;
}

BaseWorker* VideoConverterTool::createWorker(const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings) {
    return new VideoConverterWorker(inputPaths, outputPath, settings);
}

QString VideoConverterTool::getOutputSuggestion(const QString& firstInputPath, int filterMode) const {
    QFileInfo fi(firstInputPath);
    QString ext = m_formatBox ? m_formatBox->currentText() : "mp4";
    return fi.completeBaseName() + "_converted." + ext;
}
