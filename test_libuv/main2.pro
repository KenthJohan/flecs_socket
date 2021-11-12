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
SOURCES += main2.c
SOURCES += flecs.c


LIBS += -lmingw32 -lws2_32 -lpthread -luv
