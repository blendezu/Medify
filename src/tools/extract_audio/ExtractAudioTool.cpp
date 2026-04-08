#include "ExtractAudioTool.h"
#include "ExtractAudioWorker.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QFileInfo>

ExtractAudioTool::ExtractAudioTool(QObject* parent) : QObject(parent), m_formatBox(nullptr) {}

void ExtractAudioTool::buildSettingsUi(QWidget* container) {
    if (!container->layout()) new QVBoxLayout(container);
    QHBoxLayout *h = new QHBoxLayout();
    QLabel *lbl = new QLabel("Convert To:");
    m_formatBox = new QComboBox();
    m_formatBox->addItems({"mp3", "wav"});
    h->addWidget(lbl); h->addWidget(m_formatBox); h->addStretch();
    QWidget *w = new QWidget(); w->setLayout(h);
    container->layout()->addWidget(w);
}

QVariantMap ExtractAudioTool::getSettings() const {
    QVariantMap map;
    if (m_formatBox) map["format"] = m_formatBox->currentText();
    return map;
}

BaseWorker* ExtractAudioTool::createWorker(const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings) {
    return new ExtractAudioWorker(inputPaths, outputPath, settings);
}

QString ExtractAudioTool::getOutputSuggestion(const QString& firstInputPath, int filterMode) const {
    QFileInfo fi(firstInputPath);
    QString ext = m_formatBox ? m_formatBox->currentText() : "mp3";
    return fi.completeBaseName() + "_audio." + ext;
}
