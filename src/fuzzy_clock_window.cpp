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

/* цвета по умолчанию совпадают со style.css */
const QColor DefaultBackground(Qt::black);
const QColor DefaultText(0x00, 0xff, 0x00);     // lime
const QColor DefaultBorder(0x00, 0xff, 0x00);

struct ColorScheme {
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

struct opacity {
    const char *name;
    double value;
};

// процент - видимость окна: 100% - обычное непрозрачное окно
const opacity Opacities[] = { { "10%", 0.1 }, { "50%", 0.5 }, { "100%", 1.0 } };

const char *const FuzzinessNames[] = { "Четверть пятого", "Вечер", "Середина недели" };

// на каком расстоянии от края экрана окно к нему притягивается
constexpr int SnapDistance = 15;

QString autostart_command() {
    return "\"" + QDir::toNativeSeparators(QCoreApplication::applicationFilePath()) + "\"";
}

COLORREF to_color_ref(const QColor &color) {
    return RGB(color.red(), color.green(), color.blue());
}

QColor from_color_ref(COLORREF color) {
    return QColor(GetRValue(color), GetGValue(color), GetBValue(color));
}

/* стандартное окно выбора цвета Windows */
bool choose_color(HWND owner, QColor &color) {
    static COLORREF customColors[16] = {};      // "Дополнительные цвета" сохраняются до выхода из программы

    CHOOSECOLORW cc = {};
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = owner;
    cc.rgbResult = to_color_ref(color);
    cc.lpCustColors = customColors;
    cc.Flags = CC_RGBINIT | CC_FULLOPEN;
    if (!ChooseColorW(&cc))
        return false;
    color = from_color_ref(cc.rgbResult);
    return true;
}

}

fuzzyClockWindow::fuzzyClockWindow()
    : m_label_(new QLabel(this))
    , m_settings_("HKEY_CURRENT_USER\\SOFTWARE\\rasa\\FuzzyClock", QSettings::NativeFormat)
{
    // флаги задаются до создания нативного окна (winId)
    Qt::WindowFlags flags = Qt::SubWindow | Qt::Window | Qt::FramelessWindowHint;
    if (m_settings_.value("OnTop", true).toBool())
        flags |= Qt::WindowStaysOnTopHint;
    setWindowFlags(flags);
    setWindowTitle(tr("Неточные часы"));

    /* for CSS */
    setObjectName("fuzzyClockWindow");
    setAttribute(Qt::WA_StyledBackground);          /* без этого рамка из CSS у наследника QWidget не рисуется */
    m_label_->setObjectName("timeLabel");
    m_label_->move(5, 5);

    fuzzyHelper::instance()->readArrays(m_fuzzy_clock_);  // конфиг читается один раз при старте

    WTSRegisterSessionNotification((HWND)winId(), NOTIFY_FOR_THIS_SESSION);

    m_timer.setSingleShot(true);
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, &fuzzyClockWindow::update_time);

    restorePosition();
    setWindowOpacity(m_settings_.value("Opacity", 1.0).toDouble());

    // панель задач переместилась, сменилось разрешение - окно остаётся у своих краёв
    const auto watchScreen = [this](QScreen *screen) {
        connect(screen, &QScreen::availableGeometryChanged, this, &fuzzyClockWindow::apply_dock);
    };
    for (QScreen *screen : QGuiApplication::screens())
        watchScreen(screen);
    connect(qApp, &QGuiApplication::screenAdded, this, watchScreen);
    connect(qApp, &QGuiApplication::screenRemoved, this, &fuzzyClockWindow::apply_dock, Qt::QueuedConnection);

    /* Context menu */
    create_menu();

    apply_style();                                   // заодно выводит время
}

fuzzyClockWindow::~fuzzyClockWindow() {
    // internalWinId() не создаёт окно заново, в отличие от winId()
    if (WId id = internalWinId())
        WTSUnRegisterSessionNotification((HWND)id);
}

fuzzyClock::Fuzziness fuzzyClockWindow::fuzziness() const
{
    const int mode = m_settings_.value("Fuzziness", 0).toInt();
    if (mode < 0 || mode >= int(std::size(FuzzinessNames)))
        return fuzzyClock::Fuzziness::Minutes;
    return fuzzyClock::Fuzziness(mode);
}

void fuzzyClockWindow::update_time() {
    const QDateTime now = QDateTime::currentDateTime();
    m_label_->setText(m_fuzzy_clock_.text(now, fuzziness()));
    /* точное время и дата - в подсказке */
    setToolTip(QLocale(QLocale::Russian).toString(now, "HH:mm, dddd, d MMMM"));
    m_label_->adjustSize();                          /* учитывает шрифт из CSS (вызывает ensurePolished) */
    setFixedSize(m_label_->size() + QSize(10, 10));
    apply_dock();                                    // при прилипании справа или снизу окно растёт внутрь экрана

    /* следующее обновление - в начале следующей минуты */
    m_timer.start(60000 - now.time().msecsSinceStartOfDay() % 60000);
}

QColor fuzzyClockWindow::stored_color(const QString &key, const QColor &fallback) const
{
    const QColor color(m_settings_.value(key).toString());
    return color.isValid() ? color : fallback;
}

void fuzzyClockWindow::apply_style() {
    QStringList window, label;

    const QString background = m_settings_.value("BackgroundColor").toString();
    if (!background.isEmpty())
        window << "background-color: " + background;
    const QString border = m_settings_.value("BorderColor").toString();
    if (!border.isEmpty())
        window << "border: 1px solid " + border;
    const QString text = m_settings_.value("TextColor").toString();
    if (!text.isEmpty())
        label << "color: " + text;

    QFont font;
    const QString fontString = m_settings_.value("Font").toString();
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

    /* стиль окна имеет приоритет над style.css, не заданные здесь свойства берутся из style.css */
    setStyleSheet(QString("QWidget#fuzzyClockWindow { %1 } QWidget#timeLabel { %2 }")
                  .arg(window.join("; "), label.join("; ")));
    update_time();                                   // размер окна зависит от шрифта
}

void fuzzyClockWindow::contextMenuEvent(QContextMenuEvent *event) {
    update_menu_checks();
    m_menu->exec(event->globalPos());
}

void fuzzyClockWindow::mousePressEvent(QMouseEvent *event) {
    m_pos_ = event->pos();
}

/* Drag window without title; Shift отключает прилипание к краям */
void fuzzyClockWindow::mouseMoveEvent(QMouseEvent *event) {
    if (!(event->buttons() & Qt::LeftButton))
        return;
    const QPoint p = pos() + event->pos() - m_pos_;
    if (event->modifiers() & Qt::ShiftModifier)
        move(p);
    else
        move(fuzzyDock::snapped(QRect(p, size()), dock_area(p), SnapDistance));
}

void fuzzyClockWindow::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dock = fuzzyDock::edges(geometry(), dock_area(pos()));
        save_position();                             // перетаскивание закончено
    }
}

QRect fuzzyClockWindow::dock_area(const QPoint &topLeft) const
{
    const QScreen *screen = QGuiApplication::screenAt(QRect(topLeft, size()).center());
    if (!screen)
        screen = QGuiApplication::screenAt(pos());
    if (!screen)
        screen = QGuiApplication::primaryScreen();
    return screen->availableGeometry();             /* без панели задач */
}

void fuzzyClockWindow::apply_dock() {
    if (!m_dock)
        return;
    const QPoint p = fuzzyDock::docked(geometry(), dock_area(pos()), m_dock);
    if (p != pos()) {
        move(p);
        save_position();
    }
}

void fuzzyClockWindow::closeEvent(QCloseEvent *event) {
    save_position();                                 /* и через меню, и по Alt+F4 */
    QWidget::closeEvent(event);
    QApplication::quit();
}

bool fuzzyClockWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result) {
    if (eventType == "windows_generic_MSG") {
        const MSG *msg = static_cast<const MSG *>(message);
        // после разблокировки (и выхода из сна) сразу обновить время и перезапустить таймер
        if (msg->message == WM_WTSSESSION_CHANGE && msg->wParam == WTS_SESSION_UNLOCK)
            update_time();
    }
    return QWidget::nativeEvent(eventType, message, result);
}

void fuzzyClockWindow::save_position() {
    m_settings_.setValue("currentPath", QDir::currentPath());
    m_settings_.setValue("Top", pos().y());
    m_settings_.setValue("Left", pos().x());
    m_settings_.setValue("Dock", m_dock);
}

void fuzzyClockWindow::restorePosition()
{
    QPoint p(m_settings_.value("Left", 300).toInt(), m_settings_.value("Top", 300).toInt());
    /* если монитор, на котором было окно, отключён, окно переносится на основной экран */
    if (!QGuiApplication::screenAt(p))
        p = QGuiApplication::primaryScreen()->availableGeometry().center();
    move(p);
    m_dock = m_settings_.value("Dock", 0).toInt();   /* применяется после первого расчёта размера в update_tTime() */
}

void fuzzyClockWindow::create_menu() {
    m_menu = new QMenu(this);

    m_act_copy_to_clipboard_ = m_menu->addAction(tr("Скопировать"));
    connect(m_act_copy_to_clipboard_, &QAction::triggered, this, &fuzzyClockWindow::copy_timetext_to_clipboard);

    m_act_autostart_ = m_menu->addAction(tr("Добавить в автозагрузку"));
    m_act_autostart_->setCheckable(true);
    connect(m_act_autostart_, &QAction::triggered, this, &fuzzyClockWindow::set_autostart);

    m_act_on_top_ = m_menu->addAction(tr("Всегда наверху"));
    m_act_on_top_->setCheckable(true);
    connect(m_act_on_top_, &QAction::triggered, this, &fuzzyClockWindow::set_on_top);

    m_menu->addSeparator();
    m_menu->addAction(tr("Шрифт..."), this, &fuzzyClockWindow::choose_font);
    m_menu->addAction(tr("Цвет фона..."), this, &fuzzyClockWindow::choose_background_color);
    m_menu->addAction(tr("Цвет рамки..."), this, &fuzzyClockWindow::choose_border_color);

    QMenu *opacityMenu = m_menu->addMenu(tr("Прозрачность"));
    m_act_opacity_group_ = new QActionGroup(this);
    for (const opacity &opacity : Opacities) {
        QAction *act = opacityMenu->addAction(opacity.name);
        act->setCheckable(true);
        act->setData(opacity.value);
        m_act_opacity_group_->addAction(act);
    }
    connect(m_act_opacity_group_, &QActionGroup::triggered, this, [this](QAction *act) {
        setWindowOpacity(act->data().toDouble());
        m_settings_.setValue("Opacity", act->data().toDouble());
    });

    QMenu *fuzzinessMenu = m_menu->addMenu(tr("Неточность"));
    m_act_fuzziness_group_ = new QActionGroup(this);
    for (int i = 0; i < int(std::size(FuzzinessNames)); ++i) {
        QAction *act = fuzzinessMenu->addAction(tr(FuzzinessNames[i]));
        act->setCheckable(true);
        act->setData(i);
        m_act_fuzziness_group_->addAction(act);
    }
    connect(m_act_fuzziness_group_, &QActionGroup::triggered, this, [this](QAction *act) {
        m_settings_.setValue("Fuzziness", act->data().toInt());
        update_time();
    });

    QMenu *schemeMenu = m_menu->addMenu(tr("Схемы"));
    m_act_scheme_group_ = new QActionGroup(this);
    for (int i = 0; i < int(std::size(Schemes)); ++i) {
        QAction *act = schemeMenu->addAction(tr(Schemes[i].name));
        act->setCheckable(true);
        act->setData(i);
        m_act_scheme_group_->addAction(act);
    }
    connect(m_act_scheme_group_, &QActionGroup::triggered, this, [this](QAction *act) {
        const ColorScheme &scheme = Schemes[act->data().toInt()];
        m_settings_.setValue("BackgroundColor", scheme.background.name());
        m_settings_.setValue("TextColor", scheme.text.name());
        m_settings_.setValue("BorderColor", scheme.border.name());
        apply_style();
    });

    m_menu->addSeparator();
    m_menu->addAction(tr("О »Неточных часах«..."), this, &fuzzyClockWindow::about);
    m_menu->addAction(tr("О Qt..."), qApp, &QApplication::aboutQt);
    m_menu->addSeparator();
    m_menu->addAction(tr("Выход"), this, &QWidget::close);
}

/* галочки отражают текущее состояние, в том числе изменённое вне программы (автозагрузка) */
void fuzzyClockWindow::update_menu_checks()
{
    const QSettings run(RunKey, QSettings::NativeFormat);
    m_act_autostart_->setChecked(run.value(RunValue).toString().compare(autostart_command(), Qt::CaseInsensitive) == 0);

    m_act_on_top_->setChecked(windowFlags().testFlag(Qt::WindowStaysOnTopHint));

    for (QAction *act : m_act_opacity_group_->actions())
        act->setChecked(qFuzzyCompare(act->data().toDouble(), windowOpacity()));

    for (QAction *act : m_act_fuzziness_group_->actions())
        act->setChecked(act->data().toInt() == int(fuzziness()));

    // схема отмечена, только если все три цвета совпадают с ней
    const QColor background = stored_color("BackgroundColor", DefaultBackground);
    const QColor text = stored_color("TextColor", DefaultText);
    const QColor border = stored_color("BorderColor", DefaultBorder);
    for (QAction *act : m_act_scheme_group_->actions()) {
        const ColorScheme &scheme = Schemes[act->data().toInt()];
        act->setChecked(scheme.background == background && scheme.text == text && scheme.border == border);
    }
}

void fuzzyClockWindow::copy_timetext_to_clipboard() {
    QApplication::clipboard()->setText(m_fuzzy_clock_.text(QDateTime::currentDateTime(), fuzziness()));
}

void fuzzyClockWindow::set_autostart(bool on) {
    QSettings run(RunKey, QSettings::NativeFormat);
    if (on)
        run.setValue(RunValue, autostart_command());
    else
        run.remove(RunValue);
}

void fuzzyClockWindow::set_on_top(bool on) {
    setWindowFlag(Qt::WindowStaysOnTopHint, on);
    show();                                         /* setWindowFlag() скрывает окно */
    m_settings_.setValue("OnTop", on);
}

// стандартное окно выбора шрифта Windows; цвет текста выбирается там же
void fuzzyClockWindow::choose_font() {
    const QFont current = m_label_->font();          // шрифт уже с учётом CSS

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
    cf.rgbColors = to_color_ref(stored_color("TextColor", DefaultText));
    if (!ChooseFontW(&cf))
        return;

    QFont font(QString::fromWCharArray(lf.lfFaceName));
    font.setPointSizeF(cf.iPointSize / 10.0);       // iPointSize - в десятых долях пункта
    font.setWeight(QFont::Weight(lf.lfWeight ? lf.lfWeight : FW_NORMAL));
    font.setItalic(lf.lfItalic);
    font.setUnderline(lf.lfUnderline);
    font.setStrikeOut(lf.lfStrikeOut);

    m_settings_.setValue("Font", font.toString());
    m_settings_.setValue("TextColor", from_color_ref(cf.rgbColors).name());
    apply_style();
}

void fuzzyClockWindow::choose_background_color() {
    QColor color = stored_color("BackgroundColor", DefaultBackground);
    if (!choose_color((HWND)winId(), color))
        return;
    m_settings_.setValue("BackgroundColor", color.name());
    apply_style();
}

void fuzzyClockWindow::choose_border_color() {
    QColor color = stored_color("BorderColor", DefaultBorder);
    if (!choose_color((HWND)winId(), color))
        return;
    m_settings_.setValue("BorderColor", color.name());
    apply_style();
}

void fuzzyClockWindow::about() {
    QMessageBox::about(this, tr("О \"Неточных\" часах..."),
            tr("\"Неточные\" часы v3.1.4,        <br/> (ремейк версии 2.1  "
               "от 12.01.2004)<br/>rasa"));
}
