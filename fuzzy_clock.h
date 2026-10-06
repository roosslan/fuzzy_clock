#ifndef FUZZY_CLOCK_H
#define FUZZY_CLOCK_H

#include <vector>
#include <QString>
#include <QTime>

// Преобразование времени в «неточную» фразу. Данные загружаются из fuzzy.conf
struct fuzzyClock
{
    std::vector<QString> vectMinutes;
    std::vector<int> vectMinuteRefer;           // для каждой минуты - номер фразы в vectMinutes
    std::vector<QString> vectNominativeHours;
    std::vector<QString> vectGenitiveHours;

    bool isValid() const;
    QString text(const QTime &time) const;
};

#endif // FUZZY_CLOCK_H
