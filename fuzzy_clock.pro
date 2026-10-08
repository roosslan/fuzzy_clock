# work-around for QTBUG-13496
CONFIG += no_batch

QT +=   core gui \
        widgets

LIBS += \
        -lwtsapi32 \
        -luser32

RC_ICONS = app.ico
ICON = app.ico
win32: RC_FILE = resource.rc

HEADERS += \
        fuzzy_clock.h \
        fuzzy_helper.h \
        fuzzy_clock_window.h

SOURCES += \
        fuzzy_clock.cpp \
        fuzzy_helper.cpp \
        fuzzy_clock_window.cpp \
        main.cpp

DISTFILES += \
        fuzzy.conf \
        readme.md \
        style.css \
        resource.rc
