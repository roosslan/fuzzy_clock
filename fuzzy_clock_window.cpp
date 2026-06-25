// #define QT_NO_DEBUG_OUTPUT
#include <stdio.h>
#include <QtGui>
#include <QtWidgets>
#include <Windows.h>
#include <WtsApi32.h>
#include <QDebug>
#include <QLabel>

#include "fuzzy_clock.h"
#include "fuzzy_clock_window.h"

/*
#include <functional>
#include "rxqt.hpp"
#include "rx.hpp"
void/int fuzzyClockWindow::DisplayTime(int a,int b)
{
    fuzzyClock.DisplayTime();
}

*/

fuzzyClockWindow::fuzzyClockWindow() : m_Settings("HKEY_CURRENT_USER\\SOFTWARE\\rasa\\FuzzyClock", QSettings::NativeFormat) // , fuzzyClock(this->winId() )
{
    this->setToolTip("TO DO: time as a hint");

/*    rxqt::from_event(this, QEvent::MouseButtonPress)
        .subscribe([this](const QEvent* e)
    {
        auto me = static_cast<const QMouseEvent*>(e);
        mpos = me->pos();
    });
*/

    /* TODO
    std::function<int(int,int)> funcWrapper;
    funcWrapper = DisplayTime(1, 2);

    auto scheduler = rxcpp::observe_on_new_thread();
    auto period = std::chrono::milliseconds(1000);
    auto counter = rxcpp::observable<>::interval(period, scheduler);
    counter.subscribe( [&](){ DisplayTime(1,2); },
                       [](std::exception_ptr){} );
    */


    ::SetTimer((HWND)this->winId(), ID_TIMER, 1000, (TIMERPROC) NULL);

    WTSRegisterSessionNotification((HWND)this->winId(), NOTIFY_FOR_THIS_SESSION);

    fuzzyClock::instance().SetLabel(new QLabel(this));
    fuzzyClock::instance().SetWindow(this);

    /* registry read */
    int wTop = m_Settings.value("Top", 300).toInt();
    int wLeft = m_Settings.value("Left", 300).toInt();
    this->setGeometry(wLeft, wTop, 200, 20);

    /* for CSS */
    setObjectName("fuzzyClockWindow");

    /* Context menu */
    createActions();
}

void fuzzyClockWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(aboutAct);
    menu.addAction(aboutQtAct);
    menu.addAction(exitAct);
    menu.exec(event->globalPos());
}

/* coule be replaced with rqQt/rxCPP */
void fuzzyClockWindow::mousePressEvent(QMouseEvent *event)
{
    mpos = event->pos();
}

// Drag window without title
void fuzzyClockWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        QPoint diff = event->pos() - mpos;
        QPoint newpos = this->pos() + diff;

        this->move(newpos);
    }
}

bool fuzzyClockWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
    if (eventType != "windows_generic_MSG") return false;
    MSG *msg = static_cast<MSG*>(message);

    switch (msg->message) {
      case WM_WTSSESSION_CHANGE:
        qDebug() << "Session changed: " << msg->wParam;
        if (msg->wParam == 7) {

        }
      case WM_TIMER:
        fuzzyClock::instance().DisplayTime();
    }
    return false;
}

fuzzyClockWindow::~fuzzyClockWindow()
{
    KillTimer((HWND)this->winId(), ID_TIMER);
    WTSUnRegisterSessionNotification((HWND)this->winId());
}


void fuzzyClockWindow::createActions()
{
    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, &QAction::triggered, this, &fuzzyClockWindow::about);

    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, &QAction::triggered, qApp, &QApplication::aboutQt);

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, &QAction::triggered, this, &fuzzyClockWindow::exit);
}

void fuzzyClockWindow::about()
{
    QMessageBox::about(this, tr("О \"Неточных\" часах..."),
            tr("\"Неточные\" часы v3.1.3,        <br/> (ремейк версии 2.1  "
               "от 12.01.2004)<br/>rasa"));
}

void fuzzyClockWindow::exit()
{
    m_Settings.setValue("currentPath", QDir::currentPath());
    QPoint XY = this->pos();
    m_Settings.setValue("Top", QString::number(XY.y()));
    m_Settings.setValue("Left", QString::number(XY.x()));
    QApplication::quit();
}

/* const A ca;          // A()
   A b(ca)              // A(const A&)      */
fuzzyClockWindow::fuzzyClockWindow(const fuzzyClockWindow &other) : fuzzyClockWindow()
{
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::SubWindow | Qt::Window | Qt::FramelessWindowHint);
    this->setWindowTitle(QApplication::translate("Fuzzy clock", "Неточные часы"));
}

/*  A c = ca;            // A(const A&)
    d = c                // =(const A&)     */
fuzzyClockWindow &fuzzyClockWindow::operator=(const fuzzyClockWindow &other)
{
    if (this == &other)
        return *this;
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::SubWindow | Qt::Window | Qt::FramelessWindowHint);
    this->setWindowTitle(QApplication::translate("Fuzzy clock", "Неточные часы"));
    return *this;
}

/* A d(std::move(c))    // A(A&&)           */
fuzzyClockWindow::fuzzyClockWindow(fuzzyClockWindow &&other) : fuzzyClockWindow()
{
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::SubWindow | Qt::Window | Qt::FramelessWindowHint);
    this->setWindowTitle(QApplication::translate("Fuzzy clock", "Неточные часы"));
}

/*  c = std::move(d)    // =A(A&&)          */
fuzzyClockWindow &fuzzyClockWindow::operator=(fuzzyClockWindow &&other)
{
    if (this == &other)
        return *this;
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::SubWindow | Qt::Window | Qt::FramelessWindowHint);
    this->setWindowTitle(QApplication::translate("Fuzzy clock", "Неточные часы"));
    return *this;
}
