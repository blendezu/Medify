#ifndef VIDEOCONVERTERTOOL_H
#define VIDEOCONVERTERTOOL_H
#include "ITool.h"
#include <QComboBox>
#include <QObject>

class VideoConverterTool : public QObject, public ITool {
    Q_OBJECT
public:
    explicit VideoConverterTool(QObject* parent = nullptr);
    QString getTitle() const override { return "Video Converter"; }
    QString getIcon() const override { return "🎬"; }
    QString getFilter() const override { return "Video Files (*.mp4 *.mkv *.avi *.mov)"; }
    void buildSettingsUi(QWidget* container) override;
    QVariantMap getSettings() const override;
    BaseWorker* createWorker(const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings) override;
    QString getOutputSuggestion(const QString& firstInputPath, int filterMode) const override;
private:
    QComboBox* m_formatBox;
    QComboBox* m_qualityBox;
};
#endif
