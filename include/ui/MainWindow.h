#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QList>
#include "BaseToolWidget.h"
#include "../tools/ITool.h"

class PdfToImageWidget;
class SplitPdfWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(bool hasFFmpeg, bool hasFfprobe, bool hasGs, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onDashboardTileClicked(int index);
    void onBackToDashboard();

private:
    void setupUi();
    void buildDashboard();
    void applyStyles();

    QStackedWidget *m_stackedWidget;
    QWidget *m_dashboardWidget;
    BaseToolWidget *m_baseToolWidget;
    PdfToImageWidget *m_pdfToImageWidget;
    SplitPdfWidget *m_splitPdfWidget;
    
    QList<ITool*> m_tools;
};
#endif
