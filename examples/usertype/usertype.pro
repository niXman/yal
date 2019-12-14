
TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += \
    -std=c++11 \
    -Wall \
    -Wextra

#DEFINES += \
#    YAL_DISABLE_LOGGING

INCLUDEPATH += \
    ../../include

LIBS += \
    -lz

SOURCES += \
    main.cpp \
    ../../src/yal.cpp \
    ../../src/index.cpp

HEADERS += \
    ../../include/yal/yal.hpp \
    ../../include/yal/options.hpp \
    ../../include/yal/throw.hpp \
    ../../include/yal/index.hpp \
    ../../include/yal/dtf.hpp
