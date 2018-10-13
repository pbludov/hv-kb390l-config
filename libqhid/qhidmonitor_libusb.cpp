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

#include "qhidmonitor.h"
#include "qhidmonitor_libusb.h"

#include <QDebug>

int LIBUSB_CALL QHIDMonitorPrivate::callback(
    libusb_context *, libusb_device *device, libusb_hotplug_event event, void *userData)
{
    auto q = reinterpret_cast<QHIDMonitorPrivate *>(userData)->q_func();

    if (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED == event)
    {
        QString str;
        QByteArray path(16, 0);
        int pathLen = libusb_get_port_numbers(device, (uint8_t *)path.data(), path.length());
        for (int i = 0; i < pathLen; ++i)
        {
            str.append(QString::number(path.at(i))).append(":");
        }
        str.append(QString::number(libusb_get_device_address(device)));
        emit(q->deviceArrival(str));
    }
    else
    {
        emit(q->deviceRemove());
    }

    return 0;
}

QHIDMonitorPrivate::QHIDMonitorPrivate(QHIDMonitor *q_ptr, int vendorId, int deviceId)
    : timerId(0)
    , handle(0)
    , q_ptr(q_ptr)
{
    int rc = libusb_init(&ctx);

    if (LIBUSB_SUCCESS != rc)
    {
        qWarning() << "Error creating a context" << libusb_error_name(rc);
        return;
    }

    auto events = (libusb_hotplug_event)(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT);
    rc = libusb_hotplug_register_callback(ctx, events, (libusb_hotplug_flag)0, vendorId, deviceId,
        LIBUSB_HOTPLUG_MATCH_ANY, QHIDMonitorPrivate::callback, this, &handle);

    if (LIBUSB_SUCCESS != rc)
    {
        qWarning() << "Error creating a hotplug callback" << libusb_error_name(rc);
    }
    else
    {
        timerId = startTimer(1000);
    }
}

QHIDMonitorPrivate::~QHIDMonitorPrivate()
{
    if (timerId)
    {
        killTimer(timerId);
        timerId = 0;
    }

    if (ctx)
    {
        if (handle)
        {
            libusb_hotplug_deregister_callback(ctx, handle);
            handle = 0;
        }

        libusb_exit(ctx);
        ctx = nullptr;
    }
}

void QHIDMonitorPrivate::timerEvent(QTimerEvent *evt)
{
    QObject::timerEvent(evt);
    timeval tv = {0, 10000};
    libusb_handle_events_timeout(ctx, &tv);
}
