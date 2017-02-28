INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

CONFIG += c++11

HEADERS += \
    $$PWD/qtshell.h \
    $$PWD/QtShell \
    $$PWD/priv/qtshellpriv.h

SOURCES += \
    $$PWD/qtshell.cpp \
    $$PWD/priv/qtshellpriv.cpp
