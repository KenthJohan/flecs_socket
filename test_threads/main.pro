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
SOURCES += eg_cklog.c
SOURCES += ck/src/ck_array.c
SOURCES += ck/src/ck_barrier_centralized.c

INCLUDEPATH += ck/include

HEADERS += ck/include/**
HEADERS += eg_cklog.h


LIBS += -lmingw32 -lws2_32
