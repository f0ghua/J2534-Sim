INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/third-party/include
INCLUDEPATH += $$PWD/third-party/concurrentqueue
INCLUDEPATH += $$PWD/third-party/spdlog/include
DEPENDPATH  += $$PWD

DEFINES += J2534_SIM_EXPORTS

SOURCES += \

LIBS += -lws2_32 -lshell32
