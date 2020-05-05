TEMPLATE = app
TARGET = MultipleHotkeys_Demo

QT = core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS += -fmax-errors=5  # stop after 5 errors

SOURCES += \
    main.cpp \
    qdemowindow.cpp \
    ../src/multihotkey.cpp

HEADERS += \
    qdemowindow.h \
    ../src/multihotkey.h
