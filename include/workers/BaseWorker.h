#ifndef BASEWORKER_H
#define BASEWORKER_H

#include <QObject>
#include <QProcess>
#include <QStringList>
#include <QVariantMap>

class BaseWorker : public QObject {
    Q_OBJECT
public:
    explicit BaseWorker(const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings, QObject *parent = nullptr);
    virtual ~BaseWorker();

public slots:
    virtual void start() = 0;
    virtual void cancel();

signals:
    void started();
    void finished(bool success, const QString& error);
    void canceled();

public slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);

protected:
    QStringList m_inputPaths;
    QString m_outputPath;
    QVariantMap m_settings;
    QProcess* m_process;
    bool m_canceled;

    double getMediaDuration(const QString& path);
};

#endif // BASEWORKER_H
