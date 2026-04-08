#ifndef VIDEOCONVERTERWORKER_H
#define VIDEOCONVERTERWORKER_H
#include "BaseWorker.h"

class VideoConverterWorker : public BaseWorker {
    Q_OBJECT
public:
    explicit VideoConverterWorker(const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings, QObject *parent = nullptr);
    void start() override;
};
#endif
