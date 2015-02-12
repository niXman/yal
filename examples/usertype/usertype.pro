
TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += \
	-std=c++11

INCLUDEPATH += \
	../../include

LIBS += \
	-lboost_system \
	-lboost_filesystem \
	-lz

SOURCES += \
	main.cpp \
	../../src/yal/yal.cpp \
	../../src/yal/datetime.cpp

HEADERS += \
    ../../include/yal/yal.hpp \
    ../../include/yal/datetime.hpp \
    ../../include/yal/options.hpp \
    ../../include/yal/throw.hpp

