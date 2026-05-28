#include <QtWidgets/QApplication>
#include <Windows.h>
#include <WtsApi32.h>
#include <QDebug>
#include <QLabel>
#include "fuzzy_clock_window.h"

void createFuzzyWindow(fuzzyClockWindow &fuzzyDlgWindow);
void  SetCSS(QApplication &app);

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    fuzzyClockWindow fWindowCC, fWindowMv;
    fuzzyClockWindow fWindowCA(fWindowCC);

    createFuzzyWindow(fWindowCC); // but not show
    fWindowMv = fWindowCA;

    fuzzyClockWindow fWindowMA(std::move(fWindowMv));

    // =A(A&&)
    fuzzyClockWindow fuzzyWindow;
    fuzzyWindow = std::move(fWindowMA);

    SetCSS(app);

    fuzzyWindow.show();

    return app.exec();
}

void createFuzzyWindow(fuzzyClockWindow &fuzzyDlgWindow)
{
    fuzzyDlgWindow.setWindowFlags(Qt::WindowStaysOnTopHint | Qt::SubWindow | Qt::Window | Qt::FramelessWindowHint);
    fuzzyDlgWindow.setWindowTitle(QApplication::translate("Fuzzy clock", "Неточные часы"));
}

void  SetCSS(QApplication &app)
{
    QDir::setCurrent(QCoreApplication::applicationDirPath());
    QFile styleFile("style.css");
    styleFile.open(QFile::ReadOnly);
    QString stylecss(styleFile.readAll());
    app.setStyleSheet(stylecss);
};
