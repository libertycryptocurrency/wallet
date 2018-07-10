#-------------------------------------------------
#
# Project created by QtCreator 2018-04-15T18:39:54
#
#-------------------------------------------------
# Qt 4.7.0
# Elementary can need "sudo apt install libqtwebkit-dev"

QT       += core gui webkit


TARGET = LibertyWallet
TEMPLATE = app


SOURCES += main.cpp\
            mainwindow.cpp \
            php.cpp

HEADERS  += mainwindow.h \
            php.h

OTHER_FILES +=


CONFIG(release, debug|release) {
    win32 {
        RC_FILE = libertywallet.rc
    }

    unix {
    }

    macx {
        ICON = liberty.icns
    }
}

RESOURCES += \
    libertywallet.qrc
