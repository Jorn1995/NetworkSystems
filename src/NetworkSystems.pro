#-------------------------------------------------
#
# Project created by QtCreator 2017-04-07T09:11:52
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NetworkSystems
TEMPLATE = app


SOURCES += main.cpp\
        ChatWidget.cpp \
    Protocol/Duplex.cpp

HEADERS  += ChatWidget.h \
    config.h.local \
    config.h \
    config.h.local \
    Protocol/Duplex.h \
    Protocol/Flag.h

OTHER_FILES +=

FORMS    += chatwidget.ui
