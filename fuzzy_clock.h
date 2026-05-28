#ifndef FUZZY_СLOCK_H
#define FUZZY_СLOCK_H

#include <vector>
#include <QtGui>
#include <QtWidgets/QApplication>
#include <Windows.h>
#include <WtsApi32.h>
#include <QDebug>
#include <QLabel>
#include <QString>

//#include "fuzzyHelper.h"

struct fuzzyClock : public QWidget
{
private:
    Q_OBJECT // macro to declare own signals and slots

    QWidget *m_window;
    QLabel *m_label = nullptr;
    int hourToUse;
    int referMinPos;
    QString timeToShow;
    int fHeight;                // font height
    int fWidth;                 // font width
    QSettings m_sSettingsFile;

    fuzzyClock(){};

public:

    static fuzzyClock& instance()    // Singleton
            {
                static fuzzyClock Instance;
                return Instance;
            }

    fuzzyClock(fuzzyClock const&)     = delete;
    void operator=(fuzzyClock const&) = delete;

    std::vector<QString> vectMinutes;
    std::vector<int> vectMinuteRefer;
    std::vector<QString> vectNominativeHours;
    std::vector<QString> vectGenitiveHours;

    void SetLabel(QLabel *timeLabel);           // pass QLabel from parent fuzzyWindow to fuzzyClock class
    void SetWindow(QWidget *pWindow);           // pass fuzzyWindow to fuzzyClock class

    void DisplayTime();
    int getHourNow(int h12);
    int getMinuteNow();

    // Depending on the minute - position of displayed text in the next arrays:
    // int minuteRefer[60] {};
    /* replaced by vectors  */
};

#endif // FUZZY_СLOCK_H
