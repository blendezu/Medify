#ifndef PDFTOIMAGEWORKER_H
#define PDFTOIMAGEWORKER_H
#include "BaseWorker.h"

class PdfToImageWorker : public BaseWorker {
    Q_OBJECT
public:
    explicit PdfToImageWorker(const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings, QObject *parent = nullptr);
    void start() override;
};
#endif
