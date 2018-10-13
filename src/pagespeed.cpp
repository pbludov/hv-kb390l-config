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

#include "pagespeed.h"
#include "ui_pagespeed.h"
#include "kb390l.h"

PageSpeed::PageSpeed(QWidget *parent)
    : KbWidget(parent)
    , ui(new Ui::PageSpeed)
{
    ui->setupUi(this);

    ui->labelResponseTime->setMinimumWidth(fontMetrics().width(tr("Response time XXms")));
    ui->labelReportRate->setMinimumWidth(fontMetrics().width(tr("Report rate XXXXHz")));
}

PageSpeed::~PageSpeed()
{
    delete ui;
}

bool PageSpeed::load(KB390L *kb)
{
    auto responseTime = kb->responseTime();
    auto gameMode = kb->gameMode();
    auto reportRate = kb->reportRate();
    if (responseTime < 0 || gameMode < 0 || reportRate < 0)
        return false;

    ui->sliderResponseTime->setValue(responseTime);
    onResponseTimeChanged(responseTime);
    ui->checkGameMode->setChecked(gameMode != 0);
    ui->sliderReportRate->setValue(reportRate);
    onReportRateChanged(reportRate);
    return true;
}

void PageSpeed::save(KB390L *kb)
{
    kb->setResponseTime(ui->sliderResponseTime->value());
    kb->setGameMode(ui->checkGameMode->isChecked());
    kb->setReportRate(ui->sliderReportRate->value());
}

void PageSpeed::onResponseTimeChanged(int value)
{
    ui->labelResponseTime->setText(tr("Response &time %1ms").arg(value * 2, 2));
}

void PageSpeed::onReportRateChanged(int value)
{
    ui->labelReportRate->setText(tr("&Report rate %1Hz").arg((1 << value) * 125));
}
