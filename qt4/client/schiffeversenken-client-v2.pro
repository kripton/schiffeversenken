#-------------------------------------------------
#
# Project created by QtCreator 2012-05-31T16:40:14
#
#-------------------------------------------------

QT       += core network gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = schiffeversenken-client-v2
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

# make debugging actually ork >_<:
LIBS     += -g

OTHER_FILES += \
    protokoll.txt
