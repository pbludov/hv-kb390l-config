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

#ifndef ENUMEDIT_H
#define ENUMEDIT_H

#include <QLineEdit>

QT_FORWARD_DECLARE_CLASS(QMenu)

class EnumEdit : public QLineEdit
{
    Q_PROPERTY(int value READ value WRITE setValue)
    Q_OBJECT

public:
    struct Item
    {
        const char *name;
        int group;
    };

    explicit EnumEdit(const std::vector<Item> &items, const std::vector<Item> &groups, QWidget *parent = 0);

    int value() const;
    void setValue(const int value);

private slots:
    void onDropDownAction();
    void prepareSubMenu();

private:
    QMenu *menu;

    std::vector<Item> items;
    std::vector<Item> groups;
};

#endif // ENUMEDIT_H
