# work-around for QTBUG-13496
CONFIG += no_batch

QT +=   core gui \
        widgets

LIBS += \
        -lwtsapi32 \
        -luser32 \
        -lgdi32 \
        -lcomdlg32

RC_ICONS = res/app.ico
ICON = res/app.ico
win32: RC_FILE = res/resource.rc

HEADERS += \
        src/fuzzy_clock.h \
        src/fuzzy_dock.h \
        src/fuzzy_helper.h \
        src/fuzzy_clock_window.h

SOURCES += \
        src/fuzzy_clock.cpp \
        src/fuzzy_dock.cpp \
        src/fuzzy_helper.cpp \
        src/fuzzy_clock_window.cpp \
        src/main.cpp

DISTFILES += \
        fuzzy.conf \
        readme.md \
        style.css \
        res/resource.rc
