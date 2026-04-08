#ifndef IMAGESTOPDFWORKER_H
#define IMAGESTOPDFWORKER_H
#include "BaseWorker.h"

class ImagesToPdfWorker : public BaseWorker {
    Q_OBJECT
public:
    explicit ImagesToPdfWorker(const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings, QObject *parent = nullptr);
    void start() override;
};
#endif
