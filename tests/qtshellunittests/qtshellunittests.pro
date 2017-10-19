QT       += testlib qml

TARGET = unittests
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += \
    main.cpp \
    qtshelltests.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"

include(vendor/vendor.pri)
include(../../qtshell.pri)

DISTFILES += \
    qpm.json \
    ../../README.md \
    ../../qpm.json \
    ../../appveyor.yml \
    ../../.travis.yml


HEADERS += \
    qtshelltests.h

RESOURCES += \
    resource.qrc

