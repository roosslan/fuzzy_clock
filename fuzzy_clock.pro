# work-around for QTBUG-13496
CONFIG += no_batch \
          precompile_header

QT +=   core gui \
        widgets \
        core5compat

LIBS += \
        -lwtsapi32 \
        -luser32

INCLUDEPATH += C:\Program Files (x64)\Qt\6.10.0\msvc2022_64\include \
                C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Tools\MSVC\14.44.35207\include

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
