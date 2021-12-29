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
SOURCES += eg_socket_mingw32.c
SOURCES += eg_geometry.c
SOURCES += eg_basic.c
SOURCES += eg_net.c
SOURCES += eg_log.c
SOURCES += eg_thread.c

HEADERS += eg_socket.h
HEADERS += eg_geometry.h
HEADERS += eg_basic.h
HEADERS += eg_net.h
HEADERS += eg_log.h
HEADERS += eg_thread.h


LIBS += -lmingw32 -lws2_32
