
QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ChatServer
TEMPLATE = app

SOURCES += main.cpp\
        widget.cpp \
    chatServer.cpp

HEADERS  += widget.h \
    chatServer.h

FORMS    += widget.ui
