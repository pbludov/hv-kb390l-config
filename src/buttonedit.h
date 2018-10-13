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

#ifndef BUTTONEDIT_H
#define BUTTONEDIT_H

#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QCheckBox)
QT_FORWARD_DECLARE_CLASS(QComboBox)
QT_FORWARD_DECLARE_CLASS(QLineEdit)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QSpinBox)

class ButtonEdit : public QWidget
{
    Q_PROPERTY(int value READ value WRITE setValue)
    Q_PROPERTY(bool buttonEnabled READ buttonEnabled WRITE setButtonEnabled)

    Q_OBJECT

public:
    explicit ButtonEdit(QString labelText, QWidget *parent = nullptr);

    int value() const;
    void setValue(int value);

    bool buttonEnabled() const;
    void setButtonEnabled(bool value);

public slots:
    void onModeChanged(int idx);

private:
    int extractValue(int mode) const;
    void hideWidgets();

    QCheckBox *cbEnabled;
    QComboBox *cbMode;

    // Key (0)
    class EnumEdit *editScans[3];

    // Mouse (1)
    class MouseButtonBox *cbButton;

    // Command (3)
    class EnumEdit *editCommand;

    // Macro (4)
    QLabel *labelRepeat;
    QSpinBox *spinMacroIndex;
    QComboBox *cbRepeatMode;

    // Advanced (10)
    QSpinBox *spinIndex;

    // Expert mode
    QLineEdit *editCustom;
};

#endif // BUTTONEDIT_H
