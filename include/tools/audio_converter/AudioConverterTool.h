#ifndef AUDIOCONVERTERTOOL_H
#define AUDIOCONVERTERTOOL_H
#include "ITool.h"
#include <QComboBox>
#include <QObject>

class AudioConverterTool : public QObject, public ITool {
    Q_OBJECT
public:
    explicit AudioConverterTool(QObject* parent = nullptr);
    QString getTitle() const override { return "Audio Converter"; }
    QString getIcon() const override { return "🎧"; }
    QString getFilter() const override { return "Audio Files (*.mp3 *.wav *.ogg *.flac)"; }
    void buildSettingsUi(QWidget* container) override;
    QVariantMap getSettings() const override;
    BaseWorker* createWorker(const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings) override;
    QString getOutputSuggestion(const QString& firstInputPath, int filterMode) const override;
private:
    QComboBox* m_formatBox;
};
#endif
