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

#include "macroedit.h"
#include "usbscancodeedit.h"
#include "mousebuttonbox.h"
#include "kb390l.h"

#include <QAction>
#include <QComboBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QSpinBox>
#include <QStyle>

MacroEdit::MacroEdit(ActionType actionType, QWidget *parent)
    : QWidget(parent)
    , actionTypeValue(actionType)
    , key(nullptr)
    , button(nullptr)
{
    auto measuredWidth = fontMetrics().width("123456789012");
    auto layout = new QHBoxLayout;
    layout->setMargin(0);

    title = new QLabel;
    title->setMinimumWidth(measuredWidth);
    layout->addWidget(title);

    if (actionType & ActionKey)
    {
        key = new UsbScanCodeEdit();
        key->setMinimumWidth(measuredWidth * 2);
        layout->addWidget(key);
        setFocusProxy(key);
        title->setBuddy(key);

        switch (actionType & (ActionFlagDown | ActionFlagUp))
        {
        case ActionFlagDown | ActionFlagUp:
            title->setText(tr("Key &Press"));
            break;

        case ActionFlagUp:
            title->setText(tr("K&ey Up"));
            break;

        case ActionFlagDown:
            title->setText(tr("Ke&y Down"));
            break;
        }
    }

    if (actionType & ActionButton)
    {
        button = new MouseButtonBox;
        button->setMinimumWidth(measuredWidth * 2);
        layout->addWidget(button);
        setFocusProxy(button);
        title->setBuddy(button);

        switch (actionType & (ActionFlagDown | ActionFlagUp))
        {
        case ActionFlagDown | ActionFlagUp:
            title->setText(tr("Button &Click"));
            break;

        case ActionFlagUp:
            title->setText(tr("Button &Up"));
            break;

        case ActionFlagDown:
            title->setText(tr("Button &Down"));
            break;
        }
    }

    spinDelay = new QSpinBox;
    spinDelay->setPrefix(tr("delay  "));
    spinDelay->setSuffix(tr("  msec"));
    spinDelay->setMaximum(6553599); // Almost 2 hour! OMG.
    spinDelay->setSingleStep(10);
    spinDelay->setValue(10);
    layout->addWidget(spinDelay);
    layout->addStretch();

    // UI buttons
    auto moveDown = new QPushButton(style()->standardIcon(QStyle::SP_ArrowDown), QString());
    moveDown->setToolTip(tr("Move down"));
    connect(moveDown, SIGNAL(clicked(bool)), this, SLOT(moveDown()));
    moveDown->setFlat(true);
    layout->addWidget(moveDown);

    auto moveUp = new QPushButton(style()->standardIcon(QStyle::SP_ArrowUp), QString());
    moveUp->setToolTip(tr("Move up"));
    connect(moveUp, SIGNAL(clicked(bool)), this, SLOT(moveUp()));
    moveUp->setFlat(true);
    layout->addWidget(moveUp);

    auto remove = new QPushButton(style()->standardIcon(QStyle::SP_DialogDiscardButton), QString());
    remove->setToolTip(tr("Remove"));
    connect(remove, SIGNAL(clicked(bool)), this, SLOT(deleteLater()));
    remove->setFlat(true);
    layout->addWidget(remove);

    setLayout(layout);
}

MacroEdit::ActionType MacroEdit::actionType() const
{
    return actionTypeValue;
}

void MacroEdit::setValue(int value)
{
    if (key)
    {
        key->setValue(value);
    }
    if (button)
    {
        button->setValue(value);
    }
}

int MacroEdit::value() const
{
    return key ? key->value() : button ? button->value() : -1;
}

void MacroEdit::setDelay(int value)
{
    spinDelay->setValue(value);
}

int MacroEdit::delay() const
{
    return spinDelay->value();
}

void MacroEdit::moveUp()
{
    auto parentLayout = static_cast<QBoxLayout *>(parentWidget()->layout());
    auto idx = parentLayout->indexOf(this);

    if (idx > 0)
    {
        parentLayout->takeAt(idx);
        parentLayout->insertWidget(--idx, this);
    }
}

void MacroEdit::moveDown()
{
    auto parentLayout = static_cast<QBoxLayout *>(parentWidget()->layout());
    auto idx = parentLayout->indexOf(this);

    if (idx < parentLayout->count() - 3)
    {
        parentLayout->takeAt(idx);
        parentLayout->insertWidget(++idx, this);
    }
}
