QT       += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
INCLUDEPATH += .

SOURCES += \
    main.cpp \
    MainWindow.cpp \
    TradingBottomPanel.cpp \
    orderbook.cpp

HEADERS += \
    MainWindow.h \
    TradingBottomPanel.h \
    orderbook.h
