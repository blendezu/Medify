#ifndef ITOOL_H
#define ITOOL_H

#include <QString>
#include <QVariantMap>
#include <QWidget>

class BaseWorker;

class ITool {
public:
    virtual ~ITool() = default;

    virtual QString getTitle() const = 0;
    virtual QString getIcon() const = 0;
    virtual QString getFilter() const = 0;

    // Build the dynamic UI elements specific to this tool
    virtual void buildSettingsUi(QWidget* container) = 0;
    
    // Read the settings from those elements
    virtual QVariantMap getSettings() const = 0;
    
    // Create the worker configured with these settings
    virtual BaseWorker* createWorker(const QStringList& inputPaths, const QString& outputPath, const QVariantMap& settings) = 0;
    
    // Provide a file name suggestion
    virtual QString getOutputSuggestion(const QString& firstInputPath, int filterMode = 0) const = 0;
};

#endif // ITOOL_H
