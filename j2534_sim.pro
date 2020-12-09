#-------------------------------------------------
#
# Project created by QtCreator 2019-06-30T14:38:56
#
#-------------------------------------------------

#QT       -= core gui

CONFIG += c++11
CONFIG -= qt

TARGET = j2534_sim
TEMPLATE = lib

DEFINES += APP_LIBRARY

DEFINES += QT_DEPRECATED_WARNINGS
gcc {
QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter -Wno-unused-variable
QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-function
}
msvc {
DEFINES += _WINSOCK_DEPRECATED_NO_WARNINGS
QMAKE_CXXFLAGS += /D "_MBCS" /Zi
QMAKE_LFLAGS += /DEBUG /OPT:REF
}

unix {
    target.path = /usr/lib
    INSTALLS += target
}

DISTFILES += \
    app_version.h.in
QMAKE_SUBSTITUTES += $$PWD/app_version.h.in

TOPSRCDIR   = $$PWD
TOPBUILDDIR = $$shadowed($$PWD)

INCLUDEPATH += $$TOPBUILDDIR

include(j2534_sim.pri)

HEADERS += \
    J2534_v0404.h \
    StdAfx.h \
    applib_global.h \
    j2534_sim.h \
    log.h

SOURCES += \
    StdAfx.cpp \
    j2534_sim.cpp
