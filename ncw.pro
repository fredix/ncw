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
    zeromq.cpp \
    process.cpp \
    service.cpp \
    worker.cpp
LIBS += ./externals/mongo-cxx-driver/libmongoclient.a \
        -lboost_system \
        -lboost_filesystem \
        -lboost_thread-mt \
        -lQxtCore \
        -lzmq

INCLUDEPATH += ./externals/cppzmq/
INCLUDEPATH += ./externals/libqxt/
INCLUDEPATH += ./externals/libqxt/core
INCLUDEPATH += ./externals/mongo-cxx-driver/src

HEADERS += main.h \
    zeromq.h \
    process.h \
    service.h \
    worker.h \
    ncw_global.h
