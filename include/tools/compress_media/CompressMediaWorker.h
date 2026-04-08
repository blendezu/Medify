#ifndef COMPRESSMEDIAWORKER_H
#define COMPRESSMEDIAWORKER_H

#include "BaseWorker.h"

class CompressMediaWorker : public BaseWorker {
    Q_OBJECT
public:
    explicit CompressMediaWorker(const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings, QObject *parent = nullptr);
    void start() override;
private:
    QString mapSizeToPdfSetting(qint64 targetBytes);
};

#endif
