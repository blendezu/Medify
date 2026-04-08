#include "SplitPdfWidget.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QToolButton>
#include <QPainter>
#include <QFrame>
#include <QSlider>
#include <QListWidget>

// Range accent colors (turquoise, amber, coral, lime, purple, orange)
static const QList<QColor> RANGE_COLORS = {
    QColor("#40e0d0"), QColor("#f0a832"), QColor("#e05060"),
    QColor("#60d060"), QColor("#9060e8"), QColor("#e08040")
};

// ─────────────────────────────────────────────────────────────────────────────
// RangeRow
// ─────────────────────────────────────────────────────────────────────────────
RangeRow::RangeRow(int index, int max, QWidget* parent) : QWidget(parent) {
    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(8, 6, 8, 4);
    outer->setSpacing(3);

    QHBoxLayout* hdr = new QHBoxLayout();
    m_label = new QLabel(QString("↕  Range %1").arg(index));
    QColor c = RANGE_COLORS[(index - 1) % RANGE_COLORS.size()];
    m_label->setStyleSheet(QString("font-weight: bold; color: %1; font-size: 13px;").arg(c.name()));
    QPushButton* del = new QPushButton("×");
    del->setObjectName("removeBtn");
    del->setFixedSize(20, 20);
    del->setCursor(Qt::PointingHandCursor);
    connect(del, &QPushButton::clicked, this, &RangeRow::removeRequested);
    hdr->addWidget(m_label);
    hdr->addStretch();
    hdr->addWidget(del);
    outer->addLayout(hdr);

    QHBoxLayout* row = new QHBoxLayout();
    QLabel* fl = new QLabel("from page");
    fl->setStyleSheet("color: #999; font-size: 11px;");
    m_fromSpin = new QSpinBox();
    m_fromSpin->setRange(1, max); m_fromSpin->setValue(1);
    m_fromSpin->setFixedWidth(55);
    QLabel* tl = new QLabel("to");
    tl->setStyleSheet("color: #999; font-size: 11px;");
    m_toSpin = new QSpinBox();
    m_toSpin->setRange(1, max); m_toSpin->setValue(max);
    m_toSpin->setFixedWidth(55);
    row->addWidget(fl); row->addWidget(m_fromSpin);
    row->addWidget(tl); row->addWidget(m_toSpin);
    row->addStretch();
    outer->addLayout(row);

    connect(m_fromSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int v){
        if (m_toSpin->value() < v) m_toSpin->setValue(v);
        emit changed();
    });
    connect(m_toSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int v){
        if (m_fromSpin->value() > v) m_fromSpin->setValue(v);
        emit changed();
    });
}

int RangeRow::fromPage() const { return m_fromSpin->value(); }
int RangeRow::toPage()   const { return m_toSpin->value();   }
void RangeRow::setIndex(int i) {
    m_label->setText(QString("↕  Range %1").arg(i));
    QColor c = RANGE_COLORS[(i - 1) % RANGE_COLORS.size()];
    m_label->setStyleSheet(QString("font-weight: bold; color: %1; font-size: 13px;").arg(c.name()));
}
void RangeRow::setMax(int max) {
    m_fromSpin->setMaximum(max);
    m_toSpin->setMaximum(max);
}
void RangeRow::setFromTo(int from, int to) {
    // Block signals to avoid triggering changed() during initialization
    m_fromSpin->blockSignals(true);
    m_toSpin->blockSignals(true);
    m_fromSpin->setValue(from);
    m_toSpin->setValue(to);
    m_fromSpin->blockSignals(false);
    m_toSpin->blockSignals(false);
}

// ─────────────────────────────────────────────────────────────────────────────
// SplitPdfWidget
// ─────────────────────────────────────────────────────────────────────────────
SplitPdfWidget::SplitPdfWidget(QWidget* parent)
    : QWidget(parent),
      m_pdfDocument(new QPdfDocument(this)),
      m_currentThumbH(130),
      m_workerThread(nullptr), m_worker(nullptr)
{
    setAcceptDrops(true);
    setupUi();
}

void SplitPdfWidget::setupUi() {
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(15, 15, 15, 15);
    root->setSpacing(10);

    // ── Header ─────────────────────────────────────────────────────────────
    QHBoxLayout* header = new QHBoxLayout();
    QPushButton* backBtn = new QPushButton("< Back");
    backBtn->setObjectName("backBtn");
    backBtn->setCursor(Qt::PointingHandCursor);
    connect(backBtn, &QPushButton::clicked, this, &SplitPdfWidget::requestBackToDashboard);
    QLabel* title = new QLabel("Split PDF");
    title->setStyleSheet("font-size: 18px; font-weight: bold; color: #40e0d0;");
    header->addWidget(backBtn); header->addStretch();
    header->addWidget(title);  header->addStretch();
    root->addLayout(header);

    // ── Internal stack ──────────────────────────────────────────────────────
    m_internalStack = new QStackedWidget(this);

    // Page 0: drop zone
    QWidget* dropZone = new QWidget();
    dropZone->setObjectName("dropZone");
    QVBoxLayout* dz = new QVBoxLayout(dropZone);
    dz->setAlignment(Qt::AlignCenter);
    QLabel* dzLbl = new QLabel("Drag & Drop a PDF or");
    dzLbl->setAlignment(Qt::AlignCenter);
    QPushButton* selBtn = new QPushButton("Select PDF File");
    selBtn->setObjectName("selectFilesButton");
    selBtn->setCursor(Qt::PointingHandCursor);
    connect(selBtn, &QPushButton::clicked, this, &SplitPdfWidget::onSelectFileClicked);
    dz->addStretch(); dz->addWidget(dzLbl);
    dz->addWidget(selBtn, 0, Qt::AlignCenter); dz->addStretch();
    m_internalStack->addWidget(dropZone);

    // Page 1: main editor — left (page grid) + right (ranges)
    QWidget* mainPage = new QWidget();
    QHBoxLayout* mainH = new QHBoxLayout(mainPage);
    mainH->setContentsMargins(0, 0, 0, 0);
    mainH->setSpacing(10);

    // ── Left: all pages as zoomable grid ───────────────────────────────────
    QWidget* leftPane = new QWidget();
    QVBoxLayout* leftLay = new QVBoxLayout(leftPane);
    leftLay->setContentsMargins(0, 0, 0, 0);
    leftLay->setSpacing(6);

    // Zoom toolbar
    QHBoxLayout* zoomBar = new QHBoxLayout();
    QLabel* zoomIco = new QLabel("🔍");
    m_zoomSlider = new QSlider(Qt::Horizontal);
    m_zoomSlider->setRange(70, 260);
    m_zoomSlider->setValue(130);
    m_zoomSlider->setFixedWidth(120);
    m_zoomSlider->setToolTip("Thumbnail size");
    connect(m_zoomSlider, &QSlider::valueChanged, this, &SplitPdfWidget::onZoomChanged);
    QLabel* hint = new QLabel("Click a range row to highlight pages  •  Double-click page for preview");
    hint->setStyleSheet("color: #777; font-size: 11px;");
    zoomBar->addWidget(zoomIco);
    zoomBar->addWidget(m_zoomSlider);
    zoomBar->addStretch();
    zoomBar->addWidget(hint);
    leftLay->addLayout(zoomBar);

    m_pageList = new QListWidget();
    m_pageList->setViewMode(QListView::IconMode);
    m_pageList->setIconSize(QSize(85, 110));
    m_pageList->setGridSize(QSize(110, 155));
    m_pageList->setResizeMode(QListView::Adjust);
    m_pageList->setSelectionMode(QAbstractItemView::NoSelection);
    m_pageList->setMovement(QListView::Static);
    m_pageList->setWrapping(true);
    m_pageList->setSpacing(4);
    m_pageList->setStyleSheet(
        "QListWidget { background: #252525; border: 1px solid #444; border-radius: 8px; padding: 6px; }"
        "QListWidget::item { color: #ccc; font-size: 10px; }"
    );
    connect(m_pageList, &QListWidget::itemDoubleClicked, this, &SplitPdfWidget::onPageDoubleClicked);
    leftLay->addWidget(m_pageList);

    mainH->addWidget(leftPane, 1);

    // ── Right: ranges panel ─────────────────────────────────────────────────
    QWidget* rightPanel = new QWidget();
    rightPanel->setFixedWidth(255);
    rightPanel->setObjectName("settingsPanel");
    rightPanel->setStyleSheet(
        "#settingsPanel { background: #2e2e2e; border: 1px solid #444; border-radius: 8px; }"
    );
    QVBoxLayout* rpL = new QVBoxLayout(rightPanel);
    rpL->setContentsMargins(10, 10, 10, 10);
    rpL->setSpacing(6);

    QLabel* splitTitle = new QLabel("Split");
    splitTitle->setStyleSheet("font-size: 15px; font-weight: bold; color: #fff;");
    splitTitle->setAlignment(Qt::AlignCenter);
    rpL->addWidget(splitTitle);

    QFrame* sep = new QFrame(); sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color: #555;"); rpL->addWidget(sep);

    QScrollArea* rowsScroll = new QScrollArea();
    rowsScroll->setWidgetResizable(true);
    rowsScroll->setStyleSheet("QScrollArea { border: none; background: transparent; }");
    m_rangeRowsContainer = new QWidget();
    m_rangeRowsContainer->setStyleSheet("background: transparent;");
    m_rangeRowsLayout = new QVBoxLayout(m_rangeRowsContainer);
    m_rangeRowsLayout->setContentsMargins(0, 0, 0, 0);
    m_rangeRowsLayout->setSpacing(4);
    m_rangeRowsLayout->addStretch();
    rowsScroll->setWidget(m_rangeRowsContainer);
    rpL->addWidget(rowsScroll, 1);

    QPushButton* addRangeBtn = new QPushButton("+ Add Range");
    addRangeBtn->setObjectName("addMoreButton");
    addRangeBtn->setCursor(Qt::PointingHandCursor);
    connect(addRangeBtn, &QPushButton::clicked, this, &SplitPdfWidget::onAddRangeClicked);
    rpL->addWidget(addRangeBtn);

    QPushButton* changePdfBtn = new QPushButton("↩ Change PDF");
    changePdfBtn->setObjectName("addMoreButton");
    changePdfBtn->setCursor(Qt::PointingHandCursor);
    connect(changePdfBtn, &QPushButton::clicked, this, &SplitPdfWidget::onSelectFileClicked);
    rpL->addWidget(changePdfBtn);

    mainH->addWidget(rightPanel, 0);

    m_internalStack->addWidget(mainPage);
    root->addWidget(m_internalStack, 1);

    // ── Bottom ──────────────────────────────────────────────────────────────
    QHBoxLayout* nameRow = new QHBoxLayout();
    QLabel* nameLbl = new QLabel("File Name:");
    nameLbl->setFixedWidth(90);
    m_baseNameEdit = new QLineEdit("split");
    m_baseNameEdit->setPlaceholderText("Base name  →  split_1-7.pdf, split_8-12.pdf …");
    nameRow->addWidget(nameLbl); nameRow->addWidget(m_baseNameEdit);
    root->addLayout(nameRow);

    QHBoxLayout* saveRow = new QHBoxLayout();
    QLabel* saveLbl = new QLabel("Save To:");
    saveLbl->setFixedWidth(90);
    m_saveDirectoryEdit = new QLineEdit();
    m_saveDirectoryEdit->setReadOnly(true);
    m_saveDirectoryEdit->setText(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    QToolButton* browseBtn = new QToolButton();
    browseBtn->setText("📁"); browseBtn->setCursor(Qt::PointingHandCursor);
    connect(browseBtn, &QToolButton::clicked, this, &SplitPdfWidget::onBrowseSaveDirectory);
    saveRow->addWidget(saveLbl); saveRow->addWidget(m_saveDirectoryEdit); saveRow->addWidget(browseBtn);
    root->addLayout(saveRow);

    m_progressBar = new QProgressBar();
    m_progressBar->setRange(0, 0); m_progressBar->setVisible(false);
    root->addWidget(m_progressBar);

    m_processButton = new QPushButton("Split PDF");
    m_processButton->setObjectName("compressButton");
    m_processButton->setCursor(Qt::PointingHandCursor);
    m_processButton->setMinimumHeight(40);
    m_processButton->setEnabled(false);
    connect(m_processButton, &QPushButton::clicked, this, &SplitPdfWidget::onProcessClicked);
    root->addWidget(m_processButton);
}

// ── Drag & Drop ─────────────────────────────────────────────────────────────
void SplitPdfWidget::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        const auto urls = event->mimeData()->urls();
        if (!urls.isEmpty() && urls.first().toLocalFile().endsWith(".pdf", Qt::CaseInsensitive))
            event->acceptProposedAction();
    }
}
void SplitPdfWidget::dropEvent(QDropEvent* event) {
    const auto urls = event->mimeData()->urls();
    if (!urls.isEmpty()) loadPdf(urls.first().toLocalFile());
}
void SplitPdfWidget::onSelectFileClicked() {
    QString path = QFileDialog::getOpenFileName(
        this, "Select PDF",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "PDF Files (*.pdf)");
    if (!path.isEmpty()) loadPdf(path);
}
void SplitPdfWidget::onBrowseSaveDirectory() {
    QString d = QFileDialog::getExistingDirectory(this, "Select Save Directory", m_saveDirectoryEdit->text());
    if (!d.isEmpty()) m_saveDirectoryEdit->setText(d);
}

// ── Load PDF ─────────────────────────────────────────────────────────────────
void SplitPdfWidget::loadPdf(const QString& path) {
    m_pdfDocument->close();
    m_pdfDocument->load(path);
    int pages = m_pdfDocument->pageCount();
    if (pages <= 0) { QMessageBox::critical(this, "Error", "Could not load the PDF."); return; }

    m_currentPdfPath = path;
    m_baseNameEdit->setText(QFileInfo(path).completeBaseName() + "_split");

    qDeleteAll(m_rows); m_rows.clear();
    onAddRangeClicked(); // default first range covering all

    m_internalStack->setCurrentIndex(1);
    m_processButton->setEnabled(true);
    rebuildPageList();
}

// ── Render helpers ───────────────────────────────────────────────────────────
QImage SplitPdfWidget::renderPageImage(int pageIndex, int targetH) {
    QSizeF ps = m_pdfDocument->pagePointSize(pageIndex);
    double aspect = (ps.height() > 0) ? ps.width() / ps.height() : 0.77;
    int w = qMax(40, (int)(targetH * aspect));
    QImage canvas(w, targetH, QImage::Format_ARGB32);
    canvas.fill(Qt::white);
    QImage rendered = m_pdfDocument->render(pageIndex, QSize(w, targetH));
    QPainter p(&canvas);
    p.drawImage(0, 0, rendered);
    p.end();
    return canvas;
}

QImage SplitPdfWidget::renderPageWithHighlight(int pageIndex, int targetH, int rangeIdx) {
    QImage canvas = renderPageImage(pageIndex, targetH);

    if (rangeIdx >= 0) {
        QColor c = RANGE_COLORS[rangeIdx % RANGE_COLORS.size()];
        QPainter p(&canvas);

        // Colored border
        p.setPen(QPen(c, 4));
        p.setBrush(Qt::NoBrush);
        p.drawRect(canvas.rect().adjusted(2, 2, -2, -2));

        // Badge
        QRect badge(0, 0, 24, 17);
        p.setBrush(c);
        p.setPen(Qt::NoPen);
        p.drawRect(badge);
        p.setPen(Qt::black);
        QFont f; f.setBold(true); f.setPointSize(8);
        p.setFont(f);
        p.drawText(badge, Qt::AlignCenter, QString::number(rangeIdx + 1));
        p.end();
    }
    return canvas;
}

// ── Rebuild page list ────────────────────────────────────────────────────────
void SplitPdfWidget::rebuildPageList() {
    m_pageList->clear();
    int total = m_pdfDocument->pageCount();
    if (total <= 0) return;

    // Build page → range index map
    QVector<int> pageRange(total + 1, -1);
    for (int ri = 0; ri < m_rows.size(); ++ri)
        for (int p = m_rows[ri]->fromPage(); p <= m_rows[ri]->toPage(); ++p)
            if (p >= 1 && p <= total) pageRange[p] = ri;

    int thumbH = m_zoomSlider->value();
    int thumbW = qMax(40, (int)(thumbH * 0.77));

    m_pageList->setIconSize(QSize(thumbW, thumbH));
    m_pageList->setGridSize(QSize(thumbW + 20, thumbH + 35));

    for (int i = 0; i < total; ++i) {
        QImage img = renderPageWithHighlight(i, thumbH, pageRange[i + 1]);
        QListWidgetItem* item = new QListWidgetItem();
        item->setIcon(QIcon(QPixmap::fromImage(img)));
        item->setText(QString("Page %1").arg(i + 1));
        item->setData(Qt::UserRole, i + 1);
        m_pageList->addItem(item);
    }
}

// ── Zoom ─────────────────────────────────────────────────────────────────────
void SplitPdfWidget::onZoomChanged(int) {
    if (!m_currentPdfPath.isEmpty()) rebuildPageList();
}

// ── Double-click preview ─────────────────────────────────────────────────────
void SplitPdfWidget::onPageDoubleClicked(QListWidgetItem* item) {
    int pageIndex = item->data(Qt::UserRole).toInt() - 1;
    QImage preview = renderPageImage(pageIndex, 900);

    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle(QString("Page %1").arg(pageIndex + 1));
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    QVBoxLayout* lay = new QVBoxLayout(dialog);
    lay->setContentsMargins(0, 0, 0, 0);
    QScrollArea* sa = new QScrollArea(dialog);
    sa->setStyleSheet("QScrollArea { border: none; background: #1e1e1e; }");
    QLabel* lbl = new QLabel();
    lbl->setPixmap(QPixmap::fromImage(preview));
    lbl->setStyleSheet("background: white; padding: 6px;");
    sa->setWidget(lbl);
    dialog->resize(qMin(preview.width() + 30, 950), qMin(preview.height() + 30, 860));
    lay->addWidget(sa);
    dialog->exec();
}

// ── Range management ─────────────────────────────────────────────────────────
void SplitPdfWidget::onAddRangeClicked() {
    int maxPage = qMax(1, m_pdfDocument->pageCount());

    // New range starts right after the last existing range
    int nextFrom = 1;
    if (!m_rows.isEmpty()) {
        nextFrom = qMin(m_rows.last()->toPage() + 1, maxPage);
    }

    RangeRow* row = new RangeRow(m_rows.size() + 1, maxPage, m_rangeRowsContainer);
    // Set sensible defaults: from = nextFrom, to = maxPage
    row->setFromTo(nextFrom, maxPage);

    int pos = m_rangeRowsLayout->count() - 1;
    if (pos < 0) pos = 0;
    m_rangeRowsLayout->insertWidget(pos, row);
    m_rows.append(row);
    connect(row, &RangeRow::changed, this, &SplitPdfWidget::onRangeChanged);
    connect(row, &RangeRow::removeRequested, this, [this, row](){
        int idx = m_rows.indexOf(row);
        if (idx >= 0) removeRange(idx);
    });
    if (!m_currentPdfPath.isEmpty()) rebuildPageList();
}


void SplitPdfWidget::removeRange(int index) {
    if (index < 0 || index >= m_rows.size()) return;
    RangeRow* row = m_rows.takeAt(index);
    m_rangeRowsLayout->removeWidget(row);
    row->deleteLater();
    renumberRanges();
    if (!m_currentPdfPath.isEmpty()) rebuildPageList();
}

void SplitPdfWidget::renumberRanges() {
    for (int i = 0; i < m_rows.size(); ++i) m_rows[i]->setIndex(i + 1);
}

void SplitPdfWidget::onRangeChanged() {
    if (!m_currentPdfPath.isEmpty()) rebuildPageList();
}

// ── Process ──────────────────────────────────────────────────────────────────
void SplitPdfWidget::onProcessClicked() {
    if (m_currentPdfPath.isEmpty() || m_rows.isEmpty()) return;
    QString saveDir = m_saveDirectoryEdit->text().trimmed();
    if (saveDir.isEmpty()) { QMessageBox::warning(this, "No Directory", "Select a save directory."); return; }
    QString baseName = m_baseNameEdit->text().trimmed();
    if (baseName.isEmpty()) baseName = "split";

    int total = m_pdfDocument->pageCount();
    QVariantList ranges;
    for (auto* row : m_rows) {
        QVariantMap r;
        r["from"] = qBound(1, row->fromPage(), total);
        r["to"]   = qBound(1, row->toPage(),   total);
        ranges.append(r);
    }
    QVariantMap settings;
    settings["ranges"] = ranges; settings["baseName"] = baseName;

    m_processButton->setEnabled(false);
    m_progressBar->setVisible(true);

    m_workerThread = new QThread(this);
    m_worker = new SplitPdfWorker({m_currentPdfPath}, saveDir, settings);
    m_worker->moveToThread(m_workerThread);
    connect(m_workerThread, &QThread::started, m_worker, &SplitPdfWorker::start);
    connect(m_worker, &SplitPdfWorker::finished, this, &SplitPdfWidget::onWorkerFinished);
    m_workerThread->start();
}

void SplitPdfWidget::onWorkerFinished(bool success, const QString& error) {
    m_progressBar->setVisible(false); m_processButton->setEnabled(true);
    if (success) {
        QMessageBox::information(this, "Done",
            QString("%1 PDF file(s) saved to:\n%2").arg(m_rows.size()).arg(m_saveDirectoryEdit->text()));
    } else {
        QMessageBox::critical(this, "Error", "Split failed:\n" + error);
    }
    if (m_workerThread) {
        m_workerThread->quit(); m_workerThread->wait();
        m_worker->deleteLater(); m_workerThread->deleteLater();
        m_worker = nullptr; m_workerThread = nullptr;
    }
}
