#ifndef FUZZY_CLOCK_H
#define FUZZY_CLOCK_H

#include <vector>
#include <QDateTime>
#include <QString>

// Преобразование времени в «неточную» фразу. Данные загружаются из fuzzy.conf
struct fuzzyClock
{
    enum class Fuzziness {
        Minutes,        // "Четверть пятого" - с точностью до пяти минут, секция [oldstyle]
        DayPart,        // "Вечер" - часть суток, секция [dayparts]
        WeekPart        // "Середина недели" - часть недели, секция [weekparts]
    };

    std::vector<QString> vectMinutes;
    std::vector<int> vectMinuteRefer;           // для каждой минуты - номер фразы в vectMinutes
    std::vector<QString> vectNominativeHours;
    std::vector<QString> vectGenitiveHours;

    std::vector<QString> vectDayParts;
    std::vector<int> vectHourRefer;             // для каждого часа (0..23) - номер в vectDayParts
    std::vector<QString> vectWeekParts;
    std::vector<int> vectDayRefer;              // для каждого дня недели (пн..вс) - номер в vectWeekParts

    bool isValid(Fuzziness fuzziness) const;
    QString text(const QDateTime &dateTime, Fuzziness fuzziness) const;
};

#endif // FUZZY_CLOCK_H
