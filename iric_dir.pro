TEMPLATE = app
TARGET = iric_dir
INCLUDEPATH += "../sdk/include"

LIBS += "../sdk/lib/cgnsdll_x64_ifort.lib"
LIBS += "../sdk/lib/iriclib_x64_ifort.lib"

# Input
SOURCES += iric_dir.cpp
