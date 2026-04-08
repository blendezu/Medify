#include "SplitPdfTool.h"
#include "SplitPdfWorker.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QFileInfo>

SplitPdfTool::SplitPdfTool(QObject* parent) : QObject(parent), m_pageRangeEdit(nullptr) {}

void SplitPdfTool::buildSettingsUi(QWidget* container) {
    if (!container->layout()) new QVBoxLayout(container);
    QHBoxLayout *h = new QHBoxLayout();
    QLabel *lbl = new QLabel("Extract Page Range:");
    m_pageRangeEdit = new QLineEdit();
    m_pageRangeEdit->setPlaceholderText("e.g. 2-5");
    h->addWidget(lbl); h->addWidget(m_pageRangeEdit);
    QWidget *w = new QWidget(); w->setLayout(h);
    container->layout()->addWidget(w);
}

QVariantMap SplitPdfTool::getSettings() const {
    QVariantMap map;
    if (m_pageRangeEdit) map["pageRange"] = m_pageRangeEdit->text();
    return map;
}

BaseWorker* SplitPdfTool::createWorker(const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings) {
    return new SplitPdfWorker(inputPaths, outputPath, settings);
}

QString SplitPdfTool::getOutputSuggestion(const QString& firstInputPath, int filterMode) const {
    QFileInfo fi(firstInputPath);
    return fi.completeBaseName() + "_split.pdf";
}
