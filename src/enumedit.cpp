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

#include "enumedit.h"

#include <QAction>
#include <QCompleter>
#include <QDebug>
#include <QMenu>
#include <QStyle>
#include <QValidator>

EnumEdit::EnumEdit(const std::vector<Item> &items, const std::vector<Item> &groups, QWidget *parent)
    : QLineEdit(parent)
    , menu(nullptr)
    , items(items)
    , groups(groups)
{
    auto chooseAction = new QAction(style()->standardIcon(QStyle::SP_ArrowRight), tr("choose"), this);
    addAction(chooseAction, LeadingPosition);
    connect(chooseAction, SIGNAL(triggered()), this, SLOT(onDropDownAction()));

    // Init completer
    QStringList strList;
    foreach (auto item, items)
    {
        if (*item.name)
            strList << item.name;
    }

    auto completer = new QCompleter(strList, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    setCompleter(completer);
}

void EnumEdit::setValue(const int value)
{
    if (value <= 0)
    {
        // No value
        setText("");
    }
    else if (value >= 0 && value < (int)items.size() && items[value].name[0])
    {
        // Known value
        setText(items[value].name);
    }
    else
    {
        // Unknown value, display as four byte hex (to avoid a possible collision with F1..F9)
        setText(QString("%1").arg(value, 4, 16, QChar('0')));
    }
}

int EnumEdit::value() const
{
    auto name = text();
    if (name.isEmpty())
        return 0;

    auto lambda = [name](const Item &i) { return name.compare(tr(i.name), Qt::CaseInsensitive) == 0; };
    auto item = std::find_if(items.cbegin(), items.cend(), lambda);

    if (item != items.cend())
        return item - items.cbegin();

    // Not found by name, treat as a hex sequence.
    return name.replace(" ", "").toInt(nullptr, 16);
}

void EnumEdit::onDropDownAction()
{
    // If the menu is not created already, fill it with groups. Items will be added later.
    if (!menu)
    {
        menu = new QMenu(this);
        foreach (auto group, groups)
        {
            auto subMenu = menu->addMenu(tr(group.name));
            connect(subMenu, SIGNAL(aboutToShow()), this, SLOT(prepareSubMenu()));
        }
    }

    auto pt = parentWidget()->mapToGlobal(geometry().bottomLeft());
    auto userSelected = menu->exec(pt);

    if (userSelected)
    {
        setText(userSelected->text());
    }
}

void EnumEdit::prepareSubMenu()
{
    auto subMenu = static_cast<QMenu *>(QObject::sender()->qt_metacast("QMenu"));
    if (!subMenu->isEmpty())
        return;

    auto lambda = [subMenu](const Item &g) { return subMenu->title().compare(tr(g.name)) == 0; };
    auto group = std::find_if(groups.cbegin(), groups.cend(), lambda);
    if (group == groups.cend())
        return;

    int idx = 0;
    foreach (auto item, items)
    {
        if (item.group == group->group)
        {
            subMenu->addAction(item.name)->setData(idx);
        }
        ++idx;
    }
}
