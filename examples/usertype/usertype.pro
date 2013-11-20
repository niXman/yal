
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
	-lboost_filesystem

SOURCES += main.cpp \
	 ../../src/yal/yal.cpp

