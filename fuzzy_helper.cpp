#include <vector>
#include <QString>
#include <QSettings>

#include "fuzzy_clock.h"
#include "fuzzy_helper.h"

void fuzzyHelper::readArrays(fuzzyClock &clock)
{
    clock.vectMinuteRefer     = iniValueToVector<int>("oldstyle", "minuteRefer");
    clock.vectMinutes         = iniValueToVector<QString>("oldstyle", "minutes");
    clock.vectNominativeHours = iniValueToVector<QString>("oldstyle", "nominativeHours");
    clock.vectGenitiveHours   = iniValueToVector<QString>("oldstyle", "genitiveHours");
}

std::shared_ptr<fuzzyHelper> fuzzyHelper::instance()
{
    static std::shared_ptr<fuzzyHelper> fH_inst{new fuzzyHelper};
    return fH_inst;
}
