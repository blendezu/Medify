#ifndef SPLITPDFWORKER_H
#define SPLITPDFWORKER_H
#include "BaseWorker.h"

class SplitPdfWorker : public BaseWorker {
    Q_OBJECT
public:
    explicit SplitPdfWorker(const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings, QObject *parent = nullptr);
    void start() override;
};
#endif
