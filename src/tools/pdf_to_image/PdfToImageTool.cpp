#include "PdfToImageTool.h"
#include "PdfToImageWorker.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QFileInfo>

PdfToImageTool::PdfToImageTool(QObject* parent) : QObject(parent), m_dpiEdit(nullptr), m_pageRangeEdit(nullptr) {}

void PdfToImageTool::buildSettingsUi(QWidget* container) {
    if (!container->layout()) new QVBoxLayout(container);
    QHBoxLayout *h = new QHBoxLayout();
    QLabel *l1 = new QLabel("DPI:");
    m_dpiEdit = new QLineEdit("300");
    m_dpiEdit->setFixedWidth(60);
    QLabel *l2 = new QLabel("Page Range (Opts):");
    m_pageRangeEdit = new QLineEdit();
    m_pageRangeEdit->setPlaceholderText("e.g. 1-5");
    h->addWidget(l1); h->addWidget(m_dpiEdit);
    h->addSpacing(20);
    h->addWidget(l2); h->addWidget(m_pageRangeEdit);
    QWidget *w = new QWidget(); w->setLayout(h);
    container->layout()->addWidget(w);
}

QVariantMap PdfToImageTool::getSettings() const {
    QVariantMap map;
    if (m_dpiEdit) map["dpi"] = m_dpiEdit->text();
    if (m_pageRangeEdit) map["pageRange"] = m_pageRangeEdit->text();
    return map;
}

BaseWorker* PdfToImageTool::createWorker(const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings) {
    return new PdfToImageWorker(inputPaths, outputPath, settings);
}

QString PdfToImageTool::getOutputSuggestion(const QString& firstInputPath, int filterMode) const {
    QFileInfo fi(firstInputPath);
    return fi.completeBaseName() + "_export.png"; // Backend will swap to %03d format
}
