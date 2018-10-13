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

#include "qhiddevice.h"
#if defined(WITH_HIDAPI) || defined(WITH_HIDAPI_LIBUSB) || defined(WITH_HIDAPI_HIDRAW)
#include "qhiddevice_hidapi.h"
#elif defined(Q_OS_WIN32)
#include "qhiddevice_win32.h"
#endif

#include <QDebug>
#include <QThread>

QHIDDevice::QHIDDevice(int vendorId, int deviceId, int usagePage, int usage, QObject *parent)
    : QObject(parent)
    , inputBufferLength(64)
    , outputBufferLength(64)
    , writeDelayValue(20)
    , readTimeoutValue(3000)
    , d_ptr(new QHIDDevicePrivate(this, vendorId, deviceId, usagePage, usage))
{
}

QHIDDevice::~QHIDDevice()
{
    d_ptr->q_ptr = nullptr;
    delete d_ptr;
    d_ptr = nullptr;
}

bool QHIDDevice::open(int vendorId, int deviceId, int usagePage, int usage)
{
    d_ptr->q_ptr = nullptr;
    delete d_ptr;
    d_ptr = new QHIDDevicePrivate(this, vendorId, deviceId, usagePage, usage);
    return d_ptr->isValid();
}

bool QHIDDevice::isValid() const
{
    Q_D(const QHIDDevice);
    return d->isValid();
}

int QHIDDevice::sendFeatureReport(const char *report, int length)
{
    Q_D(QHIDDevice);
    auto ret = d->sendFeatureReport(report, length);
    if (writeDelayValue > 0)
        QThread::msleep(ulong(writeDelayValue));
    return ret;
}

int QHIDDevice::getFeatureReport(char *report, int length)
{
    Q_D(QHIDDevice);
    return d->getFeatureReport(report, length);
}

int QHIDDevice::write(char report, const char *buffer, int length)
{
    Q_D(QHIDDevice);
    int offset = 0;

    while (length > 0)
    {
        QByteArray chunk;
        chunk.reserve(outputBufferLength);
        chunk.append(report).append(buffer + offset, qMin(length, outputBufferLength));
        auto written = d->write(chunk.cbegin(), chunk.size());

        if (written <= 0)
            return written;

        if (writeDelayValue > 0)
            QThread::msleep(ulong(writeDelayValue));
        offset += written - 1;
        length -= written - 1;
    }

    return offset;
}

int QHIDDevice::read(char *buffer, int length)
{
    return read(buffer, length, readTimeoutValue);
}

int QHIDDevice::read(char *buffer, int length, int readTimeout)
{
    Q_D(QHIDDevice);
    int offset = 0;

    while (length > 0)
    {
        auto read = d->read(buffer + offset, length, readTimeout);

        if (read <= 0)
            return read;

        offset += read;
        length -= read;
    }

    return offset;
}

int QHIDDevice::readTimeout() const
{
    return readTimeoutValue;
}

void QHIDDevice::setReadTimeout(int value)
{
    readTimeoutValue = value;
}

int QHIDDevice::writeDelay() const
{
    return writeDelayValue;
}

void QHIDDevice::setWriteDelay(int value)
{
    writeDelayValue = value;
}
