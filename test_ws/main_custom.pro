TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

DEFINES += FLECS_OS_API_IMPL
DEFINES += __USE_MINGW_ANSI_STDIO=1

QMAKE_CFLAGS += -std=c11
QMAKE_CFLAGS += -Wno-unused-function
QMAKE_CFLAGS += -Wno-unused-parameter
QMAKE_CFLAGS += -Wno-missing-braces
QMAKE_CFLAGS += -Wno-missing-field-initializers

INCLUDEPATH  += include
SOURCES += main_custom.c
SOURCES += ecs_ws.c
SOURCES += flecs.c
SOURCES += src/ws.c
SOURCES += src/base64/base64.c
SOURCES += src/handshake/handshake.c
SOURCES += src/sha1/sha1.c
SOURCES += src/utf8/utf8.c

HEADERS += ecs_ws.h
HEADERS += include/ws.h
HEADERS += flecs.h

LIBS += -lmingw32 -lws2_32 -lpthread
