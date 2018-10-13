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

#ifndef MACROEDIT_H
#define MACROEDIT_H

#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QComboBox)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QSpinBox)

class MacroEdit : public QWidget
{
    Q_PROPERTY(ActionType actionType READ actionType)
    Q_PROPERTY(int delay READ delay WRITE setDelay)
    Q_PROPERTY(int value READ value WRITE setValue)

    Q_OBJECT

public:
    enum ActionType
    {
        ActionKey = 0x01,
        ActionButton = 0x02,
        ActionFlagDown = 0x10,
        ActionFlagUp = 0x20,

        ActionKeyDown = ActionKey | ActionFlagDown,
        ActionKeyUp = ActionKey | ActionFlagUp,
        ActionKeyPress = ActionKey | ActionFlagDown | ActionFlagUp,

        ActionButtonDown = ActionButton | ActionFlagDown,
        ActionButtonUp = ActionButton | ActionFlagUp,
        ActionButtonClick = ActionButton | ActionFlagDown | ActionFlagUp,
    };

    explicit MacroEdit(ActionType actionType, QWidget *parent = 0);

    ActionType actionType() const;

    int delay() const;
    void setDelay(int value);

    int value() const;
    void setValue(int value);

public slots:
    void moveUp();
    void moveDown();

private:
    ActionType actionTypeValue;

    QLabel *title;
    class EnumEdit *key;
    class MouseButtonBox *button;
    QSpinBox *spinDelay;
};

#endif // MACROEDIT_H
