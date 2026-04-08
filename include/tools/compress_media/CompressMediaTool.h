#ifndef COMPRESSMEDIATOOL_H
#define COMPRESSMEDIATOOL_H

#include "ITool.h"
#include <QSlider>
#include <QLabel>
#include <QObject>

class CompressMediaTool : public QObject, public ITool {
    Q_OBJECT
public:
    explicit CompressMediaTool(QObject* parent = nullptr);
    
    QString getTitle() const override { return "Compress Media"; }
    QString getIcon() const override { return "🗜️"; }
    QString getFilter() const override { return "All Files (*)"; }

    void buildSettingsUi(QWidget* container) override;
    QVariantMap getSettings() const override;
    BaseWorker* createWorker(const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings) override;
    QString getOutputSuggestion(const QString& firstInputPath, int filterMode) const override;

public slots:
    void updateOriginalSize(qint64 size);

private slots:
    void onSliderMoved(int value);

private:
    QSlider* m_slider;
    QLabel* m_label;
    qint64 m_currentOriginalSize;

    QString formatSize(qint64 bytes) const;
    qint64 mapSliderToBytes(int sliderValue) const;
};

#endif
