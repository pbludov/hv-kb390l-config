###############################################################################
#
#      Copyright 2018 Pavel Bludov <pbludov@gmail.com>
#
#      This program is free software; you can redistribute it and/or modify
#      it under the terms of the GNU General Public License as published by
#      the Free Software Foundation; either version 2 of the License, or
#      (at your option) any later version.
#
#      This program is distributed in the hope that it will be useful,
#      but WITHOUT ANY WARRANTY; without even the implied warranty of
#      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#      GNU General Public License for more details.
#
#      You should have received a copy of the GNU General Public License along
#      with this program; if not, write to the Free Software Foundation, Inc.,
#      51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
###############################################################################
lessThan(QT_MAJOR_VERSION, 5) \
    | equals(QT_MAJOR_VERSION, 5) : lessThan(QT_MINOR_VERSION, 2) \
        : error (QT 5.2 or newer is required)

isEmpty(PREFIX): PREFIX   = /usr
DEFINES += PREFIX=$$PREFIX
CONFIG  += c++11
QT      += core gui widgets

include (libqhid/libqhid.pri)

TEMPLATE = app
TARGET   = hv-kb390l-config
VERSION  = 1.0

DEFINES += PRODUCT_NAME=\\\"$$TARGET\\\" \
    PRODUCT_VERSION=\\\"$$VERSION\\\"

SOURCES += src/buttonedit.cpp \
    src/enumedit.cpp \
    src/macroedit.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/mousebuttonbox.cpp \
    src/kb390l.cpp \
    src/pagelight.cpp \
    src/pagemacro.cpp \
    src/usbcommandedit.cpp \
    src/usbscancodeedit.cpp \
    src/pagespeed.cpp

HEADERS  += src/buttonedit.h \
    src/enumedit.h \
    src/macroedit.h \
    src/mainwindow.h \
    src/mousebuttonbox.h \
    src/kb390l.h \
    src/pagelight.h \
    src/pagemacro.h \
    src/usbcommandedit.h \
    src/usbscancodeedit.h \
    src/pagespeed.h \
    src/kbwidget.h

FORMS    += ui/mainwindow.ui \
    ui/pagelight.ui \
    ui/pagemacro.ui \
    ui/pagespeed.ui

RESOURCES += \
    res/hv-kb390l-config.qrc

# MacOS specific
ICON = res/hv-kb390l-config.icns
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
QMAKE_TARGET_BUNDLE_PREFIX = github.pbludov

# Windows specific
RC_ICONS = res/hv-kb390l-config.ico
QMAKE_TARGET_COPYRIGHT = Pavel Bludov <pbludov@gmail.com>
QMAKE_TARGET_DESCRIPTION = HAVIT KB390L keyboard configuration utility.

# Linux specific
target.path=$$PREFIX/bin
man.files=doc/hv-kb390l-config.1
man.path=$$PREFIX/share/man/man1
shortcut.files = hv-kb390l-config.desktop
shortcut.path = $$PREFIX/share/applications
icon.files = res/hv-kb390l-config.png
icon.path = $$PREFIX/share/icons/hicolor/48x48/apps
udev.files = 51-hv-kb390l-keyboard.rules
udev.path = /etc/udev/rules.d

INSTALLS += target man icon shortcut udev
