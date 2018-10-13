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
#include "kb390l.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QFile>
#include <QThread>

inline QString tr(const char *str)
{
    return QCoreApplication::translate("main", str);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(PRODUCT_NAME);
    QCoreApplication::setApplicationVersion(PRODUCT_VERSION);
    app.setWindowIcon(QIcon(":/app/icon"));

    QCommandLineParser parser;
    parser.setApplicationDescription(tr("HV-KB390L configuration application"));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption gameModeOption(QStringList() << "g" << "game-mode", tr("Get the game mode."));
    parser.addOption(gameModeOption);
    QCommandLineOption setGameModeOption(QStringList() << "G" << "set-game-mode", tr("Set the game mode: <on|off>."), tr("mode"));
    parser.addOption(setGameModeOption);
    QCommandLineOption rateOption(QStringList() << "r" << "rate", tr("Get the report <rate>: {0..3}."));
    parser.addOption(rateOption);
    QCommandLineOption setRateOption(QStringList() << "R" << "set-rate", tr("Select the report <rate>."), tr("rate"));
    parser.addOption(setRateOption);
    QCommandLineOption responseTimeOption(QStringList() << "t" << "response-time", tr("Get the response time <msecs>: {2..20}."));
    parser.addOption(responseTimeOption);
    QCommandLineOption setResponseTimeOption(QStringList() << "T" << "set-response-time", tr("Select the response time <msecs>."), tr("msecs"));
    parser.addOption(setResponseTimeOption);
    QCommandLineOption backupOption(QStringList() << "backup", tr("Backup NAND data to a <file>."), tr("file"));
    parser.addOption(backupOption);
    QCommandLineOption resetOption(QStringList() << "reset", tr("Reset the device to the factory settings."));
    parser.addOption(resetOption);
    QCommandLineOption restoreOption(QStringList() << "restore", tr("Restore NAND data from a <file>."), tr("file"));
    parser.addOption(restoreOption);
    QCommandLineOption verboseOption(QStringList() << "verbose", tr("Verbose output."));
    parser.addOption(verboseOption);

    // Process the actual command line arguments given by the user.
    parser.process(app);

    if (!parser.isSet(verboseOption))
    {
        QLoggingCategory::setFilterRules("*.debug=false");
    }

    auto optionsNames = parser.optionNames();
    optionsNames.removeAll("verbose");

    if (optionsNames.isEmpty())
    {
        MainWindow w;
        w.show();
        return app.exec();
    }

    KB390L kb;

    // For any other command line option we need the device, so check it in advance.
    if (!kb.ping())
    {
        qWarning() << "The device was not found.";
        return 1;
    }

    if (parser.isSet(backupOption))
    {
        qWarning() << "Reading config from the device...";

        QFile file(parser.value(backupOption));

        if (!file.open(QFile::WriteOnly))
        {
            qWarning() << "Failed to open" << file.fileName() << "for writing.";
            return 2;
        }

        if (!kb.backupConfig(&file))
        {
            qWarning() << "Failed to read the config.";
            return 3;
        }

        qWarning() << "The config has been successfully read from the device and written to" << file.fileName();
        return 0;
    }

    if (parser.isSet(restoreOption))
    {
        qWarning() << "Writing config to the device...";

        QFile file(parser.value(restoreOption));

        if (!file.open(QFile::ReadOnly))
        {
            qWarning() << "Failed to open" << file.fileName() << "for reading.";
            return 2;
        }

        if (!kb.restoreConfig(&file))
        {
            qWarning() << "Failed to write the config.";
            return 3;
        }

        qWarning() << "The config has been successfully read from " << file.fileName() << " and written to the device";
        return 0;
    }

    if (parser.isSet(resetOption))
    {
        kb.resetToFactoryDefaults();
        return 0;
    }

    if (parser.isSet(setGameModeOption))
    {
        kb.setGameMode(!!parser.value(setGameModeOption).compare("off", Qt::CaseInsensitive));
        return 0;
    }

    if (parser.isSet(gameModeOption))
    {
        auto mode = kb.gameMode();
        qWarning() << (mode == 1 ? "on" : mode == 0 ? "off" : "error");
        return 0;
    }

    if (parser.isSet(rateOption))
    {
        qWarning() << kb.reportRate();
        return 0;
    }

    if (parser.isSet(setRateOption))
    {
        kb.setReportRate(parser.value(setRateOption).toInt());
        return 0;
    }

    if (parser.isSet(responseTimeOption))
    {
        qWarning() << kb.responseTime() * 2;
        return 0;
    }

    if (parser.isSet(setResponseTimeOption))
    {
        kb.setResponseTime(parser.value(setResponseTimeOption).toInt() / 2);
        return 0;
    }

    // Should never happen.
    qCritical() << "Unhandled options: " << parser.optionNames();
    return 0;
}
