#ifndef AUDIOCONVERTERWORKER_H
#define AUDIOCONVERTERWORKER_H
#include "BaseWorker.h"

class AudioConverterWorker : public BaseWorker {
    Q_OBJECT
public:
    explicit AudioConverterWorker(const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings, QObject *parent = nullptr);
    void start() override;
};
#endif
