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

#include "buttonedit.h"
#include "usbscancodeedit.h"
#include "mousebuttonbox.h"
#include "usbcommandedit.h"
#include "kb390l.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QSpinBox>

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof(*x))
#endif

ButtonEdit::ButtonEdit(QString labelText, QWidget *parent)
    : QWidget(parent)
{
    auto layout = new QHBoxLayout;
    layout->setMargin(4);

    cbEnabled = new QCheckBox(labelText);
    auto measuredWidth = fontMetrics().width("HH HH HH HH");
    cbEnabled->setMinimumWidth(measuredWidth);
    layout->addWidget(cbEnabled);
    cbMode = new QComboBox();
    cbMode->setEditable(false);
    cbMode->addItem(tr("Key"), KB390L::EventKey);
    cbMode->addItem(tr("Button"), KB390L::EventButton);
    cbMode->addItem(tr("Key Fn"), KB390L::EventFunctionalKey);
    cbMode->addItem(tr("Command"), KB390L::EventCommand);
    cbMode->addItem(tr("Macro"), KB390L::EventMacro);
    cbMode->addItem(tr("Advanced"), KB390L::EventAdvanced);
    cbMode->addItem(tr("Custom"), KB390L::EventCustom);
    layout->addWidget(cbMode);
    connect(cbMode, SIGNAL(currentIndexChanged(int)), this, SLOT(onModeChanged(int)));

    for (size_t i = 0; i < _countof(editScans); ++i)
    {
        editScans[i] = new UsbScanCodeEdit();
        layout->addWidget(editScans[i]);
    }

    //
    // Mouse
    //
    cbButton = new MouseButtonBox;
    layout->addWidget(cbButton);

    //
    // Advanced
    //
    spinIndex = new QSpinBox;
    spinIndex->setRange(0, 0xFF);
    spinIndex->setPrefix(tr("index  "));
    layout->addWidget(spinIndex);

    //
    // Command
    //
    editCommand = new UsbCommandEdit();
    editCommand->setMinimumWidth(measuredWidth * 2);
    layout->addWidget(editCommand);

    //
    // Macro
    //
    spinMacroIndex = new QSpinBox;
    spinMacroIndex->setRange(KB390L::MinMacroNum, KB390L::MaxMacroNum);
    layout->addWidget(spinMacroIndex);
    labelRepeat = new QLabel(tr("&play"));
    labelRepeat->setAlignment(Qt::AlignRight | Qt::AlignCenter);
    layout->addWidget(labelRepeat);
    cbRepeatMode = new QComboBox;
    cbRepeatMode->addItem(tr("once"));
    cbRepeatMode->addItem(tr("number of times"));
    cbRepeatMode->addItem(tr("until released"));
    cbRepeatMode->setEditable(false);
    layout->addWidget(cbRepeatMode);
    labelRepeat->setBuddy(cbRepeatMode);

    //
    // Expert mode
    //
    editCustom = new QLineEdit;
    editCustom->setInputMask("HH HH HH HH");
    editCustom->setMaximumWidth(measuredWidth);
    layout->addWidget(editCustom);
    layout->addStretch();
    setLayout(layout);
}

void ButtonEdit::setValue(int value)
{
    quint8 mode = 0xFF & value;
    quint8 arg1 = 0xFF & (value >> 8);
    quint8 arg2 = 0xFF & (value >> 16);
    quint8 arg3 = 0xFF & (value >> 24);
    setUpdatesEnabled(false);
    editCustom->setText(QString("%1 %2 %3 %4")
                            .arg(arg3, 2, 16, QChar('0'))
                            .arg(arg2, 2, 16, QChar('0'))
                            .arg(arg1, 2, 16, QChar('0'))
                            .arg(mode, 2, 16, QChar('0')));

    // Hide everything, will show some ot them later
    hideWidgets();

    switch (mode)
    {
    case KB390L::EventKey:
        editScans[0]->setValue(arg1);
        editScans[0]->setVisible(true);
        editScans[1]->setValue(arg2);
        editScans[1]->setVisible(true);
        editScans[2]->setValue(arg3);
        editScans[2]->setVisible(true);
        break;

    case KB390L::EventButton:
        cbButton->setValue(arg2);
        cbButton->setVisible(true);
        break;

    case KB390L::EventFunctionalKey:
        break;

    case KB390L::EventCommand:
        // Command is 16-bit wide
        editCommand->setValue(0xFFFF & (value >> 16));
        editCommand->setVisible(true);
        break;

    case KB390L::EventMacro:
        spinMacroIndex->setValue(arg2);
        spinMacroIndex->setVisible(true);
        labelRepeat->setVisible(true);
        cbRepeatMode->setCurrentIndex(arg1);
        cbRepeatMode->setVisible(true);
        break;

    case KB390L::EventAdvanced:
        spinIndex->setValue(arg2);
        spinIndex->setVisible(true);
        break;

    default:
        mode = KB390L::EventCustom;
        editCustom->setVisible(true);
        break;
    }

    auto block = cbMode->blockSignals(true);
    cbMode->setCurrentIndex(cbMode->findData(mode));
    cbMode->blockSignals(block);
    setUpdatesEnabled(true);
}

int ButtonEdit::value() const
{
    return extractValue(cbMode->currentData().toInt());
}

int ButtonEdit::extractValue(int mode) const
{
    quint8 arg1 = 0, arg2 = 0, arg3 = 0;

    switch (mode)
    {
    case KB390L::EventKey:
        arg1 = 0xFF & editScans[0]->value();
        arg2 = 0xFF & editScans[1]->value();
        arg3 = 0xFF & editScans[2]->value();
        break;

    case KB390L::EventButton:
        arg2 = 0xFF & cbButton->value();
        break;

    case KB390L::EventFunctionalKey:
        break;

    case KB390L::EventCommand:
    {
        int value = editCommand->value();
        arg2 = 0xFF & value;
        arg3 = 0xFF & (value >> 8);
    }
    break;

    case KB390L::EventMacro:
        arg2 = 0xFF & spinMacroIndex->value();
        arg1 = 0xFF & cbRepeatMode->currentIndex();
        break;

    case KB390L::EventAdvanced:
        arg2 = 0xFF & spinIndex->value();
        break;

    case KB390L::EventCustom:
        return editCustom->text().replace(" ", "").toInt(nullptr, 16);

    default:
        break;
    }

    return (arg3 << 24) | (arg2 << 16) | (arg1 << 8) | mode;
}

bool ButtonEdit::buttonEnabled() const
{
    return cbEnabled->isChecked();
}

void ButtonEdit::setButtonEnabled(bool value)
{
    cbEnabled->setChecked(value);
}

void ButtonEdit::onModeChanged(int idx)
{
    setUpdatesEnabled(false);
    auto newMode = cbMode->itemData(idx).toInt();
    auto oldMode = editCustom->text().right(2).toInt(nullptr, 16);
    int value = extractValue(oldMode) & ~0xFF;

    if (newMode == KB390L::EventKey && value == 0)
    {
        // Conversion from OFF to KEY
        value = 0x2C << 16;
    }

    hideWidgets();

    if (newMode == KB390L::EventCustom)
    {
        editCustom->setText(QString("%1 %2 %3 %4")
                                .arg(0xFF & value >> 24, 2, 16, QChar('0'))
                                .arg(0xFF & value >> 16, 2, 16, QChar('0'))
                                .arg(0xFF & value >> 8, 2, 16, QChar('0'))
                                .arg(oldMode, 2, 16, QChar('0')));
        editCustom->show();
    }
    else
    {
        setValue(value | newMode);
    }

    setUpdatesEnabled(true);
}

void ButtonEdit::hideWidgets()
{
    foreach (auto widget, findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly))
    {
        if (widget == cbEnabled || widget == cbMode)
            continue;

        widget->hide();
    }
}
