#include <vector>
#include <QSettings>

#include "fuzzy_clock.h"
#include "fuzzy_helper.h"

QStringList fuzzyHelper::iniValue(const QString &groupName, const QString &keyName, QChar separator)
{
    QSettings settings("fuzzy.conf", QSettings::IniFormat);
    settings.beginGroup(groupName);
    return settings.value(keyName).toString().split(separator, Qt::SkipEmptyParts);
}

void fuzzyHelper::readArrays(fuzzyClock &clock)
{
    const auto toVector = [](const QStringList &list) { return std::vector<QString>(list.begin(), list.end()); };

    clock.vectMinuteRefer.clear();
    for (const QString &item : iniValue("oldstyle", "minuteRefer", ' ')) {
        bool ok = false;
        const int pos = item.toInt(&ok);
        clock.vectMinuteRefer.push_back(ok ? pos : -1);     // -1 не пройдёт проверку fuzzyClock::isValid()
    }

    clock.vectMinutes         = toVector(iniValue("oldstyle", "minutes", '\\'));
    clock.vectNominativeHours = toVector(iniValue("oldstyle", "nominativeHours", '\\'));
    clock.vectGenitiveHours   = toVector(iniValue("oldstyle", "genitiveHours", '\\'));
}

std::shared_ptr<fuzzyHelper> fuzzyHelper::instance()
{
    static std::shared_ptr<fuzzyHelper> fH_inst{new fuzzyHelper};
    return fH_inst;
}
