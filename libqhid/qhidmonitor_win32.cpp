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
#include "qhidmonitor_win32.h"

#include <QDebug>

#include <dbt.h>

LRESULT CALLBACK QHIDMonitorPrivate::wndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message != WM_DEVICECHANGE)
        return DefWindowProc(hwnd, message, wParam, lParam);

    DEV_BROADCAST_HDR *hdr = (DEV_BROADCAST_HDR *)lParam;

    if (hdr->dbch_devicetype != DBT_DEVTYP_DEVICEINTERFACE)
        return DefWindowProc(hwnd, message, wParam, lParam);

    PDEV_BROADCAST_DEVICEINTERFACE devInterface = (PDEV_BROADCAST_DEVICEINTERFACE)hdr;
    auto name = QString::fromStdWString(devInterface->dbcc_name);
    int vendorId = -1, deviceId = -1, interfaceNo = -1;

    for (auto ids = name.split(QRegExp("[#&_]")); !ids.isEmpty(); ids.pop_front())
    {
        if (ids.front() == "VID")
        {
            ids.pop_front();
            vendorId = strtol(ids.front().toUtf8(), nullptr, 16);
            continue;
        }

        if (ids.front() == "PID")
        {
            ids.pop_front();
            deviceId = strtol(ids.front().toUtf8(), nullptr, 16);
            continue;
        }

        if (ids.front() == "MI")
        {
            ids.pop_front();
            interfaceNo = strtol(ids.front().toUtf8(), nullptr, 16);
            continue;
        }
    }

    auto userData = GetWindowLongPtr(hwnd, GWLP_USERDATA);
    auto p = reinterpret_cast<QHIDMonitorPrivate *>(userData);
    auto q = p->q_func();

    if (vendorId == p->vendorId && deviceId == p->deviceId && interfaceNo == 0)
    {
        if (wParam == DBT_DEVICEARRIVAL)
        {
            emit(q->deviceArrival(name));
        }
        else if (wParam == DBT_DEVICEREMOVECOMPLETE)
        {
            emit(q->deviceRemove());
        }
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

QHIDMonitorPrivate::QHIDMonitorPrivate(QHIDMonitor *q_ptr, int vendorId, int deviceId)
    : vendorId(vendorId)
    , deviceId(deviceId)
    , hDevNotify(nullptr)
    , q_ptr(q_ptr)
{
    const GUID InterfaceClassGuid = {0x4d1e55b2, 0xf16f, 0x11cf, {0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30}};
    hwnd = CreateWindow(TEXT("STATIC"), TEXT(""), 0, 0, 0, 0, 0, HWND_MESSAGE, 0, 0, 0);

    if (!hwnd)
    {
        qWarning() << "QHIDMonitorPrivate: Failed to create top-level notify window" << GetLastError();
        return;
    }

    SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)wndProc);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) this);
    DEV_BROADCAST_DEVICEINTERFACE dbdi;
    ZeroMemory(&dbdi, sizeof(dbdi));
    dbdi.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    dbdi.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    dbdi.dbcc_classguid = InterfaceClassGuid;
    hDevNotify = RegisterDeviceNotification(hwnd, &dbdi, DEVICE_NOTIFY_WINDOW_HANDLE);

    if (!hDevNotify)
    {
        qWarning() << "QHIDMonitorPrivate: Failed to register device notification" << GetLastError();
    }
}

QHIDMonitorPrivate::~QHIDMonitorPrivate()
{
    if (hDevNotify)
    {
        UnregisterDeviceNotification(hDevNotify);
        hDevNotify = nullptr;
    }

    if (hwnd)
    {
        DestroyWindow(hwnd);
        hwnd = nullptr;
    }
}
