#include <QtWidgets>
#include <Windows.h>
#include <WtsApi32.h>

#include "fuzzy_clock_window.h"
#include "fuzzy_helper.h"

fuzzyClockWindow::fuzzyClockWindow()
    : m_label(new QLabel(this))
    , m_Settings("HKEY_CURRENT_USER\\SOFTWARE\\rasa\\FuzzyClock", QSettings::NativeFormat)
{
    // флаги задаются до создания нативного окна (winId)
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::SubWindow | Qt::Window | Qt::FramelessWindowHint);
    setWindowTitle(tr("Неточные часы"));

    /* for CSS */
    setObjectName("fuzzyClockWindow");
    m_label->setObjectName("timeLabel");
    m_label->move(5, 5);

    fuzzyHelper::instance()->readArrays(m_clock);  // конфиг читается один раз при старте

    WTSRegisterSessionNotification((HWND)winId(), NOTIFY_FOR_THIS_SESSION);

    m_timer.setSingleShot(true);
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, &fuzzyClockWindow::updateTime);

    restorePosition();

    /* Context menu */
    createActions();

    updateTime();
}

fuzzyClockWindow::~fuzzyClockWindow()
{
    // internalWinId() не создаёт окно заново, в отличие от winId()
    if (WId id = internalWinId())
        WTSUnRegisterSessionNotification((HWND)id);
}

void fuzzyClockWindow::updateTime()
{
    const QTime now = QTime::currentTime();
    m_label->setText(m_clock.text(now));
    setToolTip(now.toString("HH:mm"));             // точное время - в подсказке
    m_label->adjustSize();                          // учитывает шрифт из CSS (вызывает ensurePolished)
    setFixedSize(m_label->size() + QSize(10, 10));

    // следующее обновление - в начале следующей минуты
    m_timer.start(60000 - now.msecsSinceStartOfDay() % 60000);
}

void fuzzyClockWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(aboutAct);
    menu.addAction(aboutQtAct);
    menu.addAction(exitAct);
    menu.exec(event->globalPos());
}

void fuzzyClockWindow::mousePressEvent(QMouseEvent *event)
{
    mpos = event->pos();
}

// Drag window without title
void fuzzyClockWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
        move(pos() + event->pos() - mpos);
}

void fuzzyClockWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        savePosition();                             // перетаскивание закончено
}

void fuzzyClockWindow::closeEvent(QCloseEvent *event)
{
    savePosition();                                 // и через меню, и по Alt+F4
    QWidget::closeEvent(event);
    QApplication::quit();
}

bool fuzzyClockWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
    if (eventType == "windows_generic_MSG") {
        const MSG *msg = static_cast<const MSG *>(message);
        // после разблокировки (и выхода из сна) сразу обновить время и перезапустить таймер
        if (msg->message == WM_WTSSESSION_CHANGE && msg->wParam == WTS_SESSION_UNLOCK)
            updateTime();
    }
    return QWidget::nativeEvent(eventType, message, result);
}

void fuzzyClockWindow::savePosition()
{
    m_Settings.setValue("currentPath", QDir::currentPath());
    m_Settings.setValue("Top", pos().y());
    m_Settings.setValue("Left", pos().x());
}

void fuzzyClockWindow::restorePosition()
{
    QPoint p(m_Settings.value("Left", 300).toInt(), m_Settings.value("Top", 300).toInt());
    // если монитор, на котором было окно, отключён, окно переносится на основной экран
    if (!QGuiApplication::screenAt(p))
        p = QGuiApplication::primaryScreen()->availableGeometry().center();
    move(p);
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
    connect(exitAct, &QAction::triggered, this, &QWidget::close);
}

void fuzzyClockWindow::about()
{
    QMessageBox::about(this, tr("О \"Неточных\" часах..."),
            tr("\"Неточные\" часы v3.1.3,        <br/> (ремейк версии 2.1  "
               "от 12.01.2004)<br/>rasa"));
}
