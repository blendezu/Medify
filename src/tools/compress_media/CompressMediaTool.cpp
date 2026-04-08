#include "CompressMediaTool.h"
#include "CompressMediaWorker.h"
#include <QHBoxLayout>
#include <QFileInfo>

CompressMediaTool::CompressMediaTool(QObject* parent) : QObject(parent), m_slider(nullptr), m_label(nullptr), m_currentOriginalSize(0) {}

void CompressMediaTool::buildSettingsUi(QWidget* container) {
    if (!container->layout()) {
        new QVBoxLayout(container);
    }
    QHBoxLayout *h = new QHBoxLayout();
    QLabel *lbl = new QLabel("Target Size:");
    lbl->setFixedWidth(100);
    m_slider = new QSlider(Qt::Horizontal);
    m_slider->setRange(0, 100);
    m_slider->setValue(80);
    connect(m_slider, &QSlider::valueChanged, this, &CompressMediaTool::onSliderMoved);
    
    m_label = new QLabel("-");
    m_label->setFixedWidth(80);
    
    h->addWidget(lbl); 
    h->addWidget(m_slider); 
    h->addWidget(m_label);
    
    QWidget *w = new QWidget();
    w->setLayout(h);
    container->layout()->addWidget(w);
}

QVariantMap CompressMediaTool::getSettings() const {
    QVariantMap map;
    map["targetBytes"] = mapSliderToBytes(m_slider ? m_slider->value() : 80);
    return map;
}

BaseWorker* CompressMediaTool::createWorker(const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings) {
    return new CompressMediaWorker(inputPaths, outputPath, settings);
}

QString CompressMediaTool::getOutputSuggestion(const QString& firstInputPath, int filterMode) const {
    QFileInfo fi(firstInputPath);
    return fi.completeBaseName() + "_compressed." + fi.suffix();
}

void CompressMediaTool::updateOriginalSize(qint64 size) {
    m_currentOriginalSize = size;
    if (m_slider) {
        onSliderMoved(m_slider->value());
    }
}

void CompressMediaTool::onSliderMoved(int value) {
    if (m_label) {
        m_label->setText("~ " + formatSize(mapSliderToBytes(value)));
    }
}

QString CompressMediaTool::formatSize(qint64 bytes) const {
    if (bytes < 1024) return QString::number(bytes) + " B";
    if (bytes < 1024 * 1024) return QString::number(bytes / 1024.0, 'f', 1) + " KB";
    if (bytes < 1024 * 1024 * 1024) return QString::number(bytes / (1024.0 * 1024.0), 'f', 2) + " MB";
    return QString::number(bytes / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
}

qint64 CompressMediaTool::mapSliderToBytes(int sliderValue) const {
    if (m_currentOriginalSize <= 0) return 0;
    qint64 minSize = m_currentOriginalSize * 0.01;
    return minSize + (m_currentOriginalSize - minSize) * sliderValue / 100;
}
