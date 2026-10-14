# Тесты логики часов: фразы из fuzzy.conf и прилипание к краям экрана
QT = core testlib
CONFIG += console testcase
TARGET = fuzzy_clock_tests

INCLUDEPATH += ../src
DEFINES += SOURCE_DIR=\\\"$$PWD/..\\\"

HEADERS += \
        ../src/fuzzy_clock.h \
        ../src/fuzzy_dock.h \
        ../src/fuzzy_helper.h

SOURCES += \
        tst_fuzzy_clock.cpp \
        ../src/fuzzy_clock.cpp \
        ../src/fuzzy_dock.cpp \
        ../src/fuzzy_helper.cpp
