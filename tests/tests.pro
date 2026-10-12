# Тесты логики часов: фразы из fuzzy.conf и прилипание к краям экрана
QT = core testlib
CONFIG += console testcase
TARGET = fuzzy_clock_tests

INCLUDEPATH += ..
DEFINES += SOURCE_DIR=\\\"$$PWD/..\\\"

HEADERS += \
        ../fuzzy_clock.h \
        ../fuzzy_dock.h \
        ../fuzzy_helper.h

SOURCES += \
        tst_fuzzy_clock.cpp \
        ../fuzzy_clock.cpp \
        ../fuzzy_dock.cpp \
        ../fuzzy_helper.cpp
