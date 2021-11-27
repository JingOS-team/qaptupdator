/***************************************************************************
 *   Copyright © 2010 Jonathan Thomas <echidnaman@kubuntu.org>             *
 *               2021 Wang Rui <wangrui@jingos.com>                        *
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

#include "UpgradeModel.h"

// Qt includes
#include <QApplication>
#include <QtCore/QStringBuilder>
#include <QtCore/QTimer>
#include <QSplitter>
#include <QtWidgets/QStackedWidget>
#include <QToolBox>
#include <QVBoxLayout>
#include <QAction>
#include <QProcess>
#include <KNotification>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QDBusConnection>
// KDE includes
#include <KActionCollection>
#include <KLocalizedString>
#include <KMessageBox>
#include <KProtocolManager>
#include <KStandardAction>

// QApt includes
#include <QApt/Backend>
#include <QApt/Config>
#include <QApt/Transaction>

#include <QDebug>
#include <QString>

UpgradeModel::UpgradeModel(QObject *parent) : QObject(parent), m_reloading(false), m_trans(nullptr) {
    init();
    QTimer::singleShot(10, this, SLOT(initObject()));
    qaptupdatorRuning = false;

    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.registerService("com.jingos.qaptupdator");
    connection.registerObject("/signals/objects", this, QDBusConnection::ExportAllSlots);
}

UpgradeModel::~UpgradeModel()
{}

void UpgradeModel::invokeSendEndSig()
{
    QDBusMessage message = QDBusMessage::createSignal("/signals/objects", "com.jingos.qaptupdator", "sigEnd");
    QDBusConnection::sessionBus().send(message);
}

void UpgradeModel::init()
{
    updateStatus = 0 ;

    m_backend = new QApt::Backend(this);
    m_backend->init();
    connect(m_backend, SIGNAL(packageChanged()), this, SLOT(updateStatusBar()));
}

void UpgradeModel::setCurrentVersion(QString version) {
    currentVersion = version;
}

void UpgradeModel::registSysDlgAction() {
    bool rv = QDBusConnection::sessionBus().connect(QString(),
    QString("/org/kde/Polkit1AuthAgent"),
    "org.kde.Polkit1AuthAgent",
    "sigCancel", this, SLOT(slotReceiveDbusCancel()));
}

void UpgradeModel::slotReceiveDbusCancel() {
    QApplication* app;
    app->exit(0);
}

void UpgradeModel::updateCache()
{
    updateStatus = 1 ;
    if (m_trans)
        return;
    m_trans = m_backend->updateCache();
    if (KProtocolManager::proxyType() == KProtocolManager::ManualProxy) {
        m_trans->setProxy(KProtocolManager::proxyFor("http"));
    }

    m_trans->setLocale(QLatin1String(setlocale(LC_MESSAGES, 0)));

    cacheTransaction();
    connect(m_trans, SIGNAL(statusChanged(QApt::TransactionStatus)), this, SLOT(onTransactionStatusChanged(QApt::TransactionStatus)));

    m_trans->run();
    qaptupdatorRuning = true;
}

void UpgradeModel::upgrade()
{
    if (m_trans) {
        return;
    }
    m_trans = m_backend->upgradeSystem(QApt::FullUpgrade);

    if (KProtocolManager::proxyType() == KProtocolManager::ManualProxy) {
        m_trans->setProxy(KProtocolManager::proxyFor("http"));
    }

    m_trans->setLocale(QLatin1String(setlocale(LC_MESSAGES, 0)));
    updateUpgradeProcess();
    connect(m_trans, SIGNAL(statusChanged(QApt::TransactionStatus)), this, SLOT(onTransactionStatusChanged(QApt::TransactionStatus)));
    m_trans->run();
}

void UpgradeModel::commitAction()
{
    if (m_trans) {
        return;
    }

    if (!m_package->isInstalled()) {
        m_package->setInstall();
    } else {
        m_package->setRemove();
    }

    if (m_package->state() & QApt::Package::Upgradeable) {
        m_package->setInstall();
    }

    m_trans = m_backend->commitChanges();

    if (KProtocolManager::proxyType() == KProtocolManager::ManualProxy) {
        m_trans->setProxy(KProtocolManager::proxyFor("http"));
    }

    m_trans->setLocale(QLatin1String(setlocale(LC_MESSAGES, 0)));

    connect(m_trans, SIGNAL(statusChanged(QApt::TransactionStatus)), this, SLOT(onTransactionStatusChanged(QApt::TransactionStatus)));

    m_trans->run();
}

void UpgradeModel::onTransactionStatusChanged(QApt::TransactionStatus status)
{
    QString headerText;
    switch (status) {

    case QApt::RunningStatus:
        if (m_trans->role() == (QApt::UpdateCacheRole || QApt::UpgradeSystemRole || QApt::CommitChangesRole || QApt::DownloadArchivesRole || QApt::InstallFileRole)) {}
        break;

    case QApt::DownloadingStatus:
        break;

    case QApt::CommittingStatus:
        if ( m_trans->downloadProgress().fileSize() > m_trans->downloadProgress().fetchedSize()) {
            invokeSendEndSig();
            emit upgradeFinished(2);

            KNotification *notification = new KNotification("SystemUpdateDownLoadFinished", KNotification::CloseOnTimeout);
            notification->setComponentName("settings_main");
            notification->setTitle(i18n("System Upgrade"));
            notification->setText(i18n("An error was encountered while downloading packages"));
            notification->sendEvent();
            m_trans->cancel();
            m_trans->deleteLater();
        }
        break;

    case QApt::LoadingCacheStatus:
        if ( m_trans->downloadProgress().fileSize() > m_trans->downloadProgress().fetchedSize()) {
            invokeSendEndSig();
            emit upgradeFinished(2);
            KNotification *notification = new KNotification("SystemUpdateDownLoadFinished", KNotification::CloseOnTimeout);
            notification->setComponentName("settings_main");
            notification->setTitle(i18n("System Upgrade"));
            notification->setText(i18n("An error was encountered while downloading packages"));
            notification->sendEvent();
            m_trans->cancel();
            m_trans->deleteLater();
        }
        break;

    case QApt::FinishedStatus:
        m_backend->reloadCache();

        if (m_trans->exitStatus()) { //We will send a failed signal when exitStatus is failed
            qaptupdatorRuning = false;
            emit upgradeFinished(2);
            m_trans->cancel();
            m_trans->deleteLater();
            return;
        }

        m_trans->cancel();
        m_trans->deleteLater();
        m_trans = 0;

        if (updateStatus == 1 ) {
            updateStatus = 2;
            upgrade();
            emit upgradeStatusChanged(1);
        } else if (updateStatus == 2) {
            qaptupdatorRuning = false;
            updateStatus = 0;
            invokeSendEndSig();
            emit upgradeFinished(1);
            KNotification *notification = new KNotification("SystemUpdateDownLoadFinished", KNotification::CloseOnTimeout);
            notification->setComponentName("settings_main");
            notification->setTitle(i18n("System Upgrade"));
            notification->setText(i18n("System %1 has been updated", currentVersion));
            notification->sendEvent();
        }

        break;

    default:
        break;
    }
}

void UpgradeModel::updateStatusBar()
{}

void UpgradeModel::updateUpgradeProcess()
{
    connect(m_trans, SIGNAL(statusDetailsChanged(QString)), this, SLOT(updateLabel(QString)));
    connect(m_trans, SIGNAL(progressChanged(int)), this, SLOT(updateProgress(int)));
    connect(m_trans, &QApt::Transaction::errorOccurred,this, [this](QApt::ErrorCode a) {
        QString notificationText = QString();
        m_trans->cancel();
        m_trans->deleteLater();
        invokeSendEndSig();
        emit upgradeFinished(2);

        switch (a) {
            case QApt::InitError:
                notificationText = i18n("Cache could not be initialized");
                break;

            case QApt::LockError:
                notificationText = i18n("Package cache could not be locked");
                break;

            case QApt::DiskSpaceError:
                notificationText = i18n("There is not enough disk space for an install");
                break;

            case QApt::FetchError:
                notificationText = i18n("Fetching packages failed");
                break;

            case QApt::CommitError:
                notificationText = i18n("Dpkg encounters an error during commit");
                break;

            case QApt::AuthError:
                notificationText = i18n("User has not given proper authentication");
                break;

            case QApt::WorkerDisappeared:
                notificationText = i18n("Worker crashes or disappears");
                break;

            case QApt::UntrustedError:
                notificationText = i18n("APT prevents the installation of untrusted packages");
                break;

            case QApt::DownloadDisallowedError:
                notificationText = i18n("APT configuration prevents downloads");
                break;

            case QApt::NotFoundError:
                notificationText = i18n("Selected package does not exist");
                break;

            case QApt::WrongArchError:
                notificationText = i18n("A .deb package cannot be installed due to an arch mismatch");
                break;

            case QApt::MarkingError:
                notificationText = i18n("Worker cannot mark packages without broken dependencies");
                break;

            default:
                notificationText = i18n("An invalid/unknown value");
                break;
        }

        KNotification *notification = new KNotification("SystemUpdateDownLoadFinished", KNotification::CloseOnTimeout);
        notification->setComponentName("settings_main");
        notification->setTitle(i18n("System Upgrade"));
        notification->setText(notificationText);
        notification->sendEvent();
    });
}

void UpgradeModel::updateLabel(QString name) {
    RecordingModel::instance()-> insertRecording(name);
}

void UpgradeModel::updateProgress(int value)
{
    if (value >= 101) {
        return;
    }
    emit progressUpdated(value);
}

void UpgradeModel::restartDevice()
{
    QDBusMessage message = QDBusMessage::createMethodCall("org.kde.Shutdown", "/Shutdown", "org.kde.Shutdown", "logoutAndReboot");

    QDBusMessage response = QDBusConnection::sessionBus().call(message);

    if (response.type() == QDBusMessage::ReplyMessage) {
        QString value = response.arguments().takeFirst().toString();
        qWarning() << QString("dbus value =  %1").arg(value);
    } else {
        qWarning() << "value method called failed!";
    }
}

void UpgradeModel::cacheTransaction()
{
    connect(m_trans, SIGNAL(progressChanged(int)),
            this, SLOT(cacheProgressChanged(int)));
    connect(m_trans, SIGNAL(downloadProgressChanged(QApt::DownloadProgress)),
            this, SLOT(cacheDownloadProgressChanged(QApt::DownloadProgress)));
    connect(m_trans, SIGNAL(downloadSpeedChanged(quint64)),
            this, SLOT(cacheUpdateDownloadSpeed(quint64)));
    connect(m_trans, SIGNAL(downloadETAChanged(quint64)),
            this, SLOT(cacheUpdateETA(quint64)));
}

void UpgradeModel::resetNewVersion() {
    saveSetting("haveNewVersion" , "false");
}

QString UpgradeModel::loadSetting( QString key , QString defaultValue )
{
    KSharedConfigPtr profilesConfig = KSharedConfig::openConfig("update_config" , KConfig::SimpleConfig);
    KConfigGroup acProfile(profilesConfig , "UPDATE");
    QString value = acProfile.readEntry(key, defaultValue);
    return value ;
}

QString UpgradeModel::saveSetting(QString key ,QString value )
{
    KSharedConfigPtr profilesConfig = KSharedConfig::openConfig("update_config" , KConfig::SimpleConfig);
    KConfigGroup acProfile(profilesConfig , "UPDATE");
    acProfile.writeEntry(key, value);
    profilesConfig->sync();
    return "";
}

void UpgradeModel::cacheDownloadProgressChanged(const QApt::DownloadProgress &progress)
{
}

void UpgradeModel::cacheProgressChanged(int progress)
{
}

void UpgradeModel::cacheUpdateDownloadSpeed(quint64 speed)
{
    QString downloadSpeed = i18n("Download rate: %1/s", speed);
}

void UpgradeModel::cacheUpdateETA(quint64 ETA) {
    QString timeRemaining;
    int ETAMilliseconds = ETA * 1000;

    if (ETAMilliseconds <= 0 || ETAMilliseconds > 14 * 24 * 60 * 60 * 1000) {
        timeRemaining = i18n("Unknown time remaining");
    } else {
        timeRemaining = "44";
    }
}

bool UpgradeModel::updatingStatus()
{
    return qaptupdatorRuning;
}

Recording::Recording(QObject *parent ,const QString &fileName)
{
    m_fileName = fileName;
}

Recording::~Recording()
{
}

RecordingModel::RecordingModel(QObject *parent) : QAbstractListModel(parent)
{
}

void Recording::setFileName(const QString &fileName)
{
    m_fileName = fileName;
    emit propertyChanged();
}

RecordingModel::~RecordingModel()
{
    qDeleteAll(m_recordings);
}

QHash<int, QByteArray> RecordingModel::roleNames() const
{
    return {{Roles::RecordingRole, "recording"}};
}

QVariant RecordingModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_recordings.count() || index.row() < 0)
        return {};

    auto *recording = m_recordings.at(index.row());
    if (role == Roles::RecordingRole)
        return QVariant::fromValue(recording);

    return {};
}

int RecordingModel::rowCount(const QModelIndex &parent) const
{
    return m_recordings.count();
}

void RecordingModel::insertRecording(QString fileName)
{
    beginInsertRows({}, m_recordings.count(), m_recordings.count());
    Recording *rd = new Recording(this, fileName);
    m_recordings.append(rd);
    endInsertRows();
}

void RecordingModel::deleteRecording(const int index)
{
    beginRemoveRows({}, index, index);
    m_recordings.removeAt(index);
    endRemoveRows();
}
