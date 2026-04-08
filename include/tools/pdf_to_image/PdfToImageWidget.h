#ifndef PDFTOIMAGEWIDGET_H
#define PDFTOIMAGEWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QProgressBar>
#include <QLabel>
#include <QStackedWidget>
#include <QThread>
#include <QPdfDocument>
#include "PdfToImageWorker.h"

class PdfToImageWidget : public QWidget {
    Q_OBJECT
public:
    explicit PdfToImageWidget(QWidget* parent = nullptr);

signals:
    void requestBackToDashboard();

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    void onSelectFileClicked();
    void onBrowseSaveDirectory();
    void onProcessClicked();
    void onWorkerFinished(bool success, const QString& error);
    void onPageDoubleClicked(QListWidgetItem* item);
    void onZoomChanged(int value);

private:
    void setupUi();
    void loadPdf(const QString& path);
    QImage renderPageImage(int pageIndex, int targetHeight);
    void applyZoom(int thumbH);

    int m_currentThumbH;

    QString m_currentPdfPath;
    QPdfDocument* m_pdfDocument;

    QStackedWidget* m_internalStack;
    QListWidget* m_thumbnailList;
    QLabel* m_titleLabel;
    QLabel* m_pageCountLabel;
    QSlider* m_zoomSlider;
    QLabel* m_zoomLabel;
    QLineEdit* m_baseNameEdit;
    QLineEdit* m_dpiEdit;
    QLineEdit* m_saveDirectoryEdit;
    QProgressBar* m_progressBar;
    QPushButton* m_processButton;

    QThread* m_workerThread;
    PdfToImageWorker* m_worker;
};

#endif // PDFTOIMAGEWIDGET_H
