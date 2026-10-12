#include <algorithm>

#include "fuzzy_clock.h"

namespace {

// каждая ссылка должна указывать на существующий элемент списка
bool refersInside(const std::vector<int> &refer, size_t referSize, const std::vector<QString> &items)
{
    return refer.size() == referSize
        && std::all_of(refer.begin(), refer.end(), [&items](int i) { return i >= 0 && i < int(items.size()); });
}

}

bool fuzzyClock::isValid(Fuzziness fuzziness) const
{
    switch (fuzziness) {
    case Fuzziness::DayPart:
        return refersInside(vectHourRefer, 24, vectDayParts);
    case Fuzziness::WeekPart:
        return refersInside(vectDayRefer, 7, vectWeekParts);
    case Fuzziness::Minutes:
        break;
    }
    return vectNominativeHours.size() == 12 && vectGenitiveHours.size() == 12
        && refersInside(vectMinuteRefer, 60, vectMinutes);
}

QString fuzzyClock::text(const QDateTime &dateTime, Fuzziness fuzziness) const
{
    if (!isValid(fuzziness))
        return QStringLiteral("Ошибка в fuzzy.conf");

    const QTime time = dateTime.time();

    if (fuzziness == Fuzziness::DayPart)
        return vectDayParts[vectHourRefer[time.hour()]];
    if (fuzziness == Fuzziness::WeekPart)
        return vectWeekParts[vectDayRefer[dateTime.date().dayOfWeek() - 1]];   // dayOfWeek(): 1 - пн, 7 - вс

    const int minute = time.minute();
    int hour = time.hour() % 12;                    // 0..11

    if (minute <= 2)                                // "Ровно ХХ" - текущий час, а не следующий
        hour = (hour + 11) % 12;                    // 0 -> 11 ("двенадцать"), 1 -> 0 ("час")

/*  if (12 == time.hour() && 0 == minute)
        return "Полдень";
    if (0 == time.hour() && 0 == minute)
        return "Полночь";                           */

    const QString &phrase = vectMinutes[vectMinuteRefer[minute]];
    if (phrase.contains("%0"))                      // именительный падеж: "Без пяти два"
        return phrase.arg(vectNominativeHours[hour]);
    return phrase.arg(vectGenitiveHours[hour]);     // родительный падеж: "Пять минут второго"
}
