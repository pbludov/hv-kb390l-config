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

#include "mousebuttonbox.h"
#include "kb390l.h"

MouseButtonBox::MouseButtonBox(QWidget *parent)
    : QComboBox(parent)
{
    addItem(tr("Primary"), KB390L::MouseLeftButton);
    addItem(tr("Secondary"), KB390L::MouseRightButton);
    addItem(tr("Third"), KB390L::MouseMiddleButton);
    addItem(tr("Backward"), KB390L::MouseBackButton);
    addItem(tr("Forward"), KB390L::MouseForwardButton);
    addItem(tr("Scroll Left"), KB390L::WheelLeftButton);
    addItem(tr("Scroll Right"), KB390L::WheelRightButton);
    addItem(tr("Scroll Up"), KB390L::WheelUpButton);
    addItem(tr("Scroll Down"), KB390L::WheelDownButton);

    setEditable(false);
}

int MouseButtonBox::value() const
{
    return currentData().toInt();
}

void MouseButtonBox::setValue(int value)
{
    setCurrentIndex(findData(value));
}
