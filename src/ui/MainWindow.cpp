#include "MainWindow.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QToolButton>
#include <QLabel>

#include "../tools/compress_media/CompressMediaTool.h"
#include "../tools/pdf_to_image/PdfToImageTool.h"
#include "../tools/pdf_to_image/PdfToImageWidget.h"
#include "../tools/split_pdf/SplitPdfTool.h"
#include "../tools/extract_audio/ExtractAudioTool.h"
#include "../tools/audio_converter/AudioConverterTool.h"
#include "../tools/video_converter/VideoConverterTool.h"

MainWindow::MainWindow(bool hasFFmpeg, bool hasFfprobe, bool hasGs, QWidget *parent)
    : QMainWindow(parent)
{
    m_tools.append(new CompressMediaTool(this));
    m_tools.append(new PdfToImageTool(this));
    m_tools.append(new SplitPdfTool(this));
    m_tools.append(new ExtractAudioTool(this));
    m_tools.append(new AudioConverterTool(this));
    m_tools.append(new VideoConverterTool(this));

    setupUi();
    applyStyles();
}

MainWindow::~MainWindow() {
    qDeleteAll(m_tools);
    m_tools.clear();
}

void MainWindow::setupUi() {
    setWindowTitle("Medify");
    resize(700, 500);
    
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0,0,0,0);
    
    m_stackedWidget = new QStackedWidget(this);
    mainLayout->addWidget(m_stackedWidget);
    
    buildDashboard();
    
    m_baseToolWidget = new BaseToolWidget(this);
    connect(m_baseToolWidget, &BaseToolWidget::requestBackToDashboard, this, &MainWindow::onBackToDashboard);

    // Dedicated widget for PDF to Images (index 2 in stack)
    m_pdfToImageWidget = new PdfToImageWidget(this);
    connect(m_pdfToImageWidget, &PdfToImageWidget::requestBackToDashboard, this, &MainWindow::onBackToDashboard);
    
    m_stackedWidget->addWidget(m_dashboardWidget);  // index 0
    m_stackedWidget->addWidget(m_baseToolWidget);    // index 1
    m_stackedWidget->addWidget(m_pdfToImageWidget);  // index 2
    
    m_stackedWidget->setCurrentIndex(0);
}

void MainWindow::buildDashboard() {
    m_dashboardWidget = new QWidget(this);
    QVBoxLayout *dLayout = new QVBoxLayout(m_dashboardWidget);
    dLayout->setAlignment(Qt::AlignCenter);
    
    QLabel *title = new QLabel("Medify");
    title->setStyleSheet("font-size: 26px; font-weight: bold; color: #40e0d0; margin-bottom: 20px;");
    title->setAlignment(Qt::AlignCenter);
    dLayout->addWidget(title);
    
    QGridLayout *grid = new QGridLayout();
    grid->setSpacing(25);
    grid->setAlignment(Qt::AlignCenter);
    
    int row = 0; int col = 0;
    for (int i = 0; i < m_tools.size(); ++i) {
        QToolButton *btn = new QToolButton();
        btn->setText(m_tools[i]->getIcon() + "\n\n" + m_tools[i]->getTitle());
        btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        btn->setFixedSize(140, 120);
        btn->setObjectName("dashboardTile");
        btn->setCursor(Qt::PointingHandCursor);
        
        connect(btn, &QToolButton::clicked, this, [this, i]() {
            onDashboardTileClicked(i);
        });
        
        grid->addWidget(btn, row, col);
        col++;
        if (col > 2) { col = 0; row++; }
    }
    
    dLayout->addLayout(grid);
}

void MainWindow::onDashboardTileClicked(int index) {
    if (index == 1) {
        // PDF to Images: use the dedicated thumbnail widget
        m_stackedWidget->setCurrentIndex(2);
    } else if (index >= 0 && index < m_tools.size()) {
        m_baseToolWidget->setTool(m_tools[index]);
        m_stackedWidget->setCurrentIndex(1);
    }
}

void MainWindow::onBackToDashboard() {
    m_stackedWidget->setCurrentIndex(0);
}

void MainWindow::applyStyles() {
    this->setStyleSheet(R"(
        QMainWindow { background-color: #2b2b2b; }
        QWidget {
            background-color: #2b2b2b;
            color: #ffffff;
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
            font-size: 14px;
        }
        #dashboardTile {
            background-color: #333333;
            border: 2px solid #555555;
            border-radius: 12px;
            color: #ffffff;
            font-size: 14px;
            font-weight: bold;
        }
        #dashboardTile:hover {
            border: 2px solid #40e0d0;
            background-color: rgba(64, 224, 208, 0.1);
        }
        #backBtn {
            background-color: transparent;
            color: #aaaaaa;
            border: 1px solid #555555;
            border-radius: 4px;
            padding: 4px 10px;
        }
        #backBtn:hover {
            color: #ffffff;
            border: 1px solid #ffffff;
        }
        #dropZone {
            background-color: #333333;
            border: 2px dashed #555555;
            border-radius: 10px;
        }
        #selectFilesButton {
            background-color: transparent;
            border: 1px solid #40e0d0;
            color: #40e0d0;
            border-radius: 6px;
            padding: 8px 16px;
            font-size: 16px;
            margin: 10px 0;
        }
        #selectFilesButton:hover {
            background-color: rgba(64, 224, 208, 0.1);
        }
        #addMoreButton {
            background-color: transparent;
            border: 1px solid #40e0d0;
            color: #40e0d0;
            border-radius: 4px;
            padding: 4px 12px;
        }
        #addMoreButton:hover {
            background-color: rgba(64, 224, 208, 0.1);
        }
        #removeBtn {
            background-color: transparent;
            color: #ff5555;
            border: none;
            font-weight: bold;
            font-size: 18px;
        }
        #removeBtn:hover {
            color: #ff0000;
        }
        QListWidget {
            background-color: #333333;
            border: 1px solid #444444;
            border-radius: 8px;
            padding: 5px;
        }
        QListWidget::item {
            padding: 5px;
            border-bottom: 1px solid #444444;
        }
        QListWidget::item:selected {
            background-color: #2a665e;
            border: 1px solid #40e0d0;
            border-radius: 4px;
        }
        QSlider::groove:horizontal {
            border: 1px solid #555555;
            height: 6px;
            background: #444444;
            border-radius: 3px;
        }
        QSlider::handle:horizontal {
            background: #40e0d0;
            border: none;
            width: 16px;
            margin: -5px 0; 
            border-radius: 8px;
        }
        QLineEdit, QComboBox {
            background-color: #333333;
            border: 1px solid #555555;
            border-radius: 6px;
            padding: 6px;
            color: #ffffff;
        }
        QLineEdit:focus { border: 1px solid #40e0d0; }
        QToolButton {
            background-color: #333333;
            border: 1px solid #555555;
            border-radius: 6px;
            padding: 5px 10px;
        }
        QToolButton:hover { border: 1px solid #40e0d0; }
        #compressButton {
            background-color: #40e0d0;
            color: #1b1b1b;
            border: none;
            border-radius: 8px;
            font-size: 16px;
            font-weight: bold;
        }
        #compressButton:hover { background-color: #55eadb; }
        #compressButton:disabled {
            background-color: #555555;
            color: #888888;
        }
        QProgressBar {
            background-color: #333333;
            border: 1px solid #555555;
            border-radius: 4px;
            text-align: center;
        }
        QProgressBar::chunk {
            background-color: #40e0d0;
            border-radius: 4px;
        }
    )");
}
