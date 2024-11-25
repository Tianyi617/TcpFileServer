#-------------------------------------------------
#
# Project created by QtCreator 2016-11-19T12:21:11
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TcpFileServer
TEMPLATE = app


HEADERS += tcpfilesender.h \
           mainwindow.h \
           tcpfileserver.h \

SOURCES += tcpfilesender.cpp \
           mainwindow.cpp \
           tcpfileserver.cpp \
           main.cpp

