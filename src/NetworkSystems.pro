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
    Protocol/Sender.cpp \
    Protocol/Receiver.cpp

HEADERS  += ChatWidget.h \
    Protocol/Sender.h \
    Protocol/Receiver.h \
    config.h.local \
    config.h

OTHER_FILES += config.h.local.dist

FORMS    += chatwidget.ui
