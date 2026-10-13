#include <QDir>
#include <QTemporaryDir>
#include <QtTest>

#include "fuzzy_clock.h"
#include "fuzzy_dock.h"
#include "fuzzy_helper.h"

using Fuzziness = fuzzyClock::Fuzziness;

class tst_fuzzyClock : public QObject
{
    Q_OBJECT

    fuzzyClock m_clock;

    static fuzzyClock load(const QString &dir)
    {
        const QString current = QDir::currentPath();
        QDir::setCurrent(dir);                      // fuzzy.conf читается из текущей директории
        fuzzyClock clock;
        fuzzyHelper::instance()->readArrays(clock);
        QDir::setCurrent(current);
        return clock;
    }

private slots:
    void initTestCase()
    {
        m_clock = load(SOURCE_DIR);
        QVERIFY(m_clock.isValid(Fuzziness::Minutes));
        QVERIFY(m_clock.isValid(Fuzziness::DayPart));
        QVERIFY(m_clock.isValid(Fuzziness::WeekPart));
    }

    // "Четверть пятого"
    void minutes_data()
    {
        QTest::addColumn<QTime>("time");
        QTest::addColumn<QString>("text");

        QTest::newRow("полночь")       << QTime(0, 0)   << "Ровно двенадцать";
        QTest::newRow("час ночи")      << QTime(1, 0)   << "Ровно час";
        QTest::newRow("01:02")         << QTime(1, 2)   << "Ровно час";
        QTest::newRow("полдень")       << QTime(12, 0)  << "Ровно двенадцать";
        QTest::newRow("час дня")       << QTime(13, 0)  << "Ровно час";
        QTest::newRow("13:03")         << QTime(13, 3)  << "Пять минут второго";
        QTest::newRow("00:05")         << QTime(0, 5)   << "Пять минут первого";
        QTest::newRow("16:15")         << QTime(16, 15) << "Четверть пятого";
        QTest::newRow("12:30")         << QTime(12, 30) << "Половина первого";
        QTest::newRow("12:40")         << QTime(12, 40) << "Без двадцати час";
        QTest::newRow("13:45")         << QTime(13, 45) << "Без четверти два";
        QTest::newRow("11:57")         << QTime(11, 57) << "Без пяти двенадцать";
        QTest::newRow("23:58")         << QTime(23, 58) << "Почти двенадцать";
    }

    void minutes()
    {
        QFETCH(QTime, time);
        QFETCH(QString, text);
        QCOMPARE(m_clock.text(QDateTime(QDate(2026, 10, 12), time), Fuzziness::Minutes), text);
    }

    // в каждую минуту суток - готовая фраза без незаменённых %0 и %1
    void minutesWholeDay()
    {
        for (int minute = 0; minute < 24 * 60; ++minute) {
            const QTime time = QTime(0, 0).addSecs(minute * 60);
            const QString text = m_clock.text(QDateTime(QDate(2026, 10, 12), time), Fuzziness::Minutes);
            QVERIFY2(!text.isEmpty() && !text.contains('%'), qPrintable(time.toString("HH:mm") + " " + text));
        }
    }

    // "Вечер"
    void dayParts_data()
    {
        QTest::addColumn<int>("hour");
        QTest::addColumn<QString>("text");

        QTest::newRow("0")  << 0  << "Ночь";
        QTest::newRow("4")  << 4  << "Ночь";
        QTest::newRow("5")  << 5  << "Раннее утро";
        QTest::newRow("8")  << 8  << "Утро";
        QTest::newRow("11") << 11 << "Почти полдень";
        QTest::newRow("12") << 12 << "Полдень";
        QTest::newRow("13") << 13 << "День";
        QTest::newRow("18") << 18 << "Вечер";
        QTest::newRow("22") << 22 << "Поздний вечер";
        QTest::newRow("23") << 23 << "Поздний вечер";
    }

    void dayParts()
    {
        QFETCH(int, hour);
        QFETCH(QString, text);
        QCOMPARE(m_clock.text(QDateTime(QDate(2026, 10, 12), QTime(hour, 30)), Fuzziness::DayPart), text);
    }

    // "Середина недели"; 12.10.2026 - понедельник
    void weekParts_data()
    {
        QTest::addColumn<int>("day");
        QTest::addColumn<QString>("text");

        QTest::newRow("пн") << 0 << "Начало недели";
        QTest::newRow("вт") << 1 << "Середина недели";
        QTest::newRow("ср") << 2 << "Середина недели";
        QTest::newRow("чт") << 3 << "Середина недели";
        QTest::newRow("пт") << 4 << "Конец недели";
        QTest::newRow("сб") << 5 << "Выходные";
        QTest::newRow("вс") << 6 << "Выходные";
    }

    void weekParts()
    {
        QFETCH(int, day);
        QFETCH(QString, text);
        QCOMPARE(m_clock.text(QDateTime(QDate(2026, 10, 12).addDays(day), QTime(12, 0)), Fuzziness::WeekPart), text);
    }

    void missingConfig()
    {
        QTemporaryDir empty;
        const fuzzyClock clock = load(empty.path());
        QVERIFY(!clock.isValid(Fuzziness::Minutes));
        QCOMPARE(clock.text(QDateTime::currentDateTime(), Fuzziness::Minutes), QString("Ошибка в fuzzy.conf"));
    }

    void brokenReference()
    {
        fuzzyClock clock = m_clock;
        clock.vectMinuteRefer[5] = int(clock.vectMinutes.size());   // ссылка за пределы списка фраз
        QVERIFY(!clock.isValid(Fuzziness::Minutes));

        clock = m_clock;
        clock.vectHourRefer.pop_back();                             // 23 часа вместо 24
        QVERIFY(!clock.isValid(Fuzziness::DayPart));
        QVERIFY(clock.isValid(Fuzziness::Minutes));
    }

    // Прилипание к краям: рабочая область 0,0 - 1919,1039, окно 200x30
    void snapped_data()
    {
        QTest::addColumn<QPoint>("topLeft");
        QTest::addColumn<QPoint>("expected");

        QTest::newRow("далеко от краёв")    << QPoint(500, 500)   << QPoint(500, 500);
        QTest::newRow("к левому")           << QPoint(10, 500)    << QPoint(0, 500);
        QTest::newRow("к левому снаружи")   << QPoint(-12, 500)   << QPoint(0, 500);
        QTest::newRow("к правому")          << QPoint(1705, 500)  << QPoint(1720, 500);
        QTest::newRow("к верхнему")         << QPoint(500, 15)    << QPoint(500, 0);
        QTest::newRow("к нижнему")          << QPoint(500, 1000)  << QPoint(500, 1010);
        QTest::newRow("в угол")             << QPoint(1710, 1005) << QPoint(1720, 1010);
        QTest::newRow("16 пикселей - мимо") << QPoint(16, 500)    << QPoint(16, 500);
    }

    void snapped()
    {
        QFETCH(QPoint, topLeft);
        QFETCH(QPoint, expected);
        const QRect area(0, 0, 1920, 1040);
        QCOMPARE(fuzzyDock::snapped(QRect(topLeft, QSize(200, 30)), area, 15), expected);
    }

    void edges()
    {
        const QRect area(0, 0, 1920, 1040);
        QCOMPARE(fuzzyDock::edges(QRect(500, 500, 200, 30), area), 0);
        QCOMPARE(fuzzyDock::edges(QRect(0, 500, 200, 30), area), int(fuzzyDock::Left));
        QCOMPARE(fuzzyDock::edges(QRect(1720, 1010, 200, 30), area), fuzzyDock::Right | fuzzyDock::Bottom);
        QCOMPARE(fuzzyDock::edges(QRect(0, 0, 200, 30), area), fuzzyDock::Left | fuzzyDock::Top);
    }

    // окно у правого края стало шире - правый край остаётся у края экрана
    void dockedKeepsRightEdge()
    {
        const QRect area(0, 0, 1920, 1040);
        const QRect wider(1720, 1010, 260, 30);
        QCOMPARE(fuzzyDock::docked(wider, area, fuzzyDock::Right | fuzzyDock::Bottom), QPoint(1660, 1010));
        QCOMPARE(fuzzyDock::docked(wider, area, 0), wider.topLeft());
    }

    // панель задач слева: рабочая область начинается не с нуля
    void dockedShiftedArea()
    {
        const QRect area(60, 0, 1860, 1080);
        QCOMPARE(fuzzyDock::docked(QRect(0, 300, 200, 30), area, fuzzyDock::Left), QPoint(60, 300));
    }
};

QTEST_GUILESS_MAIN(tst_fuzzyClock)
#include "tst_fuzzy_clock.moc"
