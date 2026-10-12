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
    Q_DISABLE_COPY_MOVE(fuzzyClockWindow)       // QWidget не копируется и не перемещается

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
    void updateTime();

    void setAutostart(bool on);
    void setOnTop(bool on);
    void chooseFont();
    void chooseBackgroundColor();
    void chooseBorderColor();

private:
    void createMenu();
    void updateMenuChecks();
    void applyStyle();                          // цвета и шрифт из настроек поверх style.css
    QColor storedColor(const QString &key, const QColor &fallback) const;
    void restorePosition();
    void savePosition();

    fuzzyClock::Fuzziness fuzziness() const;

    fuzzyClock m_clock;
    QLabel *m_label;
    QTimer m_timer;
    QSettings m_Settings;
    QPoint mpos;

    QMenu *m_menu;
    QAction *m_autostartAct;
    QAction *m_onTopAct;
    QActionGroup *m_opacityGroup;
    QActionGroup *m_fuzzinessGroup;
    QActionGroup *m_schemeGroup;
};

#endif // FUZZY_CLOCK_WINDOW_H
