#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <QObject>
#include <QProcess>
#include <QStringList>
#include <QVariantMap>

enum class ToolMode {
    CompressMedia,
    PdfToImage,
    ImagesToPdf,
    SplitPdf,
    ExtractAudio,
    AudioConverter,
    VideoConverter
};

class ProcessManager : public QObject {
    Q_OBJECT
public:
    explicit ProcessManager(ToolMode mode, const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings, QObject *parent = nullptr);
    ~ProcessManager();

public slots:
    void start();
    void cancel();

signals:
    void started();
    void finished(bool success, const QString& error);
    void canceled();

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);

private:
    ToolMode m_mode;
    QStringList m_inputPaths;
    QString m_outputPath;
    QVariantMap m_settings;
    QProcess* m_process;
    bool m_canceled;

    void processCompressMedia();
    void processPdfToImage();
    void processImagesToPdf();
    void processSplitPdf();
    void processExtractAudio();
    void processAudioConverter();
    void processVideoConverter();

    // Helpers
    double getMediaDuration(const QString& path);
    QString mapSizeToPdfSetting(qint64 targetBytes);
};

#endif // PROCESSMANAGER_H
