#include <algorithm>

#include "fuzzy_clock.h"

bool fuzzyClock::isValid() const
{
    if (vectMinuteRefer.size() != 60 || vectNominativeHours.size() != 12 || vectGenitiveHours.size() != 12)
        return false;
    // каждая ссылка из minuteRefer должна указывать на существующую фразу
    return std::all_of(vectMinuteRefer.begin(), vectMinuteRefer.end(),
                       [this](int i) { return i >= 0 && i < int(vectMinutes.size()); });
}

QString fuzzyClock::text(const QTime &time) const
{
    if (!isValid())
        return QStringLiteral("Ошибка в fuzzy.conf");

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
