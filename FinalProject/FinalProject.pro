CONFIG += opengl / console
CONFIG -= qt
LIBS += -framework GLUT
LIBS += -lGLEW

SOURCES += main.c \
    objmesh.c

HEADERS += \
    objmesh.h

