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

INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/qhiddevice.h \
    $$PWD/qhidmonitor.h

SOURCES += \
    $$PWD/qhiddevice.cpp \
    $$PWD/qhidmonitor.cpp

CONFIG += link_pkgconfig

OPTIONAL_MODULES = hidapi hidapi-libusb libusb-1.0 libudev
for (mod, OPTIONAL_MODULES) {
  modVer = $$system(pkg-config --silence-errors --modversion $$mod)
  !isEmpty(modVer) {
    message("Found $$mod version $$modVer")
    PKGCONFIG += $$mod
    DEFINES += WITH_$$upper($$replace(mod, \W, _))
  }
}

contains(DEFINES, WITH_LIBUSB_1_0) {
  SOURCES += $$PWD/qhidmonitor_libusb.cpp
  HEADERS += $$PWD/qhidmonitor_libusb.h
}
else:contains(DEFINES, WITH_LIBUDEV) {
  SOURCES += $$PWD/qhidmonitor_udev.cpp
  HEADERS += $$PWD/qhidmonitor_udev.h
}
else:win32 {
  SOURCES += $$PWD/qhidmonitor_win32.cpp
  HEADERS += $$PWD/qhidmonitor_win32.h
}
else {
  error("Need libudev or libusb-1.0 development package.")
}

contains(DEFINES, WITH_HIDAPI) || contains(DEFINES, WITH_HIDAPI_LIBUSB) {
  SOURCES += $$PWD/qhiddevice_hidapi.cpp
  HEADERS += $$PWD/qhiddevice_hidapi.h
}
else:win32 {
  SOURCES += $$PWD/qhiddevice_win32.cpp
  HEADERS += $$PWD/qhiddevice_win32.h
  LIBS += -lhid -lsetupapi
}
else {
  error("Need hidapi or hidapi-libusb development package.")
}
