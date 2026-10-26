#ifndef FUZZY_CLOCK_WINDOW_H
#define FUZZY_CLOCK_WINDOW_H

#include <QColor>
#include <QWidget>
#include <QSettings>
#include <QTimer>

#include "fuzzy_clock.h"

class QLabel;
class QAction;
class QActionGroup;
class QMenu;

class fuzzyClockWindow : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(fuzzyClockWindow)       /* QWidget не копируется и не перемещается */

    void create_menu();
    void update_menu_checks();
    void apply_style();                          /* цвета и шрифт из настроек поверх style.css */
    QColor stored_color(const QString &key, const QColor &fallback) const;
    void restorePosition();
    void save_position();

    /* Прилипание к краям экрана (расчёты - в fuzzy_dock.h) */
    QRect dock_area(const QPoint &topLeft) const;    /* рабочая область экрана, на котором окажется окно */
    void apply_dock();                               /* удержать прижатые края при смене размера окна или экрана */

    fuzzyClock::Fuzziness fuzziness() const;

    fuzzyClock m_fuzzy_clock_;
    QLabel *m_label_;

    QTimer m_timer;
    QSettings m_settings_;
    QPoint m_pos_;
    int m_dock = 0;                                 /* сочетание fuzzyDock::Edge */

    QMenu *m_menu;
    QAction *m_act_copy_to_clipboard_;
    QAction *m_act_autostart_;
    QAction *m_act_on_top_;
    QActionGroup *m_act_opacity_group_;
    QActionGroup *m_act_fuzziness_group_;
    QActionGroup *m_act_scheme_group_;

public:
    fuzzyClockWindow();
    ~fuzzyClockWindow() override;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;

private slots:
    void about();
    void update_time();
    void copy_timetext_to_clipboard();
    void set_autostart(bool on);
    void set_on_top(bool on);
    void choose_font();
    void choose_background_color();
    void choose_border_color();
};

#endif // FUZZY_CLOCK_WINDOW_H
