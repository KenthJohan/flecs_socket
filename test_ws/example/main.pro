TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

DEFINES += __USE_MINGW_ANSI_STDIO=1

QMAKE_CFLAGS += -Wno-unused-function
QMAKE_CFLAGS += -Wno-unused-parameter
QMAKE_CFLAGS += -Wno-missing-braces
QMAKE_CFLAGS += -Wno-missing-field-initializers

INCLUDEPATH  += ../include
SOURCES += send_receive.c
SOURCES += ../src/ws.c
SOURCES += ../src/base64/base64.c
SOURCES += ../src/handshake/handshake.c
SOURCES += ../src/sha1/sha1.c
SOURCES += ../src/utf8/utf8.c

LIBS += -lmingw32 -lws2_32 -lpthread
