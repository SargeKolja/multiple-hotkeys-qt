    TEMPLATE = app
    TARGET = MultipleHotkeys_Demo

    QT = core gui

    greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += \
    main.cpp \
    qdemowindow.cpp \
    ../src/multihotkey.cpp

HEADERS += \
    qdemowindow.h \
    ../src/multihotkey.h
