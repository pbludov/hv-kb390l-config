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

#include "pagemacro.h"
#include "ui_pagemacro.h"

#include "macroedit.h"
#include "kb390l.h"

#include <QMessageBox>

PageMacro::PageMacro(QWidget *parent)
    : KbWidget(parent)
    , ui(new Ui::PageMacro)
    , kb(nullptr)
{
    ui->setupUi(this);

    auto cb = ui->cbAddAction;
    cb->addItem(tr("<add action>"));
    cb->addItem(tr("Key Press"), MacroEdit::ActionKeyPress);
    cb->addItem(tr("Key Down"), MacroEdit::ActionKeyDown);
    cb->addItem(tr("Key Up"), MacroEdit::ActionKeyUp);
    cb->addItem(tr("Button Click"), MacroEdit::ActionButtonClick);
    cb->addItem(tr("Button Down"), MacroEdit::ActionButtonDown);
    cb->addItem(tr("Button Up"), MacroEdit::ActionButtonUp);

    for (int i = KB390L::MinMacroNum; i <= KB390L::MaxMacroNum; ++i)
    {
        auto item = new QListWidgetItem(tr("Macro %1").arg(i, 3, 10, QChar('0')));
        item->setData(QListWidgetItem::UserType, i);
        ui->listMacroIndex->addItem(item);
    }
}

PageMacro::~PageMacro()
{
    delete ui;
}

bool PageMacro::load(KB390L *kb)
{
    this->kb = kb;
    auto macro = kb->macro(KB390L::MinMacroNum);
    if (macro.isNull())
        return false;

    setMacro(macro);
    auto block = ui->listMacroIndex->blockSignals(true);
    ui->listMacroIndex->setCurrentRow(0);
    ui->listMacroIndex->blockSignals(block);
    return true;
}

void PageMacro::save(KB390L *kb)
{
    auto currItem = ui->listMacroIndex->currentItem();
    if (currItem)
    {
        auto idx = currItem->data(QListWidgetItem::UserType).toInt();
        kb->setMacro(idx, macro());
    }
}

void PageMacro::selectMacro(QListWidgetItem *current, QListWidgetItem *previous)
{
    setUpdatesEnabled(false);

    if (previous)
    {
        auto prevIndex = previous->data(QListWidgetItem::UserType).toInt();
        kb->setMacro(prevIndex, macro());

        // Remove current controls
        foreach (auto edit, ui->scrollAreaWidgetContents->findChildren<MacroEdit *>())
        {
            delete edit;
        }
    }

    if (current)
    {
        auto macroIndex = current->data(QListWidgetItem::UserType).toInt();
        auto macro = kb->macro(macroIndex);
        if (macro.isNull())
        {
            QMessageBox::warning(this, windowTitle(), tr("Failed to load macro %1").arg(macroIndex, 3, 10, QChar('0')));

            // Unselect macro to prevent data loss
            ui->cbAddAction->setFocus();
            ui->listMacroIndex->setCurrentRow(-1);
        }
        else
        {
            setMacro(macro);
        }
    }

    setUpdatesEnabled(true);
}

QByteArray PageMacro::macro() const
{
    QByteArray macro;
    auto count = ui->repeat->value();
    macro.append(0xFF & (count >> 8)).append(0xFF & count);

    foreach (auto edit, ui->scrollAreaWidgetContents->findChildren<MacroEdit *>())
    {
        auto value = edit->value();
        if (value == 0)
        {
            // Unassigned key, skip it
            continue;
        }

        auto delay = edit->delay();
        auto type = edit->actionType();
        int extraDelay = 0;

        if (delay >= 127)
        {
            extraDelay = delay;
            delay = 127;
        }

        switch (type & (MacroEdit::ActionFlagDown | MacroEdit::ActionFlagUp))
        {
        case MacroEdit::ActionFlagDown:
            macro.append(delay).append(value);
            break;
        case MacroEdit::ActionFlagDown | MacroEdit::ActionFlagUp:
            macro.append(1).append(value).append(delay | 0x80).append(value);
            break;
        case MacroEdit::ActionFlagUp:
            macro.append(delay | 0x80).append(value);
            break;
        }

        if (extraDelay)
        {
            macro.append(0xFF & (extraDelay >> 8)).append(0xFF & extraDelay);
        }
    }

    // Add extra zeros to mark the end of macro
    return macro.append("\x0\x0", 192 - macro.size());
}

void PageMacro::setMacro(QByteArray &macro)
{
    // Add some zeros to not bother about boundaries
    macro.append("\x0\x0\x0\x0", 4);

    auto count = (quint8)macro.at(0) << 8 | (quint8)macro.at(1);
    if (count == 0 || count == 0xFFFF)
    {
        // empty macro
        return;
    }
    ui->repeat->setValue(count);

    for (int i = 2; i < macro.length(); i += 2)
    {
        int delay = 0xFF & macro.at(i);
        int value = 0xFF & macro.at(i + 1);

        if (value == 0)
        {
            // End of macro
            break;
        }

        int type = value < KB390L::MouseLeftButton ? MacroEdit::ActionKey : MacroEdit::ActionButton;

        if (delay & 0x80)
        {
            // Key/button up event
            type |= MacroEdit::ActionFlagUp;
            delay &= ~0x80;
        }
        else if (macro.at(i + 3) == (char)value && (0x80 & macro.at(i + 2)) && delay == 1)
        {
            // Down, then up => key press / button click
            type |= MacroEdit::ActionFlagDown | MacroEdit::ActionFlagUp;
            delay = 0x7F & macro.at(i + 2);
            i += 2;
        }
        else
        {
            // Standalone key/button down
            type |= MacroEdit::ActionFlagDown;
        }

        // Check for extended delay
        if (delay == 0x7F)
        {
            delay = (quint8)macro.at(i + 2) << 8 | (quint8)macro.at(i + 3);
            i += 2;
        }

        auto edit = new MacroEdit((MacroEdit::ActionType)type);
        ui->scrollAreaWidgetLayout->insertWidget(ui->scrollAreaWidgetLayout->count() - 2, edit);
        edit->setValue(value);
        edit->setDelay(delay);
        edit->setVisible(true);
    }
}

void PageMacro::addAction(int idx)
{
    auto type = (MacroEdit::ActionType)ui->cbAddAction->itemData(idx).toInt();

    if (type == 0)
    {
        // user canceled
        return;
    }

    setUpdatesEnabled(false);
    auto edit = new MacroEdit(type);
    ui->scrollAreaWidgetLayout->insertWidget(ui->scrollAreaWidgetLayout->count() - 2, edit);
    edit->setVisible(true);

    // Revert to "(add)"
    ui->cbAddAction->setCurrentIndex(0);

    // Make sure the content is updated
    qApp->processEvents();

    // Keep (add) combobox visible
    ui->scrollArea->ensureWidgetVisible(ui->cbAddAction);
    edit->setFocus();
    setUpdatesEnabled(true);
}
