TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

DEFINES += __USE_MINGW_ANSI_STDIO=1
DEFINES += LWS_WITH_SYS_SMD
DEFINES += _WIN32_WINNT=0x0601
DEFINES += WINVER=0x0601

QMAKE_CFLAGS += -Wno-unused-function
QMAKE_CFLAGS += -Wno-unused-parameter
QMAKE_CFLAGS += -Wno-missing-braces
QMAKE_CFLAGS += -Wno-missing-field-initializers

SOURCES += main.c
SOURCES += ss-server.c
SOURCES += flecs.c

LIBS += -lmingw32 -lwebsockets
