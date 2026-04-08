#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QProgressBar>
#include <QLineEdit>
#include <QToolButton>
#include <QTimer>
#include <QTemporaryDir>
#include <QVariantMap>
#include <QComboBox>
#include <QGridLayout>

#include "ProcessManager.h"

enum class MediaType {
    Unknown,
    Image,
    Audio,
    Video,
    Pdf
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(bool hasFFmpeg, bool hasFfprobe, bool hasGs, QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void onSelectionChanged();
    void onTargetSliderMoved(int value);
    
    // Universal Router Slots
    void onDashboardTileClicked(int modeId);
    void onBackToDashboard();

    void onFinalWorkerFinished(bool success, const QString& error);

    void onProcessAndSaveClicked();
    void onBrowseSaveDirectory();
    void onSelectFilesClicked();

private:
    bool m_hasFFmpeg;
    bool m_hasFfprobe;
    bool m_hasGs;

    QStackedWidget *m_stackedWidget;
    
    // Core Layouts
    QWidget *m_dashboardWidget;
    QWidget *m_toolWidgetShell;
    
    // Tool Frame Elements
    QLabel *m_toolTitleLabel;
    QStackedWidget *m_toolInternalStack; // Drop zone vs List
    QWidget *m_dropZoneWidget;
    QListWidget *m_listWidget;
    
    QWidget *m_settingsContainer;
    QVBoxLayout *m_settingsLayout;

    QPushButton *m_selectFilesButton;
    QPushButton *m_processButton;
    QProgressBar *m_progressBar;
    QLineEdit *m_saveDirectoryEdit;
    QToolButton *m_browseButton;

    // Concurrency
    QThread* m_finalThread;
    ProcessManager* m_finalWorker;
    
    qint64 m_currentOriginalSize = 0;
    ToolMode m_activeMode;

    // Tool Specific Inputs
    QSlider *m_targetSizeSlider;
    QLabel *m_targetSizeLabel;
    QLineEdit *m_pdfPageRangeEdit;
    QLineEdit *m_pdfDpiEdit;
    QComboBox *m_audioFormatBox;
    QComboBox *m_videoFormatBox;

    // Helpers
    MediaType getMediaType(const QString& filePath);
    QString formatSize(qint64 bytes);
    qint64 mapSliderToBytes(int sliderValue);
    
    void setupUi();
    void applyStyles();
    void addFiles(const QStringList& files);
    
    void buildDashboard();
    void buildToolFrame();
    void loadToolSettings(ToolMode mode);
};

#endif // MAINWINDOW_H
