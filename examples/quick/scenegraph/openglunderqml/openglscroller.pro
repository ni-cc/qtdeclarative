QT += qml quick

HEADERS += cubeView.h
SOURCES += cross-platform-samples/OpenGLScroller/cubeView.cpp main.cpp
RESOURCES += openglscroller.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/quick/scenegraph/openglunderqml
INSTALLS += target
