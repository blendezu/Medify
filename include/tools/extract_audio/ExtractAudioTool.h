#ifndef EXTRACTAUDIOTOOL_H
#define EXTRACTAUDIOTOOL_H
#include "ITool.h"
#include <QComboBox>
#include <QObject>

class ExtractAudioTool : public QObject, public ITool {
    Q_OBJECT
public:
    explicit ExtractAudioTool(QObject* parent = nullptr);
    QString getTitle() const override { return "Extract Audio"; }
    QString getIcon() const override { return "🎵"; }
    QString getFilter() const override { return "Video Files (*.mp4 *.mkv *.avi *.mov)"; }
    void buildSettingsUi(QWidget* container) override;
    QVariantMap getSettings() const override;
    BaseWorker* createWorker(const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings) override;
    QString getOutputSuggestion(const QString& firstInputPath, int filterMode) const override;
private:
    QComboBox* m_formatBox;
};
#endif
