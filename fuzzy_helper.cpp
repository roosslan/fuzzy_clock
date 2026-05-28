#include <vector>
#include <QString>
#include <QSettings>

#include <fuzzy_clock.h>
#include <fuzzy_helper.h>

void fuzzyHelper::readArrays()
{
    fuzzyClock::instance().vectMinuteRefer = iniValueToVector<int>("oldstyle", "minuteRefer");

    fuzzyClock::instance().vectMinutes = iniValueToVector<QString>("oldstyle", "minutes");
    fuzzyClock::instance().vectNominativeHours = iniValueToVector<QString>("oldstyle", "nominativeHours");
    fuzzyClock::instance().vectGenitiveHours = iniValueToVector<QString>("oldstyle", "genitiveHours");

//        std::copy(vectMinutes.begin(), vectMinutes.end(), fuzzyMinutes);
//        std::copy(vectMinuteRefer.begin(), vectMinuteRefer.end(), minuteRefer);

}

std::shared_ptr<fuzzyHelper> fuzzyHelper::instance()
{
    static std::shared_ptr<fuzzyHelper> fH_inst{new fuzzyHelper};
    return fH_inst;
}
