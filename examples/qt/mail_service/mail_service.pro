#-------------------------------------------------
#
# Project created by QtCreator 2013-02-21T16:20:35
#
#-------------------------------------------------

QT       += core
QT       -= gui
CONFIG += qxt
QXT     += core
QXT     += network


TARGET = mail_service
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    email.cpp

HEADERS += \
    main.h \
    email.h



LIBS += -lQxtCore

INCLUDEPATH += /usr/local/Qxt/include/
INCLUDEPATH += /usr/local/Qxt/include/QxtCore/
