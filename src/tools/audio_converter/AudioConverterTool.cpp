#include "AudioConverterTool.h"
#include "AudioConverterWorker.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QFileInfo>

AudioConverterTool::AudioConverterTool(QObject* parent) : QObject(parent), m_formatBox(nullptr), m_qualityBox(nullptr) {}

void AudioConverterTool::buildSettingsUi(QWidget* container) {
    if (!container->layout()) new QVBoxLayout(container);
    QHBoxLayout *h = new QHBoxLayout();
    QLabel *lbl = new QLabel("Convert To:");
    m_formatBox = new QComboBox();
    m_formatBox->addItems({"mp3", "wav", "ogg", "flac"});
    m_formatBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_formatBox->setMinimumWidth(90);

    QLabel *qlbl = new QLabel("Quality:");
    m_qualityBox = new QComboBox();
    m_qualityBox->addItems({"Low", "Medium", "High"});
    m_qualityBox->setCurrentIndex(1); // Medium default
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

QVariantMap AudioConverterTool::getSettings() const {
    QVariantMap map;
    if (m_formatBox)  map["format"]  = m_formatBox->currentText();
    if (m_qualityBox) map["quality"] = m_qualityBox->currentText();
    return map;
}

BaseWorker* AudioConverterTool::createWorker(const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings) {
    return new AudioConverterWorker(inputPaths, outputPath, settings);
}

QString AudioConverterTool::getOutputSuggestion(const QString& firstInputPath, int filterMode) const {
    QFileInfo fi(firstInputPath);
    QString ext = m_formatBox ? m_formatBox->currentText() : "mp3";
    return fi.completeBaseName() + "_converted." + ext;
}
