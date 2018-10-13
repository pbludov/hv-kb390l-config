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

#ifndef QHIDMONITOR_LIBUSB_H
#define QHIDMONITOR_LIBUSB_H

#include <QObject>
#include <libusb.h>

class QHIDMonitor;
class QHIDMonitorPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(QHIDMonitor)

public:
    QHIDMonitorPrivate(QHIDMonitor *q_ptr, int vendorId, int deviceId);
    ~QHIDMonitorPrivate();

protected:
    virtual void timerEvent(QTimerEvent *evt);

private:
    static int LIBUSB_CALL callback(libusb_context *, libusb_device *device, libusb_hotplug_event event, void *userData);

    int timerId;
    libusb_context *ctx;
    libusb_hotplug_callback_handle handle;
    QHIDMonitor *q_ptr;
};

#endif // QHIDMONITOR_LIBUSB_H
