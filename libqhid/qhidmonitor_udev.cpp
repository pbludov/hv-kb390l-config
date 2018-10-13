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
#include "qhidmonitor_udev.h"

#include <QDebug>
#include <QSocketNotifier>

#include <libudev.h>

QHIDMonitorPrivate::QHIDMonitorPrivate(QHIDMonitor *q_ptr, int vendorId, int deviceId)
    : vendorId(vendorId)
    , deviceId(deviceId)
    , q_ptr(q_ptr)
{
    monitor = udev_monitor_new_from_netlink(udev_new(), "udev");
    udev_monitor_filter_add_match_subsystem_devtype(monitor, "usb", 0);
    udev_monitor_enable_receiving(monitor);
    int fd = udev_monitor_get_fd(monitor);
    monitorNotifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(monitorNotifier, SIGNAL(activated(int)), this, SLOT(udevDataAvailable()));
}

QHIDMonitorPrivate::~QHIDMonitorPrivate()
{
    monitorNotifier->setEnabled(false);
    auto udev = udev_monitor_get_udev(monitor);
    udev_monitor_unref(monitor);
    udev_unref(udev);
}

void QHIDMonitorPrivate::udevDataAvailable()
{
    auto device = udev_monitor_receive_device(monitor);

    if (!device)
    {
        qWarning() << "udev_monitor_receive_device fails";
        return;
    }

    auto strVendorId = udev_device_get_property_value(device, "ID_VENDOR_ID");
    auto strDeviceId = udev_device_get_property_value(device, "ID_MODEL_ID");
    auto action = udev_device_get_action(device);

    if (strVendorId && vendorId == strtol(strVendorId, nullptr, 16) && strDeviceId
        && deviceId == strtol(strDeviceId, nullptr, 16))
    {
#if 0
        auto list = udev_device_get_properties_list_entry(device);
        for (auto item = list; item; item = udev_list_entry_get_next(item))
        {
            qDebug() << udev_list_entry_get_name(item) << udev_list_entry_get_value(item);
        }
#endif
        Q_Q(QHIDMonitor);

        if (strcasecmp(action, "add") == 0)
        {
            auto path = udev_device_get_syspath(device);
            emit(q->deviceArrival(QString::fromLocal8Bit(path)));
        }
        else if (strcasecmp(action, "remove") == 0)
        {
            emit(q->deviceRemove());
        }
        else
        {
            qWarning() << "Unknown action" << action;
        }
    }

    udev_device_unref(device);
}
