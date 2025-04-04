CONFIG       += ordered
TEMPLATE      = subdirs
SUBDIRS       = QGLViewer designerPlugin

QT_VERSION=$$[QT_VERSION]

equals (QT_MAJOR_VERSION, 5) {
	cache()
}

ROOT_DIRECTORY = .
INCLUDE_DIR = $${ROOT_DIRECTORY}
LIB_DIR = $${ROOT_DIRECTORY}/QGLViewer
OBJECTS_DIR = obj/
EIGEN=/mingw64/include/eigen3

# Include path, taking QGLViewer path prefix into account
INCLUDEPATH *= $${INCLUDE_DIR}
               DEPENDPATH  *= $${INCLUDE_DIR}

               INCLUDEPATH += $${EIGEN}
               INCLUDEPATH += $${OPTIM}

CONFIG -= debug_and_release
CONFIG *= release
CONFIG += debug

TEMPLATE = app
TARGET   = wd

SRC_DIR = src/
HEADERS  = $${SRC_DIR}*.hpp
HEADERS  += $${SRC_DIR}*.h
SOURCES  = $${SRC_DIR}*.cpp

QMAKE_CXXFLAGS += -g -fopenmp -O3 -D__MODE_DEBUG=3 -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable -DOPTIM_ENABLE_EIGEN_WRAPPERS -Wdeprecated-declarations -Wdeprecated-copy
QMAKE_CXXFLAGS += -g
LIBS += -L./QGLViewer -lQGLViewer -lpthread -lgomp -fopenmp -LC:/msy64/mingw64/lib -lpng
LIBS += -LC:/msys64/mingw64/bin/ -lSDL2 -lSDL2_image
LIBS += -L./QGLViewer/ -lQGLViewer2

DEFINES += PNG_STATIC
LIBS += -lpng16 -lz
include( ./examples.pri )
