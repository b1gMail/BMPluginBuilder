# BMPluginBuilder

This repository contains the GPL-ized version of BMPluginBuilder, a tool to create .bmplugin plugin archives for b1gMail.

## Overview
There are two versions of this tool:
 - Command line version in folder `cli/` - useful for automated build flows, scripts, etc. This version has been used internally to produce the .bmplugin file for the b1gMailServer admin plugin, for example.
 - Graphical user interface version in folder `gui/`. This is the tool which has been distributed as part of the b1gMail Plugin SDK.

## Building
The CLI version can be built with CMake. For the GUI version, use Qt's qmake or Qt Creator. The GUI version uses a fairly old version of Qt (4.x series) and might need adjustements to build with newer Qt versions.
