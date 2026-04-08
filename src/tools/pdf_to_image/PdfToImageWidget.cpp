#include "PdfToImageWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QToolButton>
#include <QPainter>
#include <QDialog>
#include <QScrollArea>
#include <QSlider>

PdfToImageWidget::PdfToImageWidget(QWidget* parent)
    : QWidget(parent),
      m_pdfDocument(new QPdfDocument(this)),
      m_currentThumbH(130),
      m_workerThread(nullptr), m_worker(nullptr)
{
    setAcceptDrops(true);
    setupUi();
}

void PdfToImageWidget::setupUi() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(12);

    // Header
    QHBoxLayout* header = new QHBoxLayout();
    QPushButton* backBtn = new QPushButton("< Back");
    backBtn->setObjectName("backBtn");
    backBtn->setCursor(Qt::PointingHandCursor);
    connect(backBtn, &QPushButton::clicked, this, &PdfToImageWidget::requestBackToDashboard);

    m_titleLabel = new QLabel("PDF to Images");
    m_titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #40e0d0;");

    header->addWidget(backBtn);
    header->addStretch();
    header->addWidget(m_titleLabel);
    header->addStretch();
    layout->addLayout(header);

    // Internal stack: page 0 = drop zone, page 1 = thumbnails
    m_internalStack = new QStackedWidget(this);

    // --- Page 0: Drop zone ---
    QWidget* dropZone = new QWidget();
    dropZone->setObjectName("dropZone");
    QVBoxLayout* dropLayout = new QVBoxLayout(dropZone);
    dropLayout->setAlignment(Qt::AlignCenter);

    QLabel* dropLabel = new QLabel("Drag & Drop a PDF or");
    dropLabel->setAlignment(Qt::AlignCenter);

    QPushButton* selectBtn = new QPushButton("Select PDF File");
    selectBtn->setObjectName("selectFilesButton");
    selectBtn->setCursor(Qt::PointingHandCursor);
    connect(selectBtn, &QPushButton::clicked, this, &PdfToImageWidget::onSelectFileClicked);

    dropLayout->addStretch();
    dropLayout->addWidget(dropLabel);
    dropLayout->addWidget(selectBtn, 0, Qt::AlignCenter);
    dropLayout->addStretch();
    m_internalStack->addWidget(dropZone);

    // --- Page 1: Thumbnail grid ---
    QWidget* thumbPage = new QWidget();
    QVBoxLayout* thumbLayout = new QVBoxLayout(thumbPage);
    thumbLayout->setContentsMargins(0, 0, 0, 0);
    thumbLayout->setSpacing(6);

    QHBoxLayout* thumbActions = new QHBoxLayout();
    m_pageCountLabel = new QLabel("");
    m_pageCountLabel->setStyleSheet("color: #aaaaaa; font-size: 12px;");
    thumbActions->addWidget(m_pageCountLabel);
    thumbActions->addStretch();

    // Zoom controls
    QLabel* zoomIcon = new QLabel("🔍");
    thumbActions->addWidget(zoomIcon);
    m_zoomSlider = new QSlider(Qt::Horizontal);
    m_zoomSlider->setRange(80, 280);   // thumb height range in px
    m_zoomSlider->setValue(130);
    m_zoomSlider->setFixedWidth(120);
    m_zoomSlider->setToolTip("Thumbnail size");
    connect(m_zoomSlider, &QSlider::valueChanged, this, &PdfToImageWidget::onZoomChanged);
    thumbActions->addWidget(m_zoomSlider);

    QPushButton* changeFileBtn = new QPushButton("↩ Change PDF");
    changeFileBtn->setObjectName("addMoreButton");
    changeFileBtn->setCursor(Qt::PointingHandCursor);
    connect(changeFileBtn, &QPushButton::clicked, this, &PdfToImageWidget::onSelectFileClicked);
    thumbActions->addWidget(changeFileBtn);
    thumbLayout->addLayout(thumbActions);

    m_thumbnailList = new QListWidget();
    m_thumbnailList->setViewMode(QListView::IconMode);
    m_thumbnailList->setIconSize(QSize(100, 130));
    m_thumbnailList->setGridSize(QSize(130, 175));
    m_thumbnailList->setResizeMode(QListView::Adjust);
    m_thumbnailList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_thumbnailList->setMovement(QListView::Static);
    m_thumbnailList->setWrapping(true);
    m_thumbnailList->setSpacing(6);
    m_thumbnailList->setStyleSheet(
        "QListWidget { background: #2b2b2b; border: 1px solid #444; border-radius: 8px; padding: 8px; }"
        "QListWidget::item { color: #ccc; font-size: 11px; border-radius: 4px; }"
        "QListWidget::item:selected { background: rgba(64,224,208,0.25); border: 2px solid #40e0d0; }"
    );
    connect(m_thumbnailList, &QListWidget::itemDoubleClicked,
            this, &PdfToImageWidget::onPageDoubleClicked);
    thumbLayout->addWidget(m_thumbnailList);

    m_internalStack->addWidget(thumbPage);
    layout->addWidget(m_internalStack, 1);

    // File Name row
    QHBoxLayout* nameLayout = new QHBoxLayout();
    QLabel* nameLabel = new QLabel("File Name:");
    nameLabel->setFixedWidth(100);
    m_baseNameEdit = new QLineEdit("page");
    m_baseNameEdit->setPlaceholderText("Base name (e.g. page → page_001.png, page_002.png …)");
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(m_baseNameEdit);
    layout->addLayout(nameLayout);

    // DPI row
    QHBoxLayout* dpiLayout = new QHBoxLayout();
    QLabel* dpiLabel = new QLabel("Export DPI:");
    dpiLabel->setFixedWidth(100);
    m_dpiEdit = new QLineEdit("150");
    m_dpiEdit->setFixedWidth(60);
    QLabel* dpiHint = new QLabel("(Higher = better quality & larger files)");
    dpiHint->setStyleSheet("color: #777; font-size: 12px;");
    dpiLayout->addWidget(dpiLabel);
    dpiLayout->addWidget(m_dpiEdit);
    dpiLayout->addWidget(dpiHint);
    dpiLayout->addStretch();
    layout->addLayout(dpiLayout);

    // Save To row
    QHBoxLayout* saveLayout = new QHBoxLayout();
    QLabel* saveLabel = new QLabel("Save To:");
    saveLabel->setFixedWidth(100);
    m_saveDirectoryEdit = new QLineEdit();
    m_saveDirectoryEdit->setReadOnly(true);
    m_saveDirectoryEdit->setText(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    QToolButton* browseBtn = new QToolButton();
    browseBtn->setText("📁");
    browseBtn->setCursor(Qt::PointingHandCursor);
    connect(browseBtn, &QToolButton::clicked, this, &PdfToImageWidget::onBrowseSaveDirectory);
    saveLayout->addWidget(saveLabel);
    saveLayout->addWidget(m_saveDirectoryEdit);
    saveLayout->addWidget(browseBtn);
    layout->addLayout(saveLayout);

    // Progress bar
    m_progressBar = new QProgressBar();
    m_progressBar->setRange(0, 0);
    m_progressBar->setVisible(false);
    layout->addWidget(m_progressBar);

    // Process button
    m_processButton = new QPushButton("Export Selected Pages");
    m_processButton->setObjectName("compressButton");
    m_processButton->setCursor(Qt::PointingHandCursor);
    m_processButton->setMinimumHeight(40);
    m_processButton->setEnabled(false);
    connect(m_processButton, &QPushButton::clicked, this, &PdfToImageWidget::onProcessClicked);
    layout->addWidget(m_processButton);
}

void PdfToImageWidget::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        const auto urls = event->mimeData()->urls();
        if (!urls.isEmpty() && urls.first().toLocalFile().endsWith(".pdf", Qt::CaseInsensitive))
            event->acceptProposedAction();
    }
}

void PdfToImageWidget::dropEvent(QDropEvent* event) {
    const auto urls = event->mimeData()->urls();
    if (!urls.isEmpty())
        loadPdf(urls.first().toLocalFile());
}

void PdfToImageWidget::onSelectFileClicked() {
    QString path = QFileDialog::getOpenFileName(
        this, "Select PDF File",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "PDF Files (*.pdf)"
    );
    if (!path.isEmpty()) loadPdf(path);
}

void PdfToImageWidget::onBrowseSaveDirectory() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Save Directory", m_saveDirectoryEdit->text());
    if (!dir.isEmpty()) m_saveDirectoryEdit->setText(dir);
}

void PdfToImageWidget::loadPdf(const QString& path) {
    m_pdfDocument->close();
    m_thumbnailList->clear();

    m_pdfDocument->load(path);
    int pageCount = m_pdfDocument->pageCount();
    if (pageCount <= 0) {
        QMessageBox::critical(this, "Error", "Could not load the PDF or the file has no pages.");
        return;
    }

    m_currentPdfPath = path;
    QFileInfo fi(path);

    // Suggest base name from the pdf filename
    m_baseNameEdit->setText(fi.completeBaseName() + "_page");

    m_pageCountLabel->setText(
        QString("%1 page(s) — Cmd+click or Shift+click to select multiple. No selection = export all.").arg(pageCount)
    );

    // Render a thumbnail for each page with white background
    for (int i = 0; i < pageCount; ++i) {
        QImage thumb = renderPageImage(i, 130);

        QListWidgetItem* item = new QListWidgetItem();
        item->setIcon(QIcon(QPixmap::fromImage(thumb)));
        item->setText(QString("Page %1").arg(i + 1));
        item->setData(Qt::UserRole, i + 1); // 1-indexed page number
        m_thumbnailList->addItem(item);
    }

    m_internalStack->setCurrentIndex(1);
    m_processButton->setEnabled(true);
}

void PdfToImageWidget::onProcessClicked() {
    if (m_currentPdfPath.isEmpty()) return;

    QString saveDir = m_saveDirectoryEdit->text().trimmed();
    if (saveDir.isEmpty()) {
        QMessageBox::warning(this, "No Directory", "Please select a save directory.");
        return;
    }

    QString baseName = m_baseNameEdit->text().trimmed();
    if (baseName.isEmpty()) baseName = "page";

    // Collect selected pages (1-indexed); empty = export all
    QVariantList selectedPages;
    const auto selectedItems = m_thumbnailList->selectedItems();
    for (auto* item : selectedItems) {
        selectedPages.append(item->data(Qt::UserRole).toInt());
    }

    QVariantMap settings;
    settings["dpi"] = m_dpiEdit->text().trimmed().isEmpty() ? "150" : m_dpiEdit->text().trimmed();
    settings["selectedPages"] = selectedPages;
    settings["baseName"] = baseName;

    m_processButton->setEnabled(false);
    m_thumbnailList->setEnabled(false);
    m_progressBar->setVisible(true);

    m_workerThread = new QThread(this);
    // outputPath = directory; worker uses it accordingly
    m_worker = new PdfToImageWorker({m_currentPdfPath}, saveDir, settings);
    m_worker->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::started, m_worker, &PdfToImageWorker::start);
    connect(m_worker, &PdfToImageWorker::finished, this, &PdfToImageWidget::onWorkerFinished);

    m_workerThread->start();
}

void PdfToImageWidget::onWorkerFinished(bool success, const QString& error) {
    m_progressBar->setVisible(false);
    m_processButton->setEnabled(true);
    m_thumbnailList->setEnabled(true);

    if (success) {
        QMessageBox::information(this, "Success", "Pages exported successfully.");
    } else {
        QMessageBox::critical(this, "Error", "Export failed:\n" + error);
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

// Renders page onto a white background at the given height, keeping aspect ratio
QImage PdfToImageWidget::renderPageImage(int pageIndex, int targetHeight) {
    QSizeF pageSize = m_pdfDocument->pagePointSize(pageIndex);
    double aspect = (pageSize.height() > 0) ? pageSize.width() / pageSize.height() : 0.77;
    int w = qMax(40, (int)(targetHeight * aspect));
    int h = targetHeight;

    QImage canvas(w, h, QImage::Format_ARGB32);
    canvas.fill(Qt::white);

    QImage rendered = m_pdfDocument->render(pageIndex, QSize(w, h));
    QPainter painter(&canvas);
    painter.drawImage(0, 0, rendered);
    painter.end();
    return canvas;
}

void PdfToImageWidget::onPageDoubleClicked(QListWidgetItem* item) {
    int pageIndex = item->data(Qt::UserRole).toInt() - 1; // 0-indexed
    int pageNumber = pageIndex + 1;

    QImage preview = renderPageImage(pageIndex, 900);

    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle(QString("Page %1").arg(pageNumber));
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setStyleSheet("background: #1e1e1e;");

    QVBoxLayout* dlgLayout = new QVBoxLayout(dialog);
    dlgLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea* scrollArea = new QScrollArea(dialog);
    scrollArea->setWidgetResizable(false);
    scrollArea->setStyleSheet("QScrollArea { border: none; background: #1e1e1e; }");

    QLabel* imgLabel = new QLabel();
    imgLabel->setPixmap(QPixmap::fromImage(preview));
    imgLabel->setStyleSheet("background: white; padding: 8px;");
    scrollArea->setWidget(imgLabel);

    int dH = qMin(preview.height() + 30, 850);
    int dW = qMin(preview.width() + 30, 950);
    dialog->resize(dW, dH);

    dlgLayout->addWidget(scrollArea);
    dialog->exec();
}

void PdfToImageWidget::onZoomChanged(int value) {
    if (m_currentPdfPath.isEmpty()) return;
    applyZoom(value);
}

void PdfToImageWidget::applyZoom(int thumbH) {
    m_currentThumbH = thumbH;

    int padding = 20;
    int thumbW = qMax(40, (int)(thumbH * 0.77)); // rough default aspect

    m_thumbnailList->setIconSize(QSize(thumbW, thumbH));
    m_thumbnailList->setGridSize(QSize(thumbW + padding, thumbH + 35));

    // Re-render all items at the new size
    int count = m_thumbnailList->count();
    for (int i = 0; i < count; ++i) {
        QListWidgetItem* item = m_thumbnailList->item(i);
        int pageIndex = item->data(Qt::UserRole).toInt() - 1; // 0-indexed
        QImage thumb = renderPageImage(pageIndex, thumbH);
        item->setIcon(QIcon(QPixmap::fromImage(thumb)));
    }
}
