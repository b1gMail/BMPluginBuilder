#-------------------------------------------------
#
# Project created by QtCreator 2010-11-27T11:38:41
#
#-------------------------------------------------

QT       += core gui

TARGET = BMPluginBuilder
TEMPLATE = app
RC_FILE = resource.rc
LIBS += -lz
ICON = app.icns
QMAKE_INFO_PLIST = info.plist
CONFIG += static
CONFIG -= exceptions
LIBS += -static-libgcc

SOURCES += main.cpp\
        FormMain.cpp \
    MetaTab.cpp \
    FilesTab.cpp \
	PluginFactory.cpp \
	md5.c

HEADERS  += FormMain.h \
    MetaTab.h \
    FilesTab.h \
	PluginFactory.h \
	md5.h

RESOURCES += \
    resource.qrc

TRANSLATIONS = bmpluginbuilder_de.ts

OTHER_FILES += \
    resource.rc
