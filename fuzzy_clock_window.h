#ifndef FUZZY_CLOCK_WINDOW_H
#define FUZZY_CLOCK_WINDOW_H

#include <QWidget>
#include <QSettings>
#include <QTimer>

#include "fuzzy_clock.h"

class QLabel;
class QAction;

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

private:
    void createActions();
    void restorePosition();
    void savePosition();

    fuzzyClock m_clock;
    QLabel *m_label;
    QTimer m_timer;
    QSettings m_Settings;
    QPoint mpos;

    QAction *aboutAct;
    QAction *aboutQtAct;
    QAction *exitAct;
};

#endif // FUZZY_CLOCK_WINDOW_H
