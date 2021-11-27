/***************************************************************************
 *   Copyright © 2010 Jonathan Thomas <echidnaman@kubuntu.org>             *
 *               2021 Bob <pengbo·wu@jingos.com>                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "qapttest.h"
#include "UpgradeModel.h"
#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <KAboutData>
#include <KPluginLoader>
#include <KPluginMetaData>
#include <QObject>
#include <QQmlContext>
#include <QQmlApplicationEngine>
#include <KDBusService>
#include <KLocalizedString>

int main(int argc, char **argv)
{
	QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain("qaptupdator");
	KAboutData aboutData("qaptupdator", i18n("qaptupdator"), "0.1", i18n("Touch-friendly settings application."),
    KAboutLicense::GPL, i18n("Copyright 2011-2015, Sebastian Kügler"));
    aboutData.addAuthor(i18n("Sebastian Kügler"), i18n("Maintainer"), "sebas@kde.org");
    aboutData.addAuthor(i18n("Marco Martin"), i18n("Maintainer"), "mart@kde.org");
    aboutData.setDesktopFileName("org.jingos.qaptupdator");
    KAboutData::setApplicationData(aboutData);

    KDBusService *service = new KDBusService(KDBusService::Unique, &app);

    QCommandLineParser parser;

    const QCommandLineOption agruments (
        {QStringLiteral("s"), QStringLiteral("sysversion")},
        i18n("system version"), i18n("sysversion"));

    parser.addOption(agruments);
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    const QString module = parser.value(agruments);

    QQmlApplicationEngine engine;

    qmlRegisterType<UpgradeModel>("org.jingos.updator" , 1, 0 , "UpgradeModel");
    qmlRegisterType<Recording>("org.jingos.updator", 1, 0, "Recording");
    qmlRegisterSingletonType<RecordingModel>("org.jingos.updator", 1, 0, "RecordingModel", [] (QQmlEngine *, QJSEngine *) -> QObject* {
        return RecordingModel::instance();
    });
    QObject::connect(service, &KDBusService::activateRequested, RecordingModel::instance(), [](const QStringList &arguments) {
       emit RecordingModel::instance() -> activateRequested();
    });

    engine.rootContext()->setContextProperty("currentVersion", module);
    engine.load(QUrl(QStringLiteral("qrc:/qml/update.qml")));

    return app.exec();
}
