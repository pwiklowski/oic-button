TEMPLATE = app

QT += qml quick

LIBS += -L../liboic-build -loic
INCLUDEPATH += ../liboic

CONFIG += c++11

SOURCES += main.cpp \
    OICServer.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    OICServer.h
