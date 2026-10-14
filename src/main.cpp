#include <QtWidgets/QApplication>
#include <QDir>
#include <QFile>
#include "fuzzy_clock_window.h"

void SetCSS(QApplication &app);

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    // до создания окна: задаёт текущую директорию, из которой читается fuzzy.conf
    SetCSS(app);

    fuzzyClockWindow fuzzyWindow;
    fuzzyWindow.show();

    return app.exec();
}

void SetCSS(QApplication &app)
{
    QDir::setCurrent(QCoreApplication::applicationDirPath());
    QFile styleFile("style.css");
    if (styleFile.open(QFile::ReadOnly))
        app.setStyleSheet(QString::fromUtf8(styleFile.readAll()));
}
