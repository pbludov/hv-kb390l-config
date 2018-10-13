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
#include "qhiddevice_win32.h"

#include <QDebug>

#include <SetupAPI.h>
extern "C" {
#include <Hidsdi.h>
}

QHIDDevicePrivate::QHIDDevicePrivate(QHIDDevice *q_ptr, int vendorId, int deviceId, int usagePage, int usage)
    : hDevice(INVALID_HANDLE_VALUE)
    , q_ptr(q_ptr)
{
    ZeroMemory(&overlapped, sizeof(OVERLAPPED));

    const GUID InterfaceClassGuid = {0x4d1e55b2, 0xf16f, 0x11cf, {0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30}};
    auto dis = SetupDiGetClassDevs(&InterfaceClassGuid, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (dis == INVALID_HANDLE_VALUE)
    {
        qWarning() << "SetupDiGetClassDevs failed, error" << GetLastError();
        return;
    }

    SP_DEVICE_INTERFACE_DATA did;
    SP_DEVICE_INTERFACE_DETAIL_DATA *pdidd;
    ZeroMemory(&did, sizeof(did));
    did.cbSize = sizeof(did);

    for (int idx = 0; SetupDiEnumDeviceInterfaces(dis, nullptr, &InterfaceClassGuid, idx, &did) && !isValid(); ++idx)
    {
        DWORD size = 0;
        SetupDiGetDeviceInterfaceDetail(dis, &did, nullptr, 0, &size, nullptr);
        pdidd = (SP_DEVICE_INTERFACE_DETAIL_DATA *)malloc(size);

        if (!pdidd)
        {
            qWarning() << "failed to allocate(" << size << ") bytes, error" << GetLastError();
            continue;
        }

        pdidd->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

        if (!SetupDiGetDeviceInterfaceDetail(dis, &did, pdidd, size, &size, nullptr))
        {
            qWarning() << "SetupDiGetDeviceInterfaceDetail(data) failed, error" << GetLastError();
        }
        else
        {
            auto name = QString::fromUtf16((const ushort *)&pdidd->DevicePath[0]);
            int vid = -1, pid = -1;

            for (auto ids = name.split(QRegExp("[#&_]")); !ids.isEmpty(); ids.pop_front())
            {
                if (ids.front().compare("vid", Qt::CaseInsensitive) == 0)
                {
                    ids.pop_front();
                    vid = strtol(ids.front().toUtf8(), nullptr, 16);
                    continue;
                }

                if (ids.front().compare("pid", Qt::CaseInsensitive) == 0)
                {
                    ids.pop_front();
                    pid = strtol(ids.front().toUtf8(), nullptr, 16);
                    continue;
                }
            }

            if (vid == vendorId && pid == deviceId)
            {
                hDevice = CreateFile(pdidd->DevicePath, GENERIC_WRITE | GENERIC_READ,
                    FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);

                if (!isValid())
                {
                    continue;
                }

                PHIDP_PREPARSED_DATA ppData = nullptr;
                if (!HidD_GetPreparsedData(hDevice, &ppData))
                {
                    qWarning() << "HidD_GetPreparsedData failed, error" << GetLastError();
                    CloseHandle(hDevice);
                    hDevice = INVALID_HANDLE_VALUE;
                }

                HIDP_CAPS caps;
                auto hr = HidP_GetCaps(ppData, &caps);
                if (hr == HIDP_STATUS_SUCCESS)
                {
                    if (caps.UsagePage == usagePage && caps.Usage == usage)
                    {
                        // For Windows it's wMaxPacketSize + 1 (report byte), so we decrement.
                        q_ptr->inputBufferLength = caps.InputReportByteLength;
                        q_ptr->outputBufferLength = caps.OutputReportByteLength;

                        overlapped.hEvent = CreateEvent(nullptr, false, false, nullptr);
                    }
                    else
                    {
                        // Not the interface we are looking for.
                        CloseHandle(hDevice);
                        hDevice = INVALID_HANDLE_VALUE;
                    }
                }
                else
                {
                    qWarning() << "HidP_GetCaps failed, error" << hr;
                    CloseHandle(hDevice);
                    hDevice = INVALID_HANDLE_VALUE;
                }
                HidD_FreePreparsedData(ppData);
            }
        }

        free(pdidd);
    }

    SetupDiDestroyDeviceInfoList(dis);
}

QHIDDevicePrivate::~QHIDDevicePrivate()
{
    if (isValid())
    {
        HidD_FlushQueue(hDevice);
        CloseHandle(hDevice);
        hDevice = INVALID_HANDLE_VALUE;
    }

    if (overlapped.hEvent)
    {
        CloseHandle(overlapped.hEvent);
        overlapped.hEvent = nullptr;
    }
}

bool QHIDDevicePrivate::isValid() const
{
    return hDevice != INVALID_HANDLE_VALUE;
}

int QHIDDevicePrivate::sendFeatureReport(const char *buffer, int length)
{
    // Is it safe to cast const void* to void* here?
    return isValid() && HidD_SetFeature(hDevice, (PVOID)buffer, length) ? length : -1;
}

int QHIDDevicePrivate::getFeatureReport(char *buffer, int length)
{
    return isValid() && HidD_GetFeature(hDevice, buffer, length) ? length : -1;
}

int QHIDDevicePrivate::write(const char *buffer, int length)
{
    if (!isValid())
    {
        return -1;
    }

    Q_Q(QHIDDevice);
    DWORD written = 0;
    int ret;

    if (length < q->outputBufferLength)
    {
        // Windows expects the number of bytes which are in the _longest_ report
        // (plus one for the report number) bytes even if the data is a report
        // which is shorter than that.
        QByteArray tmp(buffer, length);
        tmp.resize(q->outputBufferLength);
        ret = WriteFile(hDevice, tmp.cbegin(), tmp.length(), &written, &overlapped);
    }
    else
    {
        // Safe to send the buffer as is.
        ret = WriteFile(hDevice, buffer, length, &written, &overlapped);
    }

    if (!ret && GetLastError() == ERROR_IO_PENDING)
    {
        ret = GetOverlappedResult(hDevice, &overlapped, &written, true);
    }

    if (ret)
    {
        return qMin((int)written, length);
    }

    CancelIo(hDevice);
    return -1;
}

int QHIDDevicePrivate::read(char *buffer, int length, unsigned int timeout)
{
    Q_Q(QHIDDevice);
    DWORD read = 0;
    QByteArray tmp(q->inputBufferLength, Qt::Uninitialized);

    ResetEvent(overlapped.hEvent);
    auto ret = ReadFile(hDevice, tmp.begin(), tmp.length(), &read, &overlapped);
    if (!ret)
    {
        if (GetLastError() == ERROR_IO_PENDING && timeout)
        {
            ret = WaitForSingleObject(overlapped.hEvent, timeout);
            if (ret == WAIT_OBJECT_0)
            {
                ret = GetOverlappedResult(hDevice, &overlapped, &read, true);
            }
        }
    }

    if (ret && (int)read == tmp.length())
    {
        QByteArray::const_iterator begin;
        if (tmp.at(0))
        {
            // Copy as is (with report id)
            begin = tmp.cbegin();
        }
        else
        {
            // Remove the first byte (zero report id).
            begin = tmp.cbegin() + 1;
            --read;
        }

        length = qMin((int)read, length);
        memcpy(buffer, begin, size_t(length));
        return length;
    }

    CancelIo(hDevice);
    return -1;
}
