#ifndef BASETOOLWIDGET_H
#define BASETOOLWIDGET_H

#include <QWidget>
#include <QStackedWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QLineEdit>
#include <QThread>

#include "../tools/ITool.h"
#include "../workers/BaseWorker.h"

enum class MediaType { Unknown, Image, Audio, Video, Pdf };

class BaseToolWidget : public QWidget {
    Q_OBJECT
public:
    explicit BaseToolWidget(QWidget* parent = nullptr);
    void setTool(ITool* tool);

signals:
    void requestBackToDashboard();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void onSelectionChanged();
    void onProcessClicked();
    void onBrowseSaveDirectory();
    void onSelectFilesClicked();
    void onWorkerFinished(bool success, const QString& error);

private:
    ITool* m_currentTool;

    QLabel *m_toolTitleLabel;
    QStackedWidget *m_internalStack;
    QListWidget *m_listWidget;
    QWidget *m_settingsContainer;
    QLineEdit *m_saveDirectoryEdit;
    QProgressBar *m_progressBar;
    QPushButton *m_processButton;

    QThread* m_workerThread;
    BaseWorker* m_worker;

    void setupUi();
    void addFiles(const QStringList& files);
    MediaType getMediaType(const QString& filePath);
    QString formatSize(qint64 bytes);
};
#endif
