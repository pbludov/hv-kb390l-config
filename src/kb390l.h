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

#ifndef KB390L_H
#define KB390L_H

#include <QObject>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(UsbIo)

class KB390L : public QObject
{
    Q_PROPERTY(int lightType READ lightType WRITE setLightType)
    Q_PROPERTY(int LightDelay READ LightDelay WRITE setLightDelay)
    Q_PROPERTY(int lightBrightness READ lightBrightness WRITE setLightBrightness)
    Q_PROPERTY(int lightDirection READ lightDirection WRITE setLightDirection)
    Q_PROPERTY(int reportRate READ reportRate WRITE setReportRate)
    Q_PROPERTY(bool unsavedChanges READ unsavedChanges)

    Q_OBJECT

    enum Command
    {
        CmdPing,
        CmdReportRate,
        CmdResponseTime = 4,
        CmdControl = 8,
        CmdGameMode = 9,
        CmdButtons = 0x0D,
        CmdEnabledButtons = 0x0E,
        CmdMacro = 0x11,
        CmdDIY = 0x12, // DIY brightness per button
        CmdReset = 0x13,
        CmdColor = 0x14, // Useless for this model
        CmdFlagGet = 0x80,
    };

    enum Notify
    {
        NotifyAdvanced = 0x00,
        NotifyChanged = 0x04,
    };

    enum LightOffset
    {
        LightTypeOffset = 3,
        LightDelayOffset,
        LightBrightnessOffset,
        LightDirectionOffset = 7,
    };

public:
    enum Constants
    {
        MinMacroNum = 0,
        MaxMacroNum = 31,
        MaxReportRate = 3,
        MaxResponseTime = 10,
        MaxLightDirection = 4,
        MaxLightDelay = 10,
        MaxLightBrightness = 50,
        ButtonsPerRow = 21,
    };

    enum Event
    {
        EventKey,
        EventButton,
        EventFunctionalKey,
        EventCommand,
        EventMacro,
        EventAdvanced = 10,
        EventCustom = 0xFE,
    };

    enum KeyIndex
    {
        KeyLCtrl,
        KeySuper,
        KeyLAlt,
        KeyReserved1,
        KeyReserved2,
        KeySpace,
        KeyReserved3,
        KeyReserved4,
        KeyReserved5,
        KeyRAlt,
        KeyFn,
        KeyMenu,
        KeyReserved6,
        KeyRCtrl,
        KeyLeft,
        KeyDown,
        KeyRight,

        KeyLShift = 21,
        KeyZ,
        KeyX,
        KeyC,
        KeyV,
        KeyB,
        KeyN,
        KeyM,
        KeyComma,
        KeyDot,
        KeySlash,
        KeyReserved7,
        KeyRShift,
        KeyReserved8,
        KeyReserved9,
        KeyUp,

        KeyCapsLock = 42,
        KeyA,
        KeyS,
        KeyD,
        KeyF,
        KeyG,
        KeyH,
        KeyJ,
        KeyK,
        KeyL,
        KeySemicolon,
        KeyQuote,
        KeyReserved10,
        KeyEnter,

        KeyTab = 63,
        KeyQ,
        KeyW,
        KeyE,
        KeyR,
        KeyT,
        KeyY,
        KeyU,
        KeyI,
        KeyO,
        KeyP,
        KeyLParen,
        KeyRParen,
        KeyBackSlash,
        KeyDel,
        KeyEnd,
        KeyPgDn,

        KeyTilde = 84,
        Key1,
        Key2,
        Key3,
        Key4,
        Key5,
        Key6,
        Key7,
        Key8,
        Key9,
        Key0,
        KeyMinus,
        KeyPlus,
        KeyBackspace,
        KeyInsert,
        KeyHome,
        KeyPgUp,

        KeyEsc = 105,
        KeyReserved11,
        KeyF1,
        KeyF2,
        KeyF3,
        KeyF4,
        KeyF5,
        KeyF6,
        KeyF7,
        KeyF8,
        KeyF9,
        KeyF10,
        KeyF11,
        KeyF12,
        KeySysRq,
        KeyScrollLock,
        KeyPause,
    };

    enum MouseButton
    {
        MouseLeftButton = 0xF0,
        MouseRightButton,
        MouseMiddleButton,
        MouseBackButton,
        MouseForwardButton,
        WheelLeftButton,
        WheelRightButton,
        WheelUpButton,
        WheelDownButton,
    };

    enum LightType
    {
        LightStatic = 1,
        LightBreath,
        LightWave,
        LightReactive,
        LightSidewinder,
        LightRipple,
        LightAltReactive,
        LightAltWave,
        LightAltSidewinder,
        LightRaindrop,
        LightWortex,
        LightSpotlight,
        LightRadar,
        LightRunningWater,
        LightRunningMold,
        LightShadeMold,
        LightMask1 = 51,
        LightMask2,
        LightMask3,
        LightMask4,
        LightMask5,
    };

    explicit KB390L(QObject *parent = nullptr);
    ~KB390L();

    int flag(Command cmd, int offset = 2);
    void setFlag(Command cmd, int value, int offset = 2);

    int reportRate();
    void setReportRate(int value);

    int responseTime();
    void setResponseTime(int value);

    int gameMode();
    void setGameMode(int value);

    int lightType();
    void setLightType(int value);

    int LightDelay();
    void setLightDelay(int value);

    int lightBrightness();
    void setLightBrightness(int value);

    int lightDirection();
    void setLightDirection(int value);

    bool unsavedChanges();
    bool save();

    int button(KeyIndex btn);
    void setButton(KeyIndex btn, int value);

    bool buttonEnabled(KeyIndex btn);
    void setButtonEnabled(KeyIndex btn, bool value);

    QByteArray macro(int index);
    void setMacro(int index, const QByteArray &value);

    bool ping();
    bool backupConfig(class QIODevice *storage);
    bool restoreConfig(class QIODevice *storage);
    bool resetToFactoryDefaults();

protected:
    virtual void timerEvent(QTimerEvent *evt);

signals:
    void connectChanged(bool connected);
    void genericCommand(int index);
    void changed(KB390L* kb);

private slots:
    void deviceArrival(const QString &path);
    void deviceRemove();

private:
    QByteArray report(Command b1, char b2 = 0, char b3 = 0, char b4 = 0, char b5 = 0, char b6 = 0, char b7 = 0);
    QByteArray readPage(Command page, int idx = 0);
    bool writePage(const QByteArray& data, Command page, int idx = 0);

    int readByte(Command page, int offset);
    void writeByte(Command page, int offset, int value);

    class QHIDDevice *device;
    class QHIDDevice *eventDevice;
    class QHIDMonitor *monitor;
    int timerId;

    std::map<int, QByteArray> cache;
    std::map<int, bool> dirtyPages;
};

#endif // KB390L_H
