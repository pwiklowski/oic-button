TEMPLATE = app

QT += qml quick

LIBS += -L../liboic-build -loic
INCLUDEPATH += ../liboic


SOURCES += main.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)
