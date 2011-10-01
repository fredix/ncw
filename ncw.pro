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
    stats.cpp \
    stats_load.cpp \
    stats_uptime.cpp \
    stats_memory.cpp \
    stats_network.cpp \
    zeromq.cpp
LIBS += ./externals/mongodb/libmongoclient.a \
    -lboost_system \
    -lboost_filesystem \
    -lboost_thread-mt \
    -lmemcached \
    -lQxtCore \
    -lzmq
#INCLUDEPATH += /usr/local/include
INCLUDEPATH += ./externals/
HEADERS += main.h \
    stats_cpu.h \
    stats_process.h \
    nosql.h \
    stats.h \
    stats_load.h \
    stats_uptime.h \
    stats_memory.h \
    stats_network.h \
    zeromq.h
