#ifndef EXTRACTAUDIOWORKER_H
#define EXTRACTAUDIOWORKER_H
#include "BaseWorker.h"

class ExtractAudioWorker : public BaseWorker {
    Q_OBJECT
public:
    explicit ExtractAudioWorker(const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings, QObject *parent = nullptr);
    void start() override;
};
#endif
