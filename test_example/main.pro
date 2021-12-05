TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

DEFINES += __USE_MINGW_ANSI_STDIO=1

QMAKE_CFLAGS += -Wno-unused-function
QMAKE_CFLAGS += -Wno-unused-parameter
QMAKE_CFLAGS += -Wno-missing-braces
QMAKE_CFLAGS += -Wno-missing-field-initializers

SOURCES += main.c
SOURCES += flecs.c
SOURCES += flecs_socket_mingw32.c
SOURCES += flecs_geometry.c

HEADERS += flecs_socket.h
HEADERS += flecs_geometry.h

LIBS += -lmingw32 -lws2_32
