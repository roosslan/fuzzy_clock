#include <QtWidgets>
#include <Windows.h>
#include <WtsApi32.h>
#include <commdlg.h>
#include <iterator>

#include "fuzzy_clock_window.h"
#include "fuzzy_dock.h"
#include "fuzzy_helper.h"

namespace {

const char RunKey[] = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
const char RunValue[] = "FuzzyClock";

// цвета по умолчанию совпадают со style.css
const QColor DefaultBackground(Qt::black);
const QColor DefaultText(0x00, 0xff, 0x00);     // lime
const QColor DefaultBorder(0x00, 0xff, 0x00);

struct ColorScheme
{
    const char *name;
    QColor background;
    QColor text;
    QColor border;
};

const ColorScheme Schemes[] = {
    { "Красно-белая", QColor(Qt::white),         QColor(0xb0, 0x00, 0x00), QColor(0xb0, 0x00, 0x00) },
    { "Сине-жёлтая",  QColor(0x00, 0x2f, 0x87), QColor(Qt::yellow),        QColor(Qt::yellow)        },
    { "Чёрно-зелёная", DefaultBackground,        DefaultText,              DefaultBorder             },
};

struct Opacity
{
    const char *name;
    double value;
};

// процент - видимость окна: 100% - обычное непрозрачное окно
const Opacity Opacities[] = { { "10%", 0.1 }, { "50%", 0.5 }, { "100%", 1.0 } };

const char *const FuzzinessNames[] = { "Четверть пятого", "Вечер", "Середина недели" };

// на каком расстоянии от края экрана окно к нему притягивается
constexpr int SnapDistance = 15;

QString autostartCommand()
{
    return "\"" + QDir::toNativeSeparators(QCoreApplication::applicationFilePath()) + "\"";
}

COLORREF toColorRef(const QColor &color)
{
    return RGB(color.red(), color.green(), color.blue());
}

QColor fromColorRef(COLORREF color)
{
    return QColor(GetRValue(color), GetGValue(color), GetBValue(color));
}

// стандартное окно выбора цвета Windows
bool chooseColor(HWND owner, QColor &color)
{
    static COLORREF customColors[16] = {};      // "Дополнительные цвета" сохраняются до выхода из программы

    CHOOSECOLORW cc = {};
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = owner;
    cc.rgbResult = toColorRef(color);
    cc.lpCustColors = customColors;
    cc.Flags = CC_RGBINIT | CC_FULLOPEN;
    if (!ChooseColorW(&cc))
        return false;
    color = fromColorRef(cc.rgbResult);
    return true;
}

}

fuzzyClockWindow::fuzzyClockWindow()
    : m_label(new QLabel(this))
    , m_Settings("HKEY_CURRENT_USER\\SOFTWARE\\rasa\\FuzzyClock", QSettings::NativeFormat)
{
    // флаги задаются до создания нативного окна (winId)
    Qt::WindowFlags flags = Qt::SubWindow | Qt::Window | Qt::FramelessWindowHint;
    if (m_Settings.value("OnTop", true).toBool())
        flags |= Qt::WindowStaysOnTopHint;
    setWindowFlags(flags);
    setWindowTitle(tr("Неточные часы"));

    /* for CSS */
    setObjectName("fuzzyClockWindow");
    setAttribute(Qt::WA_StyledBackground);          // без этого рамка из CSS у наследника QWidget не рисуется
    m_label->setObjectName("timeLabel");
    m_label->move(5, 5);

    fuzzyHelper::instance()->readArrays(m_clock);  // конфиг читается один раз при старте

    WTSRegisterSessionNotification((HWND)winId(), NOTIFY_FOR_THIS_SESSION);

    m_timer.setSingleShot(true);
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, &fuzzyClockWindow::updateTime);

    restorePosition();
    setWindowOpacity(m_Settings.value("Opacity", 1.0).toDouble());

    // панель задач переместилась, сменилось разрешение - окно остаётся у своих краёв
    const auto watchScreen = [this](QScreen *screen) {
        connect(screen, &QScreen::availableGeometryChanged, this, &fuzzyClockWindow::applyDock);
    };
    for (QScreen *screen : QGuiApplication::screens())
        watchScreen(screen);
    connect(qApp, &QGuiApplication::screenAdded, this, watchScreen);
    connect(qApp, &QGuiApplication::screenRemoved, this, &fuzzyClockWindow::applyDock, Qt::QueuedConnection);

    /* Context menu */
    createMenu();

    applyStyle();                                   // заодно выводит время
}

fuzzyClockWindow::~fuzzyClockWindow()
{
    // internalWinId() не создаёт окно заново, в отличие от winId()
    if (WId id = internalWinId())
        WTSUnRegisterSessionNotification((HWND)id);
}

fuzzyClock::Fuzziness fuzzyClockWindow::fuzziness() const
{
    const int mode = m_Settings.value("Fuzziness", 0).toInt();
    if (mode < 0 || mode >= int(std::size(FuzzinessNames)))
        return fuzzyClock::Fuzziness::Minutes;
    return fuzzyClock::Fuzziness(mode);
}

void fuzzyClockWindow::updateTime()
{
    const QDateTime now = QDateTime::currentDateTime();
    m_label->setText(m_clock.text(now, fuzziness()));
    // точное время и дата - в подсказке
    setToolTip(QLocale(QLocale::Russian).toString(now, "HH:mm, dddd, d MMMM"));
    m_label->adjustSize();                          // учитывает шрифт из CSS (вызывает ensurePolished)
    setFixedSize(m_label->size() + QSize(10, 10));
    applyDock();                                    // при прилипании справа или снизу окно растёт внутрь экрана

    // следующее обновление - в начале следующей минуты
    m_timer.start(60000 - now.time().msecsSinceStartOfDay() % 60000);
}

QColor fuzzyClockWindow::storedColor(const QString &key, const QColor &fallback) const
{
    const QColor color(m_Settings.value(key).toString());
    return color.isValid() ? color : fallback;
}

void fuzzyClockWindow::applyStyle()
{
    QStringList window, label;

    const QString background = m_Settings.value("BackgroundColor").toString();
    if (!background.isEmpty())
        window << "background-color: " + background;
    const QString border = m_Settings.value("BorderColor").toString();
    if (!border.isEmpty())
        window << "border: 1px solid " + border;
    const QString text = m_Settings.value("TextColor").toString();
    if (!text.isEmpty())
        label << "color: " + text;

    QFont font;
    const QString fontString = m_Settings.value("Font").toString();
    if (!fontString.isEmpty() && font.fromString(fontString)) {
        QStringList decoration;
        if (font.underline())
            decoration << "underline";
        if (font.strikeOut())
            decoration << "line-through";
        label << QString("font-family: \"%1\"").arg(font.family())
              << QString("font-size: %1pt").arg(font.pointSizeF())
              << QString("font-weight: %1").arg(int(font.weight()))
              << QString("font-style: %1").arg(font.italic() ? "italic" : "normal")
              << "text-decoration: " + (decoration.isEmpty() ? QString("none") : decoration.join(' '));
    }

    // стиль окна имеет приоритет над style.css, не заданные здесь свойства берутся из style.css
    setStyleSheet(QString("QWidget#fuzzyClockWindow { %1 } QWidget#timeLabel { %2 }")
                  .arg(window.join("; "), label.join("; ")));
    updateTime();                                   // размер окна зависит от шрифта
}

void fuzzyClockWindow::contextMenuEvent(QContextMenuEvent *event)
{
    updateMenuChecks();
    m_menu->exec(event->globalPos());
}

void fuzzyClockWindow::mousePressEvent(QMouseEvent *event)
{
    mpos = event->pos();
}

// Drag window without title; Shift отключает прилипание к краям
void fuzzyClockWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;
    const QPoint p = pos() + event->pos() - mpos;
    if (event->modifiers() & Qt::ShiftModifier)
        move(p);
    else
        move(fuzzyDock::snapped(QRect(p, size()), dockArea(p), SnapDistance));
}

void fuzzyClockWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dock = fuzzyDock::edges(geometry(), dockArea(pos()));
        savePosition();                             // перетаскивание закончено
    }
}

QRect fuzzyClockWindow::dockArea(const QPoint &topLeft) const
{
    const QScreen *screen = QGuiApplication::screenAt(QRect(topLeft, size()).center());
    if (!screen)
        screen = QGuiApplication::screenAt(pos());
    if (!screen)
        screen = QGuiApplication::primaryScreen();
    return screen->availableGeometry();             // без панели задач
}

void fuzzyClockWindow::applyDock()
{
    if (!m_dock)
        return;
    const QPoint p = fuzzyDock::docked(geometry(), dockArea(pos()), m_dock);
    if (p != pos()) {
        move(p);
        savePosition();
    }
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
    m_Settings.setValue("Dock", m_dock);
}

void fuzzyClockWindow::restorePosition()
{
    QPoint p(m_Settings.value("Left", 300).toInt(), m_Settings.value("Top", 300).toInt());
    // если монитор, на котором было окно, отключён, окно переносится на основной экран
    if (!QGuiApplication::screenAt(p))
        p = QGuiApplication::primaryScreen()->availableGeometry().center();
    move(p);
    m_dock = m_Settings.value("Dock", 0).toInt();   // применяется после первого расчёта размера в updateTime()
}

void fuzzyClockWindow::createMenu()
{
    m_menu = new QMenu(this);

    m_autostartAct = m_menu->addAction(tr("Добавить в автозагрузку"));
    m_autostartAct->setCheckable(true);
    connect(m_autostartAct, &QAction::triggered, this, &fuzzyClockWindow::setAutostart);

    m_onTopAct = m_menu->addAction(tr("Всегда наверху"));
    m_onTopAct->setCheckable(true);
    connect(m_onTopAct, &QAction::triggered, this, &fuzzyClockWindow::setOnTop);

    m_menu->addSeparator();
    m_menu->addAction(tr("Шрифт..."), this, &fuzzyClockWindow::chooseFont);
    m_menu->addAction(tr("Цвет фона..."), this, &fuzzyClockWindow::chooseBackgroundColor);
    m_menu->addAction(tr("Цвет рамки..."), this, &fuzzyClockWindow::chooseBorderColor);

    QMenu *opacityMenu = m_menu->addMenu(tr("Прозрачность"));
    m_opacityGroup = new QActionGroup(this);
    for (const Opacity &opacity : Opacities) {
        QAction *act = opacityMenu->addAction(opacity.name);
        act->setCheckable(true);
        act->setData(opacity.value);
        m_opacityGroup->addAction(act);
    }
    connect(m_opacityGroup, &QActionGroup::triggered, this, [this](QAction *act) {
        setWindowOpacity(act->data().toDouble());
        m_Settings.setValue("Opacity", act->data().toDouble());
    });

    QMenu *fuzzinessMenu = m_menu->addMenu(tr("Неточность"));
    m_fuzzinessGroup = new QActionGroup(this);
    for (int i = 0; i < int(std::size(FuzzinessNames)); ++i) {
        QAction *act = fuzzinessMenu->addAction(tr(FuzzinessNames[i]));
        act->setCheckable(true);
        act->setData(i);
        m_fuzzinessGroup->addAction(act);
    }
    connect(m_fuzzinessGroup, &QActionGroup::triggered, this, [this](QAction *act) {
        m_Settings.setValue("Fuzziness", act->data().toInt());
        updateTime();
    });

    QMenu *schemeMenu = m_menu->addMenu(tr("Схемы"));
    m_schemeGroup = new QActionGroup(this);
    for (int i = 0; i < int(std::size(Schemes)); ++i) {
        QAction *act = schemeMenu->addAction(tr(Schemes[i].name));
        act->setCheckable(true);
        act->setData(i);
        m_schemeGroup->addAction(act);
    }
    connect(m_schemeGroup, &QActionGroup::triggered, this, [this](QAction *act) {
        const ColorScheme &scheme = Schemes[act->data().toInt()];
        m_Settings.setValue("BackgroundColor", scheme.background.name());
        m_Settings.setValue("TextColor", scheme.text.name());
        m_Settings.setValue("BorderColor", scheme.border.name());
        applyStyle();
    });

    m_menu->addSeparator();
    m_menu->addAction(tr("О программе..."), this, &fuzzyClockWindow::about);
    m_menu->addAction(tr("О Qt..."), qApp, &QApplication::aboutQt);
    m_menu->addSeparator();
    m_menu->addAction(tr("Выход"), this, &QWidget::close);
}

// галочки отражают текущее состояние, в том числе изменённое вне программы (автозагрузка)
void fuzzyClockWindow::updateMenuChecks()
{
    const QSettings run(RunKey, QSettings::NativeFormat);
    m_autostartAct->setChecked(run.value(RunValue).toString().compare(autostartCommand(), Qt::CaseInsensitive) == 0);

    m_onTopAct->setChecked(windowFlags().testFlag(Qt::WindowStaysOnTopHint));

    for (QAction *act : m_opacityGroup->actions())
        act->setChecked(qFuzzyCompare(act->data().toDouble(), windowOpacity()));

    for (QAction *act : m_fuzzinessGroup->actions())
        act->setChecked(act->data().toInt() == int(fuzziness()));

    // схема отмечена, только если все три цвета совпадают с ней
    const QColor background = storedColor("BackgroundColor", DefaultBackground);
    const QColor text = storedColor("TextColor", DefaultText);
    const QColor border = storedColor("BorderColor", DefaultBorder);
    for (QAction *act : m_schemeGroup->actions()) {
        const ColorScheme &scheme = Schemes[act->data().toInt()];
        act->setChecked(scheme.background == background && scheme.text == text && scheme.border == border);
    }
}

void fuzzyClockWindow::setAutostart(bool on)
{
    QSettings run(RunKey, QSettings::NativeFormat);
    if (on)
        run.setValue(RunValue, autostartCommand());
    else
        run.remove(RunValue);
}

void fuzzyClockWindow::setOnTop(bool on)
{
    setWindowFlag(Qt::WindowStaysOnTopHint, on);
    show();                                         // setWindowFlag() скрывает окно
    m_Settings.setValue("OnTop", on);
}

// стандартное окно выбора шрифта Windows; цвет текста выбирается там же
void fuzzyClockWindow::chooseFont()
{
    const QFont current = m_label->font();          // шрифт уже с учётом CSS

    LOGFONTW lf = {};
    const HDC dc = GetDC(nullptr);
    const double pointSize = current.pointSizeF() > 0 ? current.pointSizeF() : 8;
    lf.lfHeight = -qRound(pointSize * GetDeviceCaps(dc, LOGPIXELSY) / 72.0);
    ReleaseDC(nullptr, dc);
    lf.lfWeight = int(current.weight());
    lf.lfItalic = current.italic();
    lf.lfUnderline = current.underline();
    lf.lfStrikeOut = current.strikeOut();
    lf.lfCharSet = DEFAULT_CHARSET;
    const std::wstring family = current.family().toStdWString();
    wcsncpy_s(lf.lfFaceName, LF_FACESIZE, family.c_str(), _TRUNCATE);

    CHOOSEFONTW cf = {};
    cf.lStructSize = sizeof(cf);
    cf.hwndOwner = (HWND)winId();
    cf.lpLogFont = &lf;
    cf.Flags = CF_INITTOLOGFONTSTRUCT | CF_EFFECTS | CF_SCREENFONTS;
    cf.rgbColors = toColorRef(storedColor("TextColor", DefaultText));
    if (!ChooseFontW(&cf))
        return;

    QFont font(QString::fromWCharArray(lf.lfFaceName));
    font.setPointSizeF(cf.iPointSize / 10.0);       // iPointSize - в десятых долях пункта
    font.setWeight(QFont::Weight(lf.lfWeight ? lf.lfWeight : FW_NORMAL));
    font.setItalic(lf.lfItalic);
    font.setUnderline(lf.lfUnderline);
    font.setStrikeOut(lf.lfStrikeOut);

    m_Settings.setValue("Font", font.toString());
    m_Settings.setValue("TextColor", fromColorRef(cf.rgbColors).name());
    applyStyle();
}

void fuzzyClockWindow::chooseBackgroundColor()
{
    QColor color = storedColor("BackgroundColor", DefaultBackground);
    if (!chooseColor((HWND)winId(), color))
        return;
    m_Settings.setValue("BackgroundColor", color.name());
    applyStyle();
}

void fuzzyClockWindow::chooseBorderColor()
{
    QColor color = storedColor("BorderColor", DefaultBorder);
    if (!chooseColor((HWND)winId(), color))
        return;
    m_Settings.setValue("BorderColor", color.name());
    applyStyle();
}

void fuzzyClockWindow::about()
{
    QMessageBox::about(this, tr("О \"Неточных\" часах..."),
            tr("\"Неточные\" часы v3.1.4,        <br/> (ремейк версии 2.1  "
               "от 12.01.2004)<br/>rasa"));
}
