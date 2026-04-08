#ifndef SPLITPDFWIDGET_H
#define SPLITPDFWIDGET_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QLineEdit>
#include <QProgressBar>
#include <QStackedWidget>
#include <QThread>
#include <QSlider>
#include <QListWidget>
#include <QDialog>
#include <QPdfDocument>
#include "SplitPdfWorker.h"

class RangeRow : public QWidget {
    Q_OBJECT
public:
    explicit RangeRow(int index, int max, QWidget* parent = nullptr);
    int fromPage() const;
    int toPage()   const;
    void setIndex(int i);
    void setMax(int max);
    void setFromTo(int from, int to);
signals:
    void changed();
    void removeRequested();
private:
    QLabel*   m_label;
    QSpinBox* m_fromSpin;
    QSpinBox* m_toSpin;
};

class SplitPdfWidget : public QWidget {
    Q_OBJECT
public:
    explicit SplitPdfWidget(QWidget* parent = nullptr);
signals:
    void requestBackToDashboard();
protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
private slots:
    void onSelectFileClicked();
    void onBrowseSaveDirectory();
    void onAddRangeClicked();
    void onRangeChanged();
    void onZoomChanged(int value);
    void onPageDoubleClicked(QListWidgetItem* item);
    void onProcessClicked();
    void onWorkerFinished(bool success, const QString& error);
private:
    void setupUi();
    void loadPdf(const QString& path);
    QImage renderPageImage(int pageIndex, int targetH);
    QImage renderPageWithHighlight(int pageIndex, int targetH, int rangeIdx);
    void rebuildPageList();
    void removeRange(int index);
    void renumberRanges();

    QString       m_currentPdfPath;
    QPdfDocument* m_pdfDocument;
    int           m_currentThumbH;

    QStackedWidget* m_internalStack;

    // Left: all-pages grid
    QListWidget* m_pageList;
    QSlider*     m_zoomSlider;

    // Right: range rows
    QWidget*     m_rangeRowsContainer;
    QVBoxLayout* m_rangeRowsLayout;
    QList<RangeRow*> m_rows;

    // Bottom
    QLineEdit*    m_baseNameEdit;
    QLineEdit*    m_saveDirectoryEdit;
    QProgressBar* m_progressBar;
    QPushButton*  m_processButton;

    QThread*        m_workerThread;
    SplitPdfWorker* m_worker;
};

#endif // SPLITPDFWIDGET_H
