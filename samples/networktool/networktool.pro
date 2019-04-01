######################################################################
# Automatically generated by qmake (3.0) ?? ?? 9 16:30:48 2018
######################################################################



TEMPLATE = app
TARGET = NetworkTool

QT += core network xml
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += . \
                $$PWD/../../include \
                $$PWD/resources

# Input
HEADERS += networktool.h listview.h
FORMS += networktool.ui addtask.ui addBatchTask.ui
SOURCES += main.cpp networktool.cpp listview.cpp

CONFIG(debug, debug|release) {
        DESTDIR = $$PWD/../../bin/Debug
        LIBPATH += $$PWD/../../lib/Debug $$PWD/../../bin/Debug
        LIBS += -lQMultiThreadNetworkd
} else {
        DESTDIR = $$PWD/../../bin/Release
        LIBPATH += $$PWD/../../lib/Release $$PWD/../../bin/Release
        LIBS += -lQMultiThreadNetwork
}

RC_ICONS = resources/networktool.ico

RESOURCES += \
    networktool.qrc