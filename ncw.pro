# -------------------------------------------------
# Project created by QtCreator 2010-08-09T12:04:26
# -------------------------------------------------
QT += network \
    xml \
    testlib
QT -= gui
TARGET = ncw
CONFIG += console
CONFIG -= app_bundle
CONFIG += qxt
QXT     += core
TEMPLATE = app
SOURCES += main.cpp \
    stats_cpu.cpp \
    stats_process.cpp \
    nosql.cpp \
    stats_load.cpp \
    stats_uptime.cpp \
    stats_memory.cpp \
    stats_network.cpp \
    zeromq.cpp \
    stats_filesystem.cpp \
    dispatcher.cpp \
    get_payload.cpp \
    process.cpp \
    service.cpp \
    torrent.cpp \
    worker.cpp
LIBS += /usr/local/lib/libmongoclient.a \
        -lboost_system \
        -lboost_filesystem \
        -lboost_thread-mt \
        -lmemcached \
        -lQxtCore \
        -lzmq

INCLUDEPATH += /usr/include/qxt/
INCLUDEPATH += /usr/include/qxt/QxtCore
INCLUDEPATH += ./externals/mongodb-src-r2.0.4
HEADERS += main.h \
    stats_cpu.h \
    stats_process.h \
    nosql.h \
    stats_load.h \
    stats_uptime.h \
    stats_memory.h \
    stats_network.h \
    zeromq.h \
    stats_filesystem.h \
    dispatcher.h \
    get_payload.h \
    process.h \
    service.h \
    torrent.h \
    worker.h
