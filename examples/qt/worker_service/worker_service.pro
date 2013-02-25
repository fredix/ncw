#-------------------------------------------------
#
# Project created by QtCreator 2013-01-18T17:32:21
#
#-------------------------------------------------

QT       += core
QT       -= gui

CONFIG += qxt
QXT     += core
QXT     += network

TARGET = worker_service
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp

HEADERS += \
    main.h

LIBS += -lQxtCore

INCLUDEPATH += /usr/local/Qxt/include/
INCLUDEPATH += /usr/local/Qxt/include/QxtCore/
