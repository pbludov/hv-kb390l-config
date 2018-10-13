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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "kb390l.h"

#include "buttonedit.h"
#include "pagelight.h"
#include "pagemacro.h"
#include "pagespeed.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QStyle>

static void initAction(QAction *action, QStyle::StandardPixmap icon, QKeySequence::StandardKey key)
{
    action->setIcon(qApp->style()->standardIcon(icon));
    action->setShortcut(key);
    action->setToolTip(action->shortcut().toString(QKeySequence::NativeText));
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , kb(new KB390L(this))
{
    ui->setupUi(this);
    // The Designer really lacs this functionality
    initAction(ui->actionExit, QStyle::SP_DialogCloseButton, QKeySequence::Quit);
    initAction(ui->actionSave, QStyle::SP_DialogSaveButton, QKeySequence::Save);

    ui->labelText->setText(ui->labelText->text().arg(PRODUCT_VERSION).arg(__DATE__));
    connect(kb, SIGNAL(connectChanged(bool)), this, SLOT(onkbConnected(bool)));
    connect(kb, SIGNAL(buttonsPressed(int)), this, SLOT(onButtonsPressed(int)));

    // Check the device availability
    onkbConnected(kb->ping());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updatekb()
{
    foreach (auto edit, findChildren<ButtonEdit *>())
    {
        auto index = KB390L::KeyIndex(edit->property("ButtonIndex").toInt());
        kb->setButton(index, edit->value());
        kb->setButtonEnabled(index, edit->buttonEnabled());
    }

    foreach (auto widget, findChildren<KbWidget *>())
    {
        widget->save(kb);
    }
}

void MainWindow::onSave()
{
    updatekb();

    if (!kb->save())
    {
        QMessageBox::warning(this, windowTitle(), tr("Failed to save"));
    }
}

void MainWindow::onkbConnected(bool connected)
{
    auto aboutIndex = ui->tabWidget->indexOf(ui->pageAbout);
    if (!connected)
        ui->tabWidget->setCurrentIndex(aboutIndex);

    for (int i = 0; i < ui->tabWidget->count(); ++i)
    {
        if (aboutIndex == i)
            continue;
        ui->tabWidget->setTabEnabled(i, connected);
    }

    ui->actionSave->setEnabled(connected);
}

static std::pair<QString, KB390L::KeyIndex> buttons[][KB390L::ButtonsPerRow] =
{
    {
        {QCoreApplication::translate("key", "&Esc"), KB390L::KeyEsc},

        {QCoreApplication::translate("key", "F&1"),  KB390L::KeyF1},
        {QCoreApplication::translate("key", "F&2"),  KB390L::KeyF2},
        {QCoreApplication::translate("key", "F&3"),  KB390L::KeyF3},
        {QCoreApplication::translate("key", "F&4"),  KB390L::KeyF4},

        {QCoreApplication::translate("key", "F&5"),  KB390L::KeyF5},
        {QCoreApplication::translate("key", "F&6"),  KB390L::KeyF6},
        {QCoreApplication::translate("key", "F&7"),  KB390L::KeyF7},
        {QCoreApplication::translate("key", "F&8"),  KB390L::KeyF8},

        {QCoreApplication::translate("key", "F&9"),  KB390L::KeyF9},
        {QCoreApplication::translate("key", "F1&0"), KB390L::KeyF10},
        {QCoreApplication::translate("key", "F11"),  KB390L::KeyF11},
        {QCoreApplication::translate("key", "F12"),  KB390L::KeyF12},

        {QCoreApplication::translate("key", "&SysRq"),     KB390L::KeySysRq},
        {QCoreApplication::translate("key", "Scrl &Lock"), KB390L::KeyScrollLock},
        {QCoreApplication::translate("key", "&Pause"),     KB390L::KeyPause},
    },
    {
        {QCoreApplication::translate("key", "&`"),  KB390L::KeyTilde},
        {QCoreApplication::translate("key", "&1"),  KB390L::Key1},
        {QCoreApplication::translate("key", "&2"),  KB390L::Key2},
        {QCoreApplication::translate("key", "&3"),  KB390L::Key3},
        {QCoreApplication::translate("key", "&4"),  KB390L::Key4},
        {QCoreApplication::translate("key", "&5"),  KB390L::Key5},
        {QCoreApplication::translate("key", "&6"),  KB390L::Key6},
        {QCoreApplication::translate("key", "&7"),  KB390L::Key7},
        {QCoreApplication::translate("key", "&8"),  KB390L::Key8},
        {QCoreApplication::translate("key", "&9"),  KB390L::Key9},
        {QCoreApplication::translate("key", "&0"),  KB390L::Key0},
        {QCoreApplication::translate("key", "&-"),  KB390L::KeyMinus},
        {QCoreApplication::translate("key", "&="),  KB390L::KeyPlus},
        {QCoreApplication::translate("key", "&Backspace"),  KB390L::KeyBackspace},

        {QCoreApplication::translate("key", "&Insert"), KB390L::KeyInsert},
        {QCoreApplication::translate("key", "&Home"),   KB390L::KeyHome},
        {QCoreApplication::translate("key", "Pg&Up"),   KB390L::KeyPgUp},
    },
    {
        {QCoreApplication::translate("key", "&Tab"), KB390L::KeyTab},
        {QCoreApplication::translate("key", "&Q"),   KB390L::KeyQ},
        {QCoreApplication::translate("key", "&W"),   KB390L::KeyW},
        {QCoreApplication::translate("key", "&E"),   KB390L::KeyE},
        {QCoreApplication::translate("key", "&R"),   KB390L::KeyR},
        {QCoreApplication::translate("key", "&T"),   KB390L::KeyT},
        {QCoreApplication::translate("key", "&Y"),   KB390L::KeyY},
        {QCoreApplication::translate("key", "&U"),   KB390L::KeyU},
        {QCoreApplication::translate("key", "&I"),   KB390L::KeyI},
        {QCoreApplication::translate("key", "&O"),   KB390L::KeyO},
        {QCoreApplication::translate("key", "&P"),   KB390L::KeyP},
        {QCoreApplication::translate("key", "&["),   KB390L::KeyLParen},
        {QCoreApplication::translate("key", "&]"),   KB390L::KeyRParen},
        {QCoreApplication::translate("key", "&\\"),  KB390L::KeyBackSlash},

        {QCoreApplication::translate("key", "&Del"),  KB390L::KeyDel},
        {QCoreApplication::translate("key", "&End"),  KB390L::KeyEnd},
        {QCoreApplication::translate("key", "P&gDn"), KB390L::KeyPgDn},
    },
    {
        {QCoreApplication::translate("key", "&Caps Lock"), KB390L::KeyCapsLock},
        {QCoreApplication::translate("key", "&A"), KB390L::KeyA},
        {QCoreApplication::translate("key", "&S"), KB390L::KeyS},
        {QCoreApplication::translate("key", "&D"), KB390L::KeyD},
        {QCoreApplication::translate("key", "&F"), KB390L::KeyF},
        {QCoreApplication::translate("key", "&G"), KB390L::KeyG},
        {QCoreApplication::translate("key", "&H"), KB390L::KeyH},
        {QCoreApplication::translate("key", "&J"), KB390L::KeyJ},
        {QCoreApplication::translate("key", "&K"), KB390L::KeyK},
        {QCoreApplication::translate("key", "&L"), KB390L::KeyL},
        {QCoreApplication::translate("key", "&;"), KB390L::KeySemicolon},
        {QCoreApplication::translate("key", "&'"), KB390L::KeyQuote},
        {QCoreApplication::translate("key", "&Enter"), KB390L::KeyEnter},
    },
    {
        {QCoreApplication::translate("key", "&Left Shift"), KB390L::KeyLShift},
        {QCoreApplication::translate("key", "&Z"), KB390L::KeyZ},
        {QCoreApplication::translate("key", "&X"), KB390L::KeyX},
        {QCoreApplication::translate("key", "&C"), KB390L::KeyC},
        {QCoreApplication::translate("key", "&V"), KB390L::KeyV},
        {QCoreApplication::translate("key", "&B"), KB390L::KeyB},
        {QCoreApplication::translate("key", "&N"), KB390L::KeyN},
        {QCoreApplication::translate("key", "&M"), KB390L::KeyM},
        {QCoreApplication::translate("key", "&,"), KB390L::KeyComma},
        {QCoreApplication::translate("key", "&Slash"), KB390L::KeySlash},
        {QCoreApplication::translate("key", "&Right Shift"), KB390L::KeyRShift},

        {QCoreApplication::translate("key", "&Up"), KB390L::KeyUp},
    },
    {
        {QCoreApplication::translate("key", "Left &Ctrl"), KB390L::KeyLCtrl},
        {QCoreApplication::translate("key", "&Super"), KB390L::KeySuper},
        {QCoreApplication::translate("key", "Left &Alt"), KB390L::KeyLAlt},
        {QCoreApplication::translate("key", "Space &Bar"), KB390L::KeySpace},
        {QCoreApplication::translate("key", "R&ight Alt"), KB390L::KeyRAlt},
        {QCoreApplication::translate("key", "&Fn"), KB390L::KeyFn},
        {QCoreApplication::translate("key", "&Menu"), KB390L::KeyMenu},
        {QCoreApplication::translate("key", "Ri&ght Ctrl"), KB390L::KeyRCtrl},

        {QCoreApplication::translate("key", "&Left"), KB390L::KeyLeft},
        {QCoreApplication::translate("key", "&Down"), KB390L::KeyDown},
        {QCoreApplication::translate("key", "&Right"), KB390L::KeyRight},
    }
};

static bool prepareButtonsPage(
    QWidget *parent, KB390L *kb, const std::pair<QString, KB390L::KeyIndex> *buttons)
{
    auto layout = new QVBoxLayout;

    for (size_t i = 0; i < KB390L::ButtonsPerRow; ++i)
    {
        if (buttons[i].first.isNull())
            break;

        auto value = kb->button(buttons[i].second);
        if (value == -1)
        {
            delete layout;
            return false;
        }
        auto edit = new ButtonEdit(buttons[i].first);
        edit->setProperty("ButtonIndex", buttons[i].second);
        edit->setValue(value);
        edit->setButtonEnabled(kb->buttonEnabled(buttons[i].second));
        layout->addWidget(edit);
    }

    layout->addStretch();
    parent->setLayout(layout);
    return true;
}

bool MainWindow::initPage(QWidget *parent, KbWidget *page)
{
    if (!page->load(kb))
    {
        delete page;
        return false;
    }

    auto layout = new QVBoxLayout;
    parent->setLayout(layout);
    layout->addWidget(page);
    return true;
}

void MainWindow::onPreparePage(int idx)
{
    auto page = ui->tabWidget->widget(idx);

    if (page->layout())
    {
        // Already prepared
        return;
    }

    bool ok = true;
    if (page == ui->pageButtons1)
        ok = prepareButtonsPage(page, kb, buttons[0]);
    else if (page == ui->pageButtons2)
        ok = prepareButtonsPage(page, kb, buttons[1]);
    else if (page == ui->pageButtons3)
        ok = prepareButtonsPage(page, kb, buttons[2]);
    else if (page == ui->pageButtons4)
        ok = prepareButtonsPage(page, kb, buttons[3]);
    else if (page == ui->pageButtons5)
        ok = prepareButtonsPage(page, kb, buttons[4]);
    else if (page == ui->pageButtons6)
        ok = prepareButtonsPage(page, kb, buttons[5]);
    else if (page == ui->pageMacros)
        ok = initPage(page, new PageMacro());
    else if (page == ui->pageSpeed)
        ok = initPage(page, new PageSpeed());
    else if (page == ui->pageLight)
    {
        auto pageLight = new PageLight();
        ok = initPage(page, pageLight);
        connect(kb, SIGNAL(changed(KB390L*)), pageLight, SLOT(onKbChanged(KB390L*)));
    }

    if (!ok)
    {
        QMessageBox::warning(this, windowTitle(), tr("Failed to read keyboard NAND"));
    }
}

void MainWindow::closeEvent(QCloseEvent *evt)
{
    updatekb();

    if (kb->unsavedChanges()
        && QMessageBox::question(this, windowTitle(), tr("You have unsaved changes.\nSave them now?"))
               == QMessageBox::Yes)
    {
        if (!kb->save())
        {
            QMessageBox::warning(this, windowTitle(), tr("Failed to save"));
            evt->ignore();
            return;
        }
    }

    QMainWindow::closeEvent(evt);
}
