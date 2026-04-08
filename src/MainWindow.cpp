#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QMimeDatabase>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QThread>

MainWindow::MainWindow(bool hasFFmpeg, bool hasFfprobe, bool hasGs, QWidget *parent)
    : QMainWindow(parent), m_hasFFmpeg(hasFFmpeg), m_hasFfprobe(hasFfprobe), m_hasGs(hasGs),
      m_finalThread(nullptr), m_finalWorker(nullptr), m_activeMode(ToolMode::CompressMedia),
      m_targetSizeSlider(nullptr), m_targetSizeLabel(nullptr), m_pdfPageRangeEdit(nullptr),
      m_pdfDpiEdit(nullptr), m_audioFormatBox(nullptr), m_videoFormatBox(nullptr)
{
    setupUi();
    applyStyles();
    setAcceptDrops(true);
}

MainWindow::~MainWindow() {
    if (m_finalThread) {
        m_finalWorker->cancel();
        m_finalThread->quit();
        m_finalThread->wait();
    }
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
    buildToolFrame();
    
    m_stackedWidget->addWidget(m_dashboardWidget);
    m_stackedWidget->addWidget(m_toolWidgetShell);
    
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
    
    struct TileData {
        QString title;
        QString icon;
        ToolMode mode;
    };
    
    QList<TileData> tiles = {
        {"Compress Media", "🗜️", ToolMode::CompressMedia},
        {"PDF to Images", "📄", ToolMode::PdfToImage},
        {"Images to PDF", "🖼️", ToolMode::ImagesToPdf},
        {"Split PDF", "✂️", ToolMode::SplitPdf},
        {"Extract Audio", "🎵", ToolMode::ExtractAudio},
        {"Audio Converter", "🎧", ToolMode::AudioConverter},
        {"Video Converter", "🎬", ToolMode::VideoConverter}
    };
    
    int row = 0; int col = 0;
    for (const auto& t : tiles) {
        QToolButton *btn = new QToolButton();
        btn->setText(t.icon + "\n" + t.title);
        btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        btn->setFixedSize(140, 120);
        btn->setObjectName("dashboardTile");
        btn->setCursor(Qt::PointingHandCursor);
        
        connect(btn, &QToolButton::clicked, this, [this, mode = t.mode]() {
            onDashboardTileClicked(static_cast<int>(mode));
        });
        
        grid->addWidget(btn, row, col);
        col++;
        if (col > 2) { col = 0; row++; }
    }
    
    dLayout->addLayout(grid);
}

void MainWindow::buildToolFrame() {
    m_toolWidgetShell = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(m_toolWidgetShell);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(15);
    
    // Header
    QHBoxLayout *header = new QHBoxLayout();
    QPushButton *backBtn = new QPushButton("< Back");
    backBtn->setObjectName("backBtn");
    backBtn->setCursor(Qt::PointingHandCursor);
    connect(backBtn, &QPushButton::clicked, this, &MainWindow::onBackToDashboard);
    
    m_toolTitleLabel = new QLabel("Tool Name");
    m_toolTitleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #40e0d0;");
    
    header->addWidget(backBtn);
    header->addStretch();
    header->addWidget(m_toolTitleLabel);
    header->addStretch();
    
    layout->addLayout(header);
    
    // Tool internal stack handling File IO
    m_toolInternalStack = new QStackedWidget(this);
    m_dropZoneWidget = new QWidget(this);
    m_dropZoneWidget->setObjectName("dropZone");
    QVBoxLayout *dropZoneLayout = new QVBoxLayout(m_dropZoneWidget);
    dropZoneLayout->setAlignment(Qt::AlignCenter);
    
    QLabel *dropTitle = new QLabel("Drag and Drop File(s) or");
    dropTitle->setAlignment(Qt::AlignCenter);
    m_selectFilesButton = new QPushButton("Select File(s)");
    m_selectFilesButton->setObjectName("selectFilesButton");
    m_selectFilesButton->setCursor(Qt::PointingHandCursor);
    connect(m_selectFilesButton, &QPushButton::clicked, this, &MainWindow::onSelectFilesClicked);
    
    dropZoneLayout->addWidget(dropTitle);
    dropZoneLayout->addWidget(m_selectFilesButton, 0, Qt::AlignHCenter);
    
    m_toolInternalStack->addWidget(m_dropZoneWidget);
    
    QWidget *listPageWidget = new QWidget(this);
    QVBoxLayout *listPageLayout = new QVBoxLayout(listPageWidget);
    listPageLayout->setContentsMargins(0,0,0,0);
    
    QHBoxLayout *listActions = new QHBoxLayout();
    listActions->addStretch();
    QPushButton *addMoreBtn = new QPushButton("+ Add Media");
    addMoreBtn->setObjectName("addMoreButton");
    addMoreBtn->setCursor(Qt::PointingHandCursor);
    connect(addMoreBtn, &QPushButton::clicked, this, &MainWindow::onSelectFilesClicked);
    listActions->addWidget(addMoreBtn);
    
    listPageLayout->addLayout(listActions);
    m_listWidget = new QListWidget(this);
    m_listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    connect(m_listWidget, &QListWidget::itemSelectionChanged, this, &MainWindow::onSelectionChanged);
    listPageLayout->addWidget(m_listWidget);
    m_toolInternalStack->addWidget(listPageWidget);
    
    layout->addWidget(m_toolInternalStack, 1);
    
    // Dynamic Settings Container
    m_settingsContainer = new QWidget(this);
    m_settingsLayout = new QVBoxLayout(m_settingsContainer);
    m_settingsLayout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_settingsContainer);
    
    // Save To / Final Execute Layout
    QHBoxLayout *saveLayout = new QHBoxLayout();
    QLabel *saveLabel = new QLabel("Save To:");
    saveLabel->setFixedWidth(100);
    m_saveDirectoryEdit = new QLineEdit(this);
    m_saveDirectoryEdit->setReadOnly(true);
    m_saveDirectoryEdit->setText(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    
    m_browseButton = new QToolButton(this);
    m_browseButton->setText("📁");
    m_browseButton->setCursor(Qt::PointingHandCursor);
    connect(m_browseButton, &QToolButton::clicked, this, &MainWindow::onBrowseSaveDirectory);
    
    saveLayout->addWidget(saveLabel);
    saveLayout->addWidget(m_saveDirectoryEdit);
    saveLayout->addWidget(m_browseButton);
    layout->addLayout(saveLayout);
    
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 0);
    m_progressBar->setVisible(false);
    layout->addWidget(m_progressBar);
    
    m_processButton = new QPushButton("Process files");
    m_processButton->setObjectName("compressButton");
    m_processButton->setCursor(Qt::PointingHandCursor);
    m_processButton->setMinimumHeight(40);
    m_processButton->setEnabled(false);
    connect(m_processButton, &QPushButton::clicked, this, &MainWindow::onProcessAndSaveClicked);
    
    layout->addWidget(m_processButton);
}

void MainWindow::onDashboardTileClicked(int modeId) {
    m_activeMode = static_cast<ToolMode>(modeId);
    loadToolSettings(m_activeMode);
    
    m_listWidget->clear();
    m_toolInternalStack->setCurrentIndex(0);
    m_processButton->setEnabled(false);
    m_stackedWidget->setCurrentIndex(1); // Bring up tool shell
}

void MainWindow::onBackToDashboard() {
    m_stackedWidget->setCurrentIndex(0);
}

void MainWindow::loadToolSettings(ToolMode mode) {
    QLayoutItem *child;
    while ((child = m_settingsLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->hide();
            child->widget()->deleteLater();
        }
        delete child;
    }
    
    m_targetSizeSlider = nullptr;
    m_targetSizeLabel = nullptr;
    m_pdfPageRangeEdit = nullptr;
    m_pdfDpiEdit = nullptr;
    m_audioFormatBox = nullptr;
    m_videoFormatBox = nullptr;
    
    if (mode == ToolMode::CompressMedia) {
        m_toolTitleLabel->setText("Media Compressor");
        QHBoxLayout *h = new QHBoxLayout();
        QLabel *lbl = new QLabel("Target Size:");
        lbl->setFixedWidth(100);
        m_targetSizeSlider = new QSlider(Qt::Horizontal);
        m_targetSizeSlider->setRange(0, 100);
        m_targetSizeSlider->setValue(80);
        connect(m_targetSizeSlider, &QSlider::valueChanged, this, &MainWindow::onTargetSliderMoved);
        m_targetSizeLabel = new QLabel("-");
        m_targetSizeLabel->setFixedWidth(80);
        h->addWidget(lbl); h->addWidget(m_targetSizeSlider); h->addWidget(m_targetSizeLabel);
        QWidget *w = new QWidget(); w->setLayout(h);
        m_settingsLayout->addWidget(w);
        
    } else if (mode == ToolMode::PdfToImage) {
        m_toolTitleLabel->setText("PDF to Images");
        QHBoxLayout *h = new QHBoxLayout();
        QLabel *l1 = new QLabel("DPI:");
        m_pdfDpiEdit = new QLineEdit("300");
        m_pdfDpiEdit->setFixedWidth(60);
        QLabel *l2 = new QLabel("Page Range (Opts):");
        m_pdfPageRangeEdit = new QLineEdit();
        m_pdfPageRangeEdit->setPlaceholderText("e.g. 1-5");
        h->addWidget(l1); h->addWidget(m_pdfDpiEdit);
        h->addSpacing(20);
        h->addWidget(l2); h->addWidget(m_pdfPageRangeEdit);
        QWidget *w = new QWidget(); w->setLayout(h);
        m_settingsLayout->addWidget(w);
        
    } else if (mode == ToolMode::ImagesToPdf) {
        m_toolTitleLabel->setText("Images to PDF");
        QLabel *lbl = new QLabel("Merge loaded images into a single PDF.");
        m_settingsLayout->addWidget(lbl);
        
    } else if (mode == ToolMode::SplitPdf) {
        m_toolTitleLabel->setText("Split PDF");
        QHBoxLayout *h = new QHBoxLayout();
        QLabel *lbl = new QLabel("Extract Page Range:");
        m_pdfPageRangeEdit = new QLineEdit();
        m_pdfPageRangeEdit->setPlaceholderText("e.g. 2-5");
        h->addWidget(lbl); h->addWidget(m_pdfPageRangeEdit);
        QWidget *w = new QWidget(); w->setLayout(h);
        m_settingsLayout->addWidget(w);
        
    } else if (mode == ToolMode::ExtractAudio) {
        m_toolTitleLabel->setText("Extract Audio from Video");
        QHBoxLayout *h = new QHBoxLayout();
        QLabel *lbl = new QLabel("Convert To:");
        m_audioFormatBox = new QComboBox();
        m_audioFormatBox->addItems({"mp3", "wav"});
        h->addWidget(lbl); h->addWidget(m_audioFormatBox); h->addStretch();
        QWidget *w = new QWidget(); w->setLayout(h);
        m_settingsLayout->addWidget(w);
        
    } else if (mode == ToolMode::AudioConverter) {
        m_toolTitleLabel->setText("Audio Converter");
        QHBoxLayout *h = new QHBoxLayout();
        QLabel *lbl = new QLabel("Convert To:");
        m_audioFormatBox = new QComboBox();
        m_audioFormatBox->addItems({"mp3", "wav", "ogg", "flac"});
        h->addWidget(lbl); h->addWidget(m_audioFormatBox); h->addStretch();
        QWidget *w = new QWidget(); w->setLayout(h);
        m_settingsLayout->addWidget(w);
        
    } else if (mode == ToolMode::VideoConverter) {
        m_toolTitleLabel->setText("Video Converter");
        QHBoxLayout *h = new QHBoxLayout();
        QLabel *lbl = new QLabel("Convert To Container:");
        m_videoFormatBox = new QComboBox();
        m_videoFormatBox->addItems({"mp4", "mkv", "avi", "mov"});
        h->addWidget(lbl); h->addWidget(m_videoFormatBox); h->addStretch();
        QWidget *w = new QWidget(); w->setLayout(h);
        m_settingsLayout->addWidget(w);
    }
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

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event) {
    QStringList files;
    foreach (const QUrl &url, event->mimeData()->urls()) {
        files.append(url.toLocalFile());
    }
    addFiles(files);
}

void MainWindow::onSelectFilesClicked() {
    QString filter = "All Files (*)";
    if (m_activeMode == ToolMode::PdfToImage || m_activeMode == ToolMode::SplitPdf) {
        filter = "PDF Files (*.pdf)";
    } else if (m_activeMode == ToolMode::ImagesToPdf) {
        filter = "Image Files (*.jpg *.jpeg *.png *.webp *.bmp)";
    } else if (m_activeMode == ToolMode::ExtractAudio || m_activeMode == ToolMode::VideoConverter) {
        filter = "Video Files (*.mp4 *.mkv *.avi *.mov)";
    } else if (m_activeMode == ToolMode::AudioConverter) {
        filter = "Audio Files (*.mp3 *.wav *.ogg *.flac)";
    }
    
    QStringList files = QFileDialog::getOpenFileNames(this, "Select Media", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), filter);
    if (!files.isEmpty()) addFiles(files);
}

void MainWindow::onBrowseSaveDirectory() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Save Directory", m_saveDirectoryEdit->text());
    if (!dir.isEmpty()) m_saveDirectoryEdit->setText(dir);
}

void MainWindow::addFiles(const QStringList& files) {
    for (const QString& file : files) {
        MediaType type = getMediaType(file);
        if (type != MediaType::Unknown || m_activeMode == ToolMode::ImagesToPdf) {
            QFileInfo fi(file);
            QListWidgetItem *item = new QListWidgetItem();
            item->setData(Qt::UserRole, file);
            item->setData(Qt::UserRole + 1, static_cast<int>(type));
            
            QWidget *itemWidget = new QWidget(m_listWidget);
            itemWidget->setStyleSheet("background: transparent;");
            itemWidget->setFixedHeight(36);
            
            QHBoxLayout *itemLayout = new QHBoxLayout(itemWidget);
            itemLayout->setContentsMargins(10, 0, 10, 0);
            itemLayout->setAlignment(Qt::AlignVCenter);
            
            QLabel *textLabel = new QLabel(QString("%1 (%2)").arg(fi.fileName()).arg(formatSize(fi.size())), itemWidget);
            textLabel->setStyleSheet("background: transparent; color: #ffffff; font-size: 14px;");
            
            QToolButton *removeBtn = new QToolButton(itemWidget);
            removeBtn->setText("×");
            removeBtn->setObjectName("removeBtn");
            removeBtn->setCursor(Qt::PointingHandCursor);
            removeBtn->setFixedSize(28, 28);
            
            itemLayout->addWidget(textLabel);
            itemLayout->addStretch();
            itemLayout->addWidget(removeBtn);
            
            item->setSizeHint(QSize(100, 36));
            
            m_listWidget->addItem(item);
            m_listWidget->setItemWidget(item, itemWidget);
            
            connect(removeBtn, &QToolButton::clicked, this, [this, item]() {
                int row = m_listWidget->row(item);
                if (row >= 0) {
                    delete m_listWidget->takeItem(row);
                    if (m_listWidget->count() == 0) {
                        m_toolInternalStack->setCurrentIndex(0);
                        m_processButton->setEnabled(false);
                        if (m_targetSizeSlider) {
                            m_targetSizeSlider->setEnabled(false);
                            m_currentOriginalSize = 0;
                            m_targetSizeLabel->setText("-");
                        }
                    }
                }
            });
        }
    }
    
    if (m_listWidget->count() > 0) {
        m_toolInternalStack->setCurrentIndex(1); 
        m_processButton->setEnabled(true);
        if (m_targetSizeSlider && m_listWidget->selectedItems().isEmpty()) {
            m_listWidget->setCurrentRow(0); // auto select first for slider size
        }
    }
}

MediaType MainWindow::getMediaType(const QString& filePath) {
    QMimeDatabase mimeDb;
    QMimeType mimeObj = mimeDb.mimeTypeForFile(filePath);
    QString mime = mimeObj.name();

    if (mime.startsWith("image/")) return MediaType::Image;
    if (mime.startsWith("audio/")) return MediaType::Audio;
    if (mime.startsWith("video/")) return MediaType::Video;
    if (mime == "application/pdf") return MediaType::Pdf;

    return MediaType::Unknown;
}

QString MainWindow::formatSize(qint64 bytes) {
    if (bytes < 1024) return QString::number(bytes) + " B";
    if (bytes < 1024 * 1024) return QString::number(bytes / 1024.0, 'f', 1) + " KB";
    if (bytes < 1024 * 1024 * 1024) return QString::number(bytes / (1024.0 * 1024.0), 'f', 2) + " MB";
    return QString::number(bytes / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
}

qint64 MainWindow::mapSliderToBytes(int sliderValue) {
    if (m_currentOriginalSize <= 0) return 0;
    qint64 minSize = m_currentOriginalSize * 0.1;
    return minSize + (m_currentOriginalSize - minSize) * sliderValue / 100;
}

void MainWindow::onSelectionChanged() {
    auto items = m_listWidget->selectedItems();
    if (items.isEmpty() || !m_targetSizeSlider) return;

    QString inputPath = items.first()->data(Qt::UserRole).toString();
    m_currentOriginalSize = QFileInfo(inputPath).size();
    
    m_targetSizeSlider->setEnabled(true);
    m_targetSizeLabel->setText("~ " + formatSize(mapSliderToBytes(m_targetSizeSlider->value())));
}

void MainWindow::onTargetSliderMoved(int value) {
    if (m_targetSizeLabel) {
        m_targetSizeLabel->setText("~ " + formatSize(mapSliderToBytes(value)));
    }
}

void MainWindow::onProcessAndSaveClicked() {
    if (m_listWidget->count() == 0) return;

    QString saveDir = m_saveDirectoryEdit->text();
    if (saveDir.isEmpty()) {
        QMessageBox::warning(this, "No Directory", "Please select a save directory first.");
        return;
    }

    QStringList inputPaths;
    for (int i = 0; i < m_listWidget->count(); ++i) {
        inputPaths.append(m_listWidget->item(i)->data(Qt::UserRole).toString());
    }

    QString firstInput = inputPaths.first();
    QFileInfo fi(firstInput);
    QString suggestion = fi.completeBaseName() + "_processed." + fi.suffix();

    QVariantMap settings;

    if (m_activeMode == ToolMode::CompressMedia) {
        MediaType type = static_cast<MediaType>(m_listWidget->item(0)->data(Qt::UserRole + 1).toInt());
        if (type == MediaType::Audio) suggestion = fi.completeBaseName() + "_compressed.mp3";
        if (type == MediaType::Video) suggestion = fi.completeBaseName() + "_compressed.mp4";
        if (type == MediaType::Image) suggestion = fi.completeBaseName() + "_compressed.jpg";
        
        qint64 targetBytes = m_targetSizeSlider ? mapSliderToBytes(m_targetSizeSlider->value()) : fi.size();
        settings.insert("targetBytes", targetBytes);
        
    } else if (m_activeMode == ToolMode::PdfToImage) {
        suggestion = fi.completeBaseName() + "_export.png"; // Ghostscript will format via %03d in backend
        if (m_pdfDpiEdit) settings.insert("dpi", m_pdfDpiEdit->text());
        if (m_pdfPageRangeEdit) settings.insert("pageRange", m_pdfPageRangeEdit->text());
        
    } else if (m_activeMode == ToolMode::ImagesToPdf) {
        suggestion = "MergedImages.pdf";
        
    } else if (m_activeMode == ToolMode::SplitPdf) {
        suggestion = fi.completeBaseName() + "_split.pdf";
        if (m_pdfPageRangeEdit) settings.insert("pageRange", m_pdfPageRangeEdit->text());
        
    } else if (m_activeMode == ToolMode::ExtractAudio) {
        QString ext = m_audioFormatBox ? m_audioFormatBox->currentText() : "mp3";
        suggestion = fi.completeBaseName() + "_audio." + ext;
        settings.insert("format", ext);
        
    } else if (m_activeMode == ToolMode::AudioConverter) {
        QString ext = m_audioFormatBox ? m_audioFormatBox->currentText() : "mp3";
        suggestion = fi.completeBaseName() + "_converted." + ext;
        settings.insert("format", ext);
        
    } else if (m_activeMode == ToolMode::VideoConverter) {
        QString ext = m_videoFormatBox ? m_videoFormatBox->currentText() : "mp4";
        suggestion = fi.completeBaseName() + "_converted." + ext;
        settings.insert("format", ext);
    }

    QString savePath = saveDir + "/" + suggestion;

    m_processButton->setEnabled(false);
    m_listWidget->setEnabled(false);
    m_progressBar->setVisible(true);

    m_finalThread = new QThread(this);
    m_finalWorker = new ProcessManager(m_activeMode, inputPaths, savePath, settings);
    m_finalWorker->moveToThread(m_finalThread);

    connect(m_finalThread, &QThread::started, m_finalWorker, &ProcessManager::start);
    connect(m_finalWorker, &ProcessManager::finished, this, &MainWindow::onFinalWorkerFinished);

    m_finalThread->start();
}

void MainWindow::onFinalWorkerFinished(bool success, const QString& error) {
    m_progressBar->setVisible(false);
    m_processButton->setEnabled(true);
    m_listWidget->setEnabled(true);

    if (success) {
        QMessageBox::information(this, "Success", "Media processing complete.");
    } else {
        QMessageBox::critical(this, "Error", "Processing failed:\n" + error);
    }

    if (m_finalThread) {
        m_finalThread->quit();
        m_finalThread->wait();
        m_finalWorker->deleteLater();
        m_finalThread->deleteLater();
        m_finalWorker = nullptr;
        m_finalThread = nullptr;
    }
}
