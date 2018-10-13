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

#ifndef QHIDDEVICE_H
#define QHIDDEVICE_H

#include <QObject>

class QHIDDevicePrivate;
class QHIDDevice : public QObject
{
    Q_PROPERTY(int writeDelay READ writeDelay WRITE setWriteDelay)
    Q_PROPERTY(int readTimeout READ readTimeout WRITE setReadTimeout)

    Q_OBJECT
    Q_DECLARE_PRIVATE(QHIDDevice)

public:
    QHIDDevice(int vendorId, int deviceId, int usagePage, int usage, QObject *parent = 0);
    ~QHIDDevice();

    bool open(int vendorId, int deviceId, int usagePage, int usage);
    bool isValid() const;

    int sendFeatureReport(const char *report, int length);
    int getFeatureReport(char *report, int length);

    int write(char report, const char *buffer, int length);
    int read(char *buffer, int length);
    int read(char *buffer, int length, int timeout);

    int readTimeout() const;
    void setReadTimeout(int value);

    int writeDelay() const;
    void setWriteDelay(int value);

protected:
    int inputBufferLength;
    int outputBufferLength;
    int writeDelayValue;
    int readTimeoutValue;
    class QHIDDevicePrivate *d_ptr;
};

#endif // QHIDDEVICE_H
