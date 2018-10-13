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

#include "kb390l.h"
#include "qhiddevice.h"
#include "qhidmonitor.h"

#include <QRgb>
#include <QThread>

#define VENDOR  0x04D9
#define PRODUCT 0xA131
#define GENERIC_USAGE_PAGE 0xFF01
#define GENERIC_USAGE      0x0001
#define EVENT_USAGE_PAGE   0xFF02
#define EVENT_USAGE        0x0001

#define PAGE_SIZE 64

Q_LOGGING_CATEGORY(UsbIo, "usb")

#ifndef qCInfo
// QT 5.2 does not have qCInfo
#define qCInfo qCWarning
#endif

static char crc(const QByteArray data)
{
    char sum = -1;

    foreach (auto ch, data)
    {
        sum -= ch;
    }

    return sum;
}

KB390L::KB390L(QObject *parent)
    : QObject(parent)
    , device(new QHIDDevice(VENDOR, PRODUCT, GENERIC_USAGE_PAGE, GENERIC_USAGE, this))
    , eventDevice(new QHIDDevice(VENDOR, PRODUCT, EVENT_USAGE_PAGE, EVENT_USAGE, this))
    , monitor(new QHIDMonitor(VENDOR, PRODUCT, this))
    , timerId(0)
{
    connect(monitor, SIGNAL(deviceArrival(QString)), this, SLOT(deviceArrival(QString)));
    connect(monitor, SIGNAL(deviceRemove()), this, SLOT(deviceRemove()));

    if (eventDevice->isValid()/*TODO && !report(CmdEventMask, EventAll).isNull()*/)
    {
        timerId = startTimer(10);
    }
}

KB390L::~KB390L()
{
    if (timerId)
    {
        killTimer(timerId);
        timerId = 0;
    }
}

void KB390L::deviceArrival(const QString &path)
{
    qCInfo(UsbIo) << "Detected device arrival at" << path;
    auto connected = device->open(VENDOR, PRODUCT, GENERIC_USAGE_PAGE, GENERIC_USAGE) && ping();
    connectChanged(connected);
    if (connected)
    {
        eventDevice->open(VENDOR, PRODUCT, EVENT_USAGE_PAGE, EVENT_USAGE);
    }
}

void KB390L::deviceRemove()
{
    qCInfo(UsbIo) << "Detected device removal";
    if (timerId)
    {
        killTimer(timerId);
        timerId = 0;
    }
    connectChanged(false);
}

QByteArray KB390L::report(Command b1, char b2, char b3, char b4, char b5, char b6, char b7)
{
    QByteArray data;
    data.reserve(9);
    data.push_back('\x0');
    data.push_back(char(b1));
    data.push_back(b2);
    data.push_back(b3);
    data.push_back(b4);
    data.push_back(b5);
    data.push_back(b6);
    data.push_back(b7);
    data.push_back(crc(data));

    qCDebug(UsbIo) << "send" << data.toHex();
    int sent = device->sendFeatureReport(data.cbegin(), data.length());
    if (sent != data.length())
    {
        qCWarning(UsbIo) << "send failed: got" << sent << "expected" << data.length();
        return nullptr;
    }

    data.fill('\x0');
    int read = device->getFeatureReport(data.begin(), data.length());
    if (read != data.length())
    {
        qCWarning(UsbIo) << "recv failed: got" << read << "expected" << data.length();
        return nullptr;
    }

    qCDebug(UsbIo) << "recv" << data.toHex();
    return data;
}

QByteArray KB390L::readPage(Command page, int idx)
{
    auto cacheId = idx << 8 | page;
    auto iter = cache.find(cacheId);

    if (iter != cache.end())
        return iter->second;

    auto cmd = Command(CmdFlagGet | page);
    auto resp = report(cmd, 0, char(idx));

    if (resp == nullptr || resp.length() < 4 || resp.at(1) != char(cmd)
            || resp.at(3) != idx)
    {
        qCWarning(UsbIo) << "readPage: invalid response:" << resp.toHex();
        return nullptr;
    }

    int numBytes = (page == CmdEnabledButtons ? 1 : resp.at(4)) * PAGE_SIZE;
    QByteArray value(numBytes, 0);

    auto read = device->read(value.begin(), numBytes);
    if (read != numBytes)
    {
        qCWarning(UsbIo) << "readPage: read failed: got" << read << "expected" << numBytes;
        return nullptr;
    }

    qCDebug(UsbIo) << "readPage" << page << idx << value.toHex();

    dirtyPages[cacheId] = false;
    return cache[cacheId] = value;
}

bool KB390L::writePage(const QByteArray &data, Command page, int idx)
{
    QByteArray cmd(9, '\x0');
    cmd[1] = char(page);
    cmd[2] = 0;
    cmd[3] = char(idx);
    cmd[4] = char(page == CmdEnabledButtons ? 18 : data.length() / PAGE_SIZE);
    cmd[8] = crc(cmd);

    qCDebug(UsbIo) << "send" << cmd.toHex();
    int sent = device->sendFeatureReport(cmd.cbegin(), cmd.length());
    if (sent != cmd.length())
    {
        qCWarning(UsbIo) << "writePage: send failed: got" << sent << "expected" << cmd.length();
        return false;
    }

    qCDebug(UsbIo) << "writePage" << page << idx << data.toHex();
    auto written = device->write(0, data, data.length());
    if (written != data.length())
    {
        qCWarning(UsbIo) << "writePage: write failed: got" << written << "expected" << data.length();
        return false;
    }

    auto cacheId = idx << 8 | page;
    dirtyPages[cacheId] = false;
    return true;
}

int KB390L::button(KeyIndex btn)
{
    auto bytes = readPage(CmdButtons);

    if (bytes.isEmpty())
        return -1;

    auto btns = reinterpret_cast<const int *>(bytes.constData());
    return btns[btn];
}

void KB390L::setButton(KeyIndex btn, int value)
{
    auto bytes = readPage(CmdButtons);

    if (!bytes.isEmpty())
    {
        auto btns = reinterpret_cast<int *>(bytes.data());

        if (btns[btn] != value)
        {
            dirtyPages[CmdButtons] = true;
            btns[btn] = value;
            cache[CmdButtons] = bytes;
        }
    }
}

bool KB390L::buttonEnabled(KeyIndex btn)
{
    auto bytes = readPage(CmdEnabledButtons);

    if (bytes.isEmpty())
        return -1;

    int row = btn / ButtonsPerRow;
    auto btns = reinterpret_cast<const int *>(bytes.constData() + row * 3);
    int bit    = btn % ButtonsPerRow;
    int mask   = 1 << bit;
    return 0 != (*btns & mask);
}

void KB390L::setButtonEnabled(KeyIndex btn, bool value)
{
    auto bytes = readPage(CmdEnabledButtons);

    if (!bytes.isEmpty())
    {
        int row = btn / ButtonsPerRow;
        auto btns = reinterpret_cast<int *>(bytes.data() + row * 3);
        int bit    = btn % ButtonsPerRow;
        int mask   = 1 << bit;
        bool curr  = 0 != (*btns & mask);

        if (value != curr)
        {
            dirtyPages[CmdEnabledButtons] = true;
            if (value)
                *btns |= mask;
            else
                *btns &= ~mask;
            cache[CmdEnabledButtons] = bytes;
        }
    }
}

QByteArray KB390L::macro(int index)
{
    return readPage(CmdMacro, index);
}

void KB390L::setMacro(int index, const QByteArray &value)
{
    auto page = readPage(CmdMacro, index);
    if (page != value)
    {
        page = value;
        dirtyPages[index << 8 | CmdMacro] = true;
    }
}

int KB390L::readByte(Command page, int offset)
{
    auto bytes = readPage(page);
    return !bytes.isEmpty() ? 0xFF & bytes[offset] : -1;
}

void KB390L::writeByte(Command page, int offset, int value)
{
    auto bytes = readPage(page);
    if (!bytes.isEmpty())
    {
        if ((0xFF & bytes[offset]) != (0xFF & value))
        {
            dirtyPages[page] = true;
            bytes[offset] = char(value);
            cache[page] = bytes;
        }
    }
}

int KB390L::flag(Command cmd, int offset)
{
    QByteArray resp;
    auto iter = cache.find(cmd);

    if (iter != cache.end())
    {
        resp = iter->second;
    }
    else
    {
        resp = report(Command(cmd | CmdFlagGet));
        if (!resp.isNull())
        {
            cache[cmd] = resp;
        }
    }

    return resp.isNull() || resp.length() < 8
        || resp.at(1) != char(cmd | CmdFlagGet) ? -1
            : (0xFF & resp.at(offset));
}

void KB390L::setFlag(Command cmd, int value, int offset)
{
    QByteArray resp;
    auto iter = cache.find(cmd);

    if (iter != cache.end())
    {
        resp = iter->second;
    }
    else
    {
        resp = report(Command(cmd | CmdFlagGet));
    }

    if (!resp.isEmpty() && resp[offset] != char(value))
    {
        resp[offset] = char(value);
        cache[cmd] = resp;
        report(cmd, resp[2], resp[3], resp[4], resp[5], resp[6], resp[7]);
    }
}

int KB390L::reportRate()
{
    return flag(CmdReportRate);
}

void KB390L::setReportRate(int value)
{
    Q_ASSERT(value >= 0 && value <= MaxReportRate);

    report(CmdReportRate, char(value));
}

int KB390L::responseTime()
{
    return flag(CmdResponseTime);
}

void KB390L::setResponseTime(int value)
{
    Q_ASSERT(value > 0 && value <= MaxResponseTime);

    report(CmdResponseTime, char(value));
}

int KB390L::gameMode()
{
    return flag(CmdGameMode);
}

void KB390L::setGameMode(int value)
{
    report(CmdGameMode, char(value));
}

int KB390L::lightType()
{
    return flag(CmdControl, LightTypeOffset);
}

void KB390L::setLightType(int value)
{
    Q_ASSERT(value >= 0 && value <= 240);

    setFlag(CmdControl, value, LightTypeOffset);
}

int KB390L::LightDelay()
{
    return flag(CmdControl, LightDelayOffset);
}

void KB390L::setLightDelay(int value)
{
    Q_ASSERT(value >= 0 && value <= MaxLightDelay);

    setFlag(CmdControl, value, LightDelayOffset);
}

int KB390L::lightBrightness()
{
    return flag(CmdControl, LightBrightnessOffset);
}

void KB390L::setLightBrightness(int value)
{
    Q_ASSERT(value >= 0 && value <= MaxLightBrightness);

    setFlag(CmdControl, value, LightBrightnessOffset);
}

int KB390L::lightDirection()
{
    return flag(CmdControl, LightDirectionOffset);
}

void KB390L::setLightDirection(int value)
{
    Q_ASSERT(value >= 0 && value <= MaxLightDirection);

    setFlag(CmdControl, value, LightDirectionOffset);
}

bool KB390L::resetToFactoryDefaults()
{
     return !report(CmdReset, '\xFF').isEmpty();
}

bool KB390L::ping()
{
     return -1 != flag(CmdPing);
}

void KB390L::timerEvent(QTimerEvent *evt)
{
    QObject::timerEvent(evt);

    char buffer[4];
    while (eventDevice->read(buffer, sizeof(buffer), 20) == sizeof(buffer))
    {
        qCDebug(UsbIo) << "event" << QByteArray(buffer, 4).toHex();
        if (buffer[0] == 4)
        {
            switch (buffer[1])
            {
            case NotifyChanged:
                cache.clear();
                changed(this);
                break;
            case NotifyAdvanced:
                genericCommand(buffer[2]);
                break;
            }
        }
        else
        {
            qCDebug(UsbIo) << "???" << QByteArray(buffer, 4).toHex();
        }
    }
}

bool KB390L::unsavedChanges()
{
    return dirtyPages.cend() != std::find_if(dirtyPages.cbegin(), dirtyPages.cend(),
                                    [](const std::map<int, bool>::value_type &x) { return x.second; });
}

bool KB390L::save()
{
    foreach (auto page, cache)
    {
        if (!dirtyPages[page.first])
            continue;

        if (!writePage(page.second, Command(0xFF & page.first), 0xFF & (page.first >> 8)))
            return false;
    }

    return true;
}

bool KB390L::backupConfig(QIODevice *storage)
{
    int bytes = 0;

    auto page = readPage(CmdButtons);
    if (!page.isEmpty())
        bytes += storage->write(page);

    for (int i = MinMacroNum; i <= MaxMacroNum; ++i)
    {
        page = readPage(CmdMacro, i);
        if (!page.isEmpty())
            bytes += storage->write(page);
    }

    return bytes == PAGE_SIZE * 8 + PAGE_SIZE * 3 * 32;
}

bool KB390L::restoreConfig(QIODevice *storage)
{
    if (storage->size() != PAGE_SIZE * 8 + PAGE_SIZE * 3 * 32)
        return false;

    auto page = storage->read(PAGE_SIZE * 8);
    if (page.isEmpty() || !writePage(page, CmdButtons))
        return false;

    for (int i = MinMacroNum; i <= MaxMacroNum; ++i)
    {
        page = storage->read(PAGE_SIZE * 3);
        if (page.isEmpty() || !writePage(page, CmdMacro, i))
            return false;
    }

    return true;
}
