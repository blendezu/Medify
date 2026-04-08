#include "VideoConverterTool.h"
#include "VideoConverterWorker.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QFileInfo>

VideoConverterTool::VideoConverterTool(QObject* parent) : QObject(parent), m_formatBox(nullptr), m_qualityBox(nullptr) {}

void VideoConverterTool::buildSettingsUi(QWidget* container) {
    if (!container->layout()) new QVBoxLayout(container);
    QHBoxLayout *h = new QHBoxLayout();
    QLabel *lbl = new QLabel("Convert To Container:");
    m_formatBox = new QComboBox();
    m_formatBox->addItems({"mp4", "mkv", "avi", "mov"});
    m_formatBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_formatBox->setMinimumWidth(90);

    QLabel *qlbl = new QLabel("Quality:");
    m_qualityBox = new QComboBox();
    m_qualityBox->addItems({"Low", "Medium", "High"});
    m_qualityBox->setCurrentIndex(1);
    m_qualityBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_qualityBox->setMinimumWidth(120);
    m_qualityBox->setMinimumContentsLength(8);

    h->addWidget(lbl); h->addWidget(m_formatBox);
    h->addSpacing(16);
    h->addWidget(qlbl); h->addWidget(m_qualityBox);
    h->addStretch();
    QWidget *w = new QWidget(); w->setLayout(h);
    container->layout()->addWidget(w);
}

QVariantMap VideoConverterTool::getSettings() const {
    QVariantMap map;
    if (m_formatBox)  map["format"]  = m_formatBox->currentText();
    if (m_qualityBox) map["quality"] = m_qualityBox->currentText();
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
