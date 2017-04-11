#-------------------------------------------------
#
# Project created by QtCreator 2017-04-07T09:11:52
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NetworkSystems
TEMPLATE = app
CONFIG += c++11


SOURCES += main.cpp\
        ChatWidget.cpp \
    Protocol/NetworkLayer/Router.cpp \
    Protocol/TransportLayer/ReliableLink.cpp \
    Protocol/NetworkLayer/HigherProtocolInterface.cpp \
    Protocol/NetworkLayer/IpHeader.cpp \
    Protocol/ApplicationLayer/ChatLink.cpp \
    Protocol/TransportLayer/Flag.cpp

HEADERS  += ChatWidget.h \
    config.h.local \
    config.h \
    config.h.local \
    Protocol/TransportLayer/Flag.h \
    Protocol/TransportLayer/ReliableLink.h \
    Protocol/NetworkLayer/HigherProtocolInterface.h \
    Protocol/NetworkLayer/Router.h \
    Protocol/NetworkLayer/IpHeader.h \
    Protocol/ApplicationLayer/ChatLink.h

OTHER_FILES +=

FORMS    += chatwidget.ui
