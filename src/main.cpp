#include <QApplication>
#include <QMessageBox>
#include <QStandardPaths>
#include <QPixmap>
#include <QPainter>
#include <QFont>
#include "ui/MainWindow.h"

bool checkDependency(const QString& program) {
    if (!QStandardPaths::findExecutable(program).isEmpty()) {
        return true;
    }
#ifdef Q_OS_WIN
    if (!QStandardPaths::findExecutable(program + ".exe").isEmpty()) {
        return true;
    }
#endif
    return false;
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Set emoji app icon
    QPixmap iconPixmap(128, 128);
    iconPixmap.fill(Qt::transparent);
    QPainter painter(&iconPixmap);
    QFont emojiFont;
    emojiFont.setPointSize(80);
    painter.setFont(emojiFont);
    painter.drawText(iconPixmap.rect(), Qt::AlignCenter, "📎");
    painter.end();
    app.setWindowIcon(QIcon(iconPixmap));

    bool hasFFmpeg = checkDependency("ffmpeg");
    bool hasFfprobe = checkDependency("ffprobe");
    bool hasGs = checkDependency("gs");
#ifdef Q_OS_WIN
    if (!hasGs) hasGs = checkDependency("gswin64c") || checkDependency("gswin32c");
#endif

    if (!hasFFmpeg || !hasFfprobe || !hasGs) {
        QString msg = "Some required external tools are missing:\n";
        msg += hasFFmpeg ? "" : "- ffmpeg (needed for exact video/audio output sizing)\n";
        msg += hasFfprobe ? "" : "- ffprobe (needed for duration checks)\n";
        msg += hasGs ? "" : "- gs (Ghostscript, needed for PDF)\n";
        msg += "\nPlease install them and ensure they are on your system PATH.";
        QMessageBox::warning(nullptr, "Missing Dependencies", msg);
    }

    MainWindow w(hasFFmpeg, hasFfprobe, hasGs);
    w.show();
    return app.exec();
}
