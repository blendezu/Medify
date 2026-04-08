#ifndef PDFTOIMAGETOOL_H
#define PDFTOIMAGETOOL_H
#include "ITool.h"
#include <QLineEdit>
#include <QObject>

class PdfToImageTool : public QObject, public ITool {
    Q_OBJECT
public:
    explicit PdfToImageTool(QObject* parent = nullptr);
    QString getTitle() const override { return "PDF to Images"; }
    QString getIcon() const override { return "📄"; }
    QString getFilter() const override { return "PDF Files (*.pdf)"; }
    void buildSettingsUi(QWidget* container) override;
    QVariantMap getSettings() const override;
    BaseWorker* createWorker(const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings) override;
    QString getOutputSuggestion(const QString& firstInputPath, int filterMode) const override;
private:
    QLineEdit* m_dpiEdit;
    QLineEdit* m_pageRangeEdit;
};
#endif
