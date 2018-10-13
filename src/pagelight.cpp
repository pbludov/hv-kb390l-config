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

#include "pagelight.h"
#include "ui_pagelight.h"
#include "kb390l.h"

PageLight::PageLight(QWidget *parent)
    : KbWidget(parent)
    , ui(new Ui::PageLight)
{
    ui->setupUi(this);

    ui->cbType->addItems(QStringList()
                         << tr("Static") << tr("Breath") << tr("Wave")
                         << tr("Reactive") << tr("Sidewinder") << tr("Ripple")
                         << tr("Alt Reactive") << tr("Alt Wave") << tr("Alt Sidewinder")
                         << tr("Raindrop") << tr("Wortex") << tr("Spotlight") << tr("Radar")
                         << tr("Running water") << tr("Running mold") << tr("Shade")
                         << tr("Mask1") << tr("Mask2") << tr("Mask3") << tr("Mask4") << tr("Mask5")
                         );
    ui->cbDirection->addItems(QStringList()
        << tr("Right") << tr("Left") << tr("Up") << tr("Down"));
}

PageLight::~PageLight()
{
    delete ui;
}

bool PageLight::load(KB390L *kb)
{
    auto type = kb->lightType();
    auto delay = kb->LightDelay();
    auto brightness = kb->lightBrightness();
    auto direction = kb->lightDirection();
    if (type < 0 || delay < 0 || brightness < 0 || direction < 0)
        return false;

    if (type >= KB390L::LightMask1)
        type -= 34;

    ui->cbType->setCurrentIndex(type - 1);
    ui->cbDirection->setCurrentIndex(direction - 1);
    ui->sliderDelay->setValue(delay);
    ui->sliderBrightness->setValue(brightness);

    return true;
}

void PageLight::save(KB390L *kb)
{
    auto type = ui->cbType->currentIndex() + 1;
    if (type > KB390L::LightShadeMold)
        type += 34;
    kb->setLightType(type);
    kb->setLightDirection(ui->cbDirection->currentIndex() + 1);
    kb->setLightDelay(ui->sliderDelay->value());
    kb->setLightBrightness(ui->sliderBrightness->value());
}

void PageLight::onLightTypeChanged(int value)
{
    bool delay = value != KB390L::LightStatic && value < KB390L::LightMask1;
    bool direction = value == KB390L::LightWave || value == KB390L::LightRadar
            || value == KB390L::LightRunningMold|| value == KB390L::LightShadeMold;

    ui->sliderDelay->setVisible(delay);
    ui->labelDelay->setVisible(delay);
    ui->cbDirection->setVisible(direction);
    ui->labelDirection->setVisible(direction);
}

void PageLight::onKbChanged(KB390L *kb)
{
    load(kb);
}
