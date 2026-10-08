#ifndef FUZZY_HELPER_H
#define FUZZY_HELPER_H

#include <memory>
#include <QChar>
#include <QString>
#include <QStringList>

struct fuzzyClock;

struct fuzzyHelper
{
    static std::shared_ptr<fuzzyHelper> instance();

    fuzzyHelper(fuzzyHelper const&)    = delete;
    void operator=(fuzzyHelper const&) = delete;

    void readArrays(fuzzyClock &clock);

private:
    fuzzyHelper(){};

    // значение ключа из fuzzy.conf, разбитое по разделителю
    static QStringList iniValue(const QString &groupName, const QString &keyName, QChar separator);
};

#endif // FUZZY_HELPER_H
