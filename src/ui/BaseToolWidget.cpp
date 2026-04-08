#include "BaseToolWidget.h"
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
#include <QToolButton>

#include "../tools/compress_media/CompressMediaTool.h"

BaseToolWidget::BaseToolWidget(QWidget *parent)
    : QWidget(parent), m_currentTool(nullptr), m_workerThread(nullptr), m_worker(nullptr)
{
    setAcceptDrops(true);
    setupUi();
}

void BaseToolWidget::setupUi() {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(15);
    
    // Header
    QHBoxLayout *header = new QHBoxLayout();
    QPushButton *backBtn = new QPushButton("< Back");
    backBtn->setObjectName("backBtn");
    backBtn->setCursor(Qt::PointingHandCursor);
    connect(backBtn, &QPushButton::clicked, this, &BaseToolWidget::requestBackToDashboard);
    
    m_toolTitleLabel = new QLabel("Tool Name");
    m_toolTitleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #40e0d0;");
    
    header->addWidget(backBtn);
    header->addStretch();
    header->addWidget(m_toolTitleLabel);
    header->addStretch();
    layout->addLayout(header);
    
    m_internalStack = new QStackedWidget(this);
    
    // Page 0: Dropzone
    QWidget *dropZoneWidget = new QWidget(this);
    dropZoneWidget->setObjectName("dropZone");
    QVBoxLayout *dropZoneLayout = new QVBoxLayout(dropZoneWidget);
    QLabel *dropTitle = new QLabel("Drag and Drop File(s) or");
    dropTitle->setAlignment(Qt::AlignCenter);
    QPushButton *selectFilesBtn = new QPushButton("Select File(s)");
    selectFilesBtn->setObjectName("selectFilesButton");
    selectFilesBtn->setCursor(Qt::PointingHandCursor);
    connect(selectFilesBtn, &QPushButton::clicked, this, &BaseToolWidget::onSelectFilesClicked);
    dropZoneLayout->addWidget(dropTitle);
    dropZoneLayout->addWidget(selectFilesBtn, 0, Qt::AlignHCenter);
    m_internalStack->addWidget(dropZoneWidget);
    
    // Page 1: List
    QWidget *listPageWidget = new QWidget(this);
    QVBoxLayout *listPageLayout = new QVBoxLayout(listPageWidget);
    listPageLayout->setContentsMargins(0,0,0,0);
    QHBoxLayout *listActions = new QHBoxLayout();
    listActions->addStretch();
    QPushButton *addMoreBtn = new QPushButton("+ Add Media");
    addMoreBtn->setObjectName("addMoreButton");
    addMoreBtn->setCursor(Qt::PointingHandCursor);
    connect(addMoreBtn, &QPushButton::clicked, this, &BaseToolWidget::onSelectFilesClicked);
    listActions->addWidget(addMoreBtn);
    listPageLayout->addLayout(listActions);
    m_listWidget = new QListWidget(this);
    m_listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    connect(m_listWidget, &QListWidget::itemSelectionChanged, this, &BaseToolWidget::onSelectionChanged);
    listPageLayout->addWidget(m_listWidget);
    m_internalStack->addWidget(listPageWidget);
    
    layout->addWidget(m_internalStack, 1);
    
    m_settingsContainer = new QWidget(this);
    QVBoxLayout* scLayout = new QVBoxLayout(m_settingsContainer);
    scLayout->setContentsMargins(0,0,0,0);
    layout->addWidget(m_settingsContainer);
    
    QHBoxLayout *saveLayout = new QHBoxLayout();
    QLabel *saveLabel = new QLabel("Save To:");
    saveLabel->setFixedWidth(100);
    m_saveDirectoryEdit = new QLineEdit(this);
    m_saveDirectoryEdit->setReadOnly(true);
    m_saveDirectoryEdit->setText(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    
    QToolButton *browseButton = new QToolButton(this);
    browseButton->setText("📁");
    browseButton->setCursor(Qt::PointingHandCursor);
    connect(browseButton, &QToolButton::clicked, this, &BaseToolWidget::onBrowseSaveDirectory);
    saveLayout->addWidget(saveLabel);
    saveLayout->addWidget(m_saveDirectoryEdit);
    saveLayout->addWidget(browseButton);
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
    connect(m_processButton, &QPushButton::clicked, this, &BaseToolWidget::onProcessClicked);
    layout->addWidget(m_processButton);
}

void BaseToolWidget::setTool(ITool* tool) {
    m_currentTool = tool;
    
    m_listWidget->clear();
    m_internalStack->setCurrentIndex(0);
    m_processButton->setEnabled(false);
    
    QLayoutItem *child;
    while ((child = m_settingsContainer->layout()->takeAt(0)) != nullptr) {
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }
    
    if (m_currentTool) {
        m_toolTitleLabel->setText(m_currentTool->getTitle());
        m_currentTool->buildSettingsUi(m_settingsContainer);
    }
}

void BaseToolWidget::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls() && m_currentTool) event->acceptProposedAction();
}

void BaseToolWidget::dropEvent(QDropEvent *event) {
    QStringList files;
    foreach (const QUrl &url, event->mimeData()->urls()) {
        files.append(url.toLocalFile());
    }
    addFiles(files);
}

void BaseToolWidget::onSelectFilesClicked() {
    if (!m_currentTool) return;
    QStringList files = QFileDialog::getOpenFileNames(this, "Select Media", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), m_currentTool->getFilter());
    if (!files.isEmpty()) addFiles(files);
}

void BaseToolWidget::onBrowseSaveDirectory() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Save Directory", m_saveDirectoryEdit->text());
    if (!dir.isEmpty()) m_saveDirectoryEdit->setText(dir);
}

void BaseToolWidget::addFiles(const QStringList& files) {
    for (const QString& file : files) {
        QFileInfo fi(file);
        QListWidgetItem *item = new QListWidgetItem();
        item->setData(Qt::UserRole, file);
        MediaType t = getMediaType(file);
        item->setData(Qt::UserRole + 1, static_cast<int>(t));
        
        QWidget *itemWidget = new QWidget(m_listWidget);
        itemWidget->setStyleSheet("background: transparent;");
        itemWidget->setFixedHeight(36);
        QHBoxLayout *itemLayout = new QHBoxLayout(itemWidget);
        itemLayout->setContentsMargins(10, 0, 10, 0);
        
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
                    m_internalStack->setCurrentIndex(0);
                    m_processButton->setEnabled(false);
                }
            }
        });
    }
    
    if (m_listWidget->count() > 0) {
        m_internalStack->setCurrentIndex(1); 
        m_processButton->setEnabled(true);
        if (m_listWidget->selectedItems().isEmpty()) {
            m_listWidget->setCurrentRow(0);
        }
    }
}

void BaseToolWidget::onSelectionChanged() {
    auto items = m_listWidget->selectedItems();
    if (items.isEmpty() || !m_currentTool) return;
    
    CompressMediaTool* cmt = dynamic_cast<CompressMediaTool*>(m_currentTool);
    if (cmt) {
        QString inputPath = items.first()->data(Qt::UserRole).toString();
        cmt->updateOriginalSize(QFileInfo(inputPath).size());
    }
}

void BaseToolWidget::onProcessClicked() {
    if (!m_currentTool || m_listWidget->count() == 0) return;

    QString saveDir = m_saveDirectoryEdit->text();
    if (saveDir.isEmpty()) {
        QMessageBox::warning(this, "No Directory", "Please select a save directory.");
        return;
    }

    QStringList inputPaths;
    for (int i = 0; i < m_listWidget->count(); ++i) {
        inputPaths.append(m_listWidget->item(i)->data(Qt::UserRole).toString());
    }

    int filterMode = static_cast<int>(getMediaType(inputPaths.first()));
    QString suggestion = m_currentTool->getOutputSuggestion(inputPaths.first(), filterMode);
    QString savePath = saveDir + "/" + suggestion;

    m_processButton->setEnabled(false);
    m_listWidget->setEnabled(false);
    m_progressBar->setVisible(true);

    m_workerThread = new QThread(this);
    m_worker = m_currentTool->createWorker(inputPaths, savePath, m_currentTool->getSettings());
    m_worker->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::started, m_worker, &BaseWorker::start);
    connect(m_worker, &BaseWorker::finished, this, &BaseToolWidget::onWorkerFinished);

    m_workerThread->start();
}

void BaseToolWidget::onWorkerFinished(bool success, const QString& error) {
    m_progressBar->setVisible(false);
    m_processButton->setEnabled(true);
    m_listWidget->setEnabled(true);

    if (success) {
        QMessageBox::information(this, "Success", "Media processing complete.");
    } else {
        QMessageBox::critical(this, "Error", "Processing failed:\n" + error);
    }

    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait();
        m_worker->deleteLater();
        m_workerThread->deleteLater();
        m_worker = nullptr;
        m_workerThread = nullptr;
    }
}

MediaType BaseToolWidget::getMediaType(const QString& filePath) {
    QMimeDatabase mimeDb;
    QMimeType mimeObj = mimeDb.mimeTypeForFile(filePath);
    QString mime = mimeObj.name();
    if (mime.startsWith("image/")) return MediaType::Image;
    if (mime.startsWith("audio/")) return MediaType::Audio;
    if (mime.startsWith("video/")) return MediaType::Video;
    if (mime == "application/pdf") return MediaType::Pdf;
    return MediaType::Unknown;
}

QString BaseToolWidget::formatSize(qint64 bytes) {
    if (bytes < 1024) return QString::number(bytes) + " B";
    if (bytes < 1024 * 1024) return QString::number(bytes / 1024.0, 'f', 1) + " KB";
    if (bytes < 1024 * 1024 * 1024) return QString::number(bytes / (1024.0 * 1024.0), 'f', 2) + " MB";
    return QString::number(bytes / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
}
