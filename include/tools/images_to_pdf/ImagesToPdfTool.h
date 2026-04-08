#ifndef IMAGESTOPDFTOOL_H
#define IMAGESTOPDFTOOL_H
#include "ITool.h"
#include <QObject>

class ImagesToPdfTool : public QObject, public ITool {
    Q_OBJECT
public:
    explicit ImagesToPdfTool(QObject* parent = nullptr);
    QString getTitle() const override { return "Images to PDF"; }
    QString getIcon() const override { return "🖼️"; }
    QString getFilter() const override { return "Image Files (*.jpg *.jpeg *.png *.webp *.bmp)"; }
    void buildSettingsUi(QWidget* container) override;
    QVariantMap getSettings() const override;
    BaseWorker* createWorker(const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings) override;
    QString getOutputSuggestion(const QString& firstInputPath, int filterMode) const override;
};
#endif
