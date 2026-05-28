#include "fuzzy_clock.h"

#include <QtGui>
#include <QtWidgets/QApplication>
#include <Windows.h>
#include <WtsApi32.h>
#include <QDebug>
#include <QLabel>


constexpr int ID_TIMER = 100;

class fuzzyClockWindow : public QWidget
{
private:
//  fuzzyClock fuzzyClock;

    QAction *aboutAct;
    QAction *aboutQtAct;
    QAction *exitAct;
    QSettings m_Settings;

    void createActions();

private slots:
    void about();
    void exit();

    void mouseMoveEvent(QMouseEvent *event) override;
    virtual bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) ;
    void contextMenuEvent(QContextMenuEvent *event) override;

public:
    QPoint mpos;
    fuzzyClockWindow();

    ~fuzzyClockWindow();

    /* const A ca;          // A()
       A b(ca)              // A(const A&)
       A c = ca;            // A(const A&)  */
    fuzzyClockWindow(const fuzzyClockWindow &other);

    fuzzyClockWindow& operator=(const fuzzyClockWindow &other);

    fuzzyClockWindow(fuzzyClockWindow&& other);    
    fuzzyClockWindow &operator=(fuzzyClockWindow&& other);

/*  int DisplayTime(int a,int b);       */
    /* could be replaced by rxQt */
    void mousePressEvent(QMouseEvent *event);

};
