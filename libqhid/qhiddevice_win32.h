/*
 *      Copyright 2018 Pavel Bludov <pbludov@gmail.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License along
 *      with this program; if not, write to the Free Software Foundation, Inc.,
 *      51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef QHIDDEVICE_WIN32_H
#define QHIDDEVICE_WIN32_H

#include <QObject>
#include <qt_windows.h>

class QHIDDevice;
class QHIDDevicePrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(QHIDDevice)

public:
    QHIDDevicePrivate(QHIDDevice *q_ptr, int vendorId, int deviceId, int usagePage, int usage);
    ~QHIDDevicePrivate();

    bool isValid() const;

    int sendFeatureReport(const char *buffer, int length);
    int getFeatureReport(char *buffer, int length);

    int write(const char *buffer, int length);
    int read(char *buffer, int length, unsigned int timeout);

private:
    HANDLE hDevice;
    OVERLAPPED overlapped;
    QHIDDevice *q_ptr;
};

#endif // QHIDDEVICE_WIN32_H
