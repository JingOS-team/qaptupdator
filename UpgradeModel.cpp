/***************************************************************************
 *   Copyright © 2010 Jonathan Thomas <echidnaman@kubuntu.org>             *
 *               2021 Wang Rui <wangrui@jingos.com>                        *
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
// Own includes
// #include "muonapt/MuonStrings.h"
// #include "TransactionWidget.h"
// #include "FilterWidget/FilterWidget.h"
// #include "ManagerWidget.h"
// #include "ReviewWidget.h"
// #include "MuonSettings.h"
// #include "StatusWidget.h"
// #include "config/ManagerSettingsDialog.h"
// #include "muonapt/QAptActions.h"
// Component.onCompleted: {
//     // 1. check for update
//     // 2. full upgrade
// }

UpgradeModel::UpgradeModel(QObject *parent)
    : QObject(parent), m_reloading(false), m_trans(nullptr)
{
    qDebug() << "create upgrader, call init";
    init();
    qDebug() << "create upgrader, inited";
    QTimer::singleShot(10, this, SLOT(initObject()));
}

UpgradeModel::~UpgradeModel()
{
    // MuonSettings::self()->save();
}

void UpgradeModel::init()
{
    updateStatus = 0 ; 

    registSysDlgAction();

    m_backend = new QApt::Backend(this);
    m_backend->init();
    connect(m_backend, SIGNAL(packageChanged()), this, SLOT(updateStatusBar()));
}

void UpgradeModel::initObject()
{
   
}

void UpgradeModel::setCurrentVersion(QString version){

    currentVersion = version;
    qDebug() <<"设置要更新的版本"<< currentVersion;
}

void UpgradeModel::registSysDlgAction(){    
        bool rv = QDBusConnection::sessionBus().connect(QString(), 
        QString("/org/kde/Polkit1AuthAgent"), 
        "org.kde.Polkit1AuthAgent",                                                    
        "sigCancel", this, SLOT(slotReceiveDbusCancel()));    
        if (rv == false){        
            qWarning() << "dbus connect sigCancel fail";    
            qDebug() <<"绑定系统对话框失败";
        }else {
            qDebug() <<"绑定系统对话框OK";
        }    

        /* rv = QDBusConnection::sessionBus().connect(QString(), 
        QString("/org/kde/Polkit1AuthAgent"), 
        "org.kde.Polkit1AuthAgent",
        "sigConfirm", 
        this, 
        SLOT(slotReceiveDbusConfirm()));    
        if (rv == false){        
            
            qWarning() << "dbus connect sigConfirm fail";    
            }     */
          
          
}

void UpgradeModel::slotReceiveDbusCancel(){
    // Qt.quit();
    qDebug() <<"绑定系统对话框OK";
    QApplication* app;
        app->exit(0);
}

void UpgradeModel::updateCache()
{
    qDebug() << "*************UpgradeModel::updateCache*********";
    updateStatus = 1 ; 
    if (m_trans) // Transaction running, you could queue these though
        return;

    qDebug() << "begin updateCache";
    m_trans = m_backend->updateCache();

    // Provide proxy/locale to the transaction
    if (KProtocolManager::proxyType() == KProtocolManager::ManualProxy) {
        m_trans->setProxy(KProtocolManager::proxyFor("http"));
    }

    m_trans->setLocale(QLatin1String(setlocale(LC_MESSAGES, 0)));

    // Pass the new current transaction on to our child widgets
    // m_cacheUpdateWidget->setTransaction(m_trans);
    connect(m_trans, SIGNAL(statusChanged(QApt::TransactionStatus)),
            this, SLOT(onTransactionStatusChanged(QApt::TransactionStatus)));

    m_trans->run();
}

void UpgradeModel::upgrade()
{
    qDebug() << "*************UpgradeModel::fullupgrade*********";
    // qDebug() << "call upgrade";
    if (m_trans) // Transaction running, you could queue these though
        return;
    qDebug() << "*************UpgradeModel::111111*********";
    m_trans = m_backend->upgradeSystem(QApt::FullUpgrade);
    // Provide proxy/locale to the transaction
    if (KProtocolManager::proxyType() == KProtocolManager::ManualProxy) {
        m_trans->setProxy(KProtocolManager::proxyFor("http"));
    }

    m_trans->setLocale(QLatin1String(setlocale(LC_MESSAGES, 0)));

    // Pass the new current transaction on to our child widgets
    // m_cacheUpdateWidget->setTransaction(m_trans);
    // m_commitWidget->setTransaction(m_trans);
    // cacheTransaction();
    updateUpgradeProcess();
    qDebug() << "*************UpgradeModel::22222*********";
    connect(m_trans, SIGNAL(statusChanged(QApt::TransactionStatus)),
            this, SLOT(onTransactionStatusChanged(QApt::TransactionStatus)));

    m_trans->run();
}

void UpgradeModel::commitAction()
{
    qDebug() << "*************UpgradeModel::commitAction*********"; 
    if (m_trans) // Transaction running, you could queue these though
        return;

    if (!m_package->isInstalled()) {
        m_package->setInstall();
    } else {
        m_package->setRemove();
    }

    if (m_package->state() & QApt::Package::Upgradeable) {
        m_package->setInstall();
    }

    m_trans = m_backend->commitChanges();

    // Provide proxy/locale to the transaction
    if (KProtocolManager::proxyType() == KProtocolManager::ManualProxy) {
        m_trans->setProxy(KProtocolManager::proxyFor("http"));
    }

    m_trans->setLocale(QLatin1String(setlocale(LC_MESSAGES, 0)));

    // Pass the new current transaction on to our child widgets
    // m_cacheUpdateWidget->setTransaction(m_trans);
    // m_commitWidget->setTransaction(m_trans);
    // cacheTransaction();
    // updateUpgradeProcess();
    connect(m_trans, SIGNAL(statusChanged(QApt::TransactionStatus)),
            this, SLOT(onTransactionStatusChanged(QApt::TransactionStatus)));

    m_trans->run();
}

void UpgradeModel::onTransactionStatusChanged(QApt::TransactionStatus status)
{
    QString headerText;
    qDebug() << "current status:" << status;
    switch (status) {
    case QApt::RunningStatus:
        qDebug() << "QApt::RunningStatus::********************"<< m_trans->role();
        // For roles that start by downloading something, switch to download view
        if (m_trans->role() == (QApt::UpdateCacheRole || QApt::UpgradeSystemRole ||
                                QApt::CommitChangesRole || QApt::DownloadArchivesRole ||
                                QApt::InstallFileRole)) {
            // m_stack->setCurrentWidget(m_cacheUpdateWidget);
        }

        break;
    case QApt::DownloadingStatus:
        qDebug() << "QApt::DownloadingStatus:********************";
        // m_stack->setCurrentWidget(m_cacheUpdateWidget);
        // cacheTransaction()
        break;
    case QApt::CommittingStatus:
        qDebug() << "QApt::CommittingStatus:********************";
        updateUpgradeProcess();
        // m_stack->setCurrentWidget(m_commitWidget);
        break;
    case QApt::FinishedStatus:
        qDebug() << "QApt::FinishedStatus:********************";
        // FIXME: Determine which transactions need to reload cache on completion
        m_backend->reloadCache();
        // m_stack->setCurrentWidget(m_mainWidget);
        updateStatusBar();
         m_trans->deleteLater();
        m_trans = 0;

        if(updateStatus == 1 ){
            updateStatus = 2;
            upgrade();
            emit upgradeStatusChanged(1);
        } else if(updateStatus == 2){
            updateStatus = 0 ; 
            emit upgradeFinished(1);
            // 
            qDebug() << "QApt::22222222222222222222222:********************";

            KNotification *notification = new KNotification("FailedToGetSecrets", KNotification::CloseOnTimeout);
            notification->setComponentName("networkmanagement");
            notification->setTitle(i18n("System Upgrade"));
            notification->setText("System upgrade complete , please restart the device.");
            notification->setIconName(QStringLiteral("dialog-warning"));
            notification->sendEvent();
        }
        break;

    // case QApt::ExitStatus:

    //     qDebug() << "QApt::ExitStatus:********************";
    //     emit upgradeFinished(2);
    //     break;   
    default:
       
        break;
    }
}

/**
 * 升级结束后提示具体有多少内容被更新
 */
void UpgradeModel::updateStatusBar()
{
    qDebug() << m_backend->packageCount(QApt::Package::Installed) << " Installed "
             << m_backend->packageCount(QApt::Package::Upgradeable) << " upgradeable "
             << m_backend->packageCount() << " available";

    qDebug() << m_backend->packageCount(QApt::Package::ToInstall) << " To install "
             << m_backend->packageCount(QApt::Package::ToUpgrade) << " to upgrade "
             << m_backend->packageCount(QApt::Package::ToRemove) << " to remove";
}

void UpgradeModel::updateUpgradeProcess(/* QApt::Transaction *trans */){
    //   m_trans = trans;
    connect(m_trans, SIGNAL(statusDetailsChanged(QString)),
            this, SLOT(updateLabel(QString)));
    connect(m_trans, SIGNAL(progressChanged(int)),
            this, SLOT(updateProgress(int)));
    // m_progressBar->setValue(trans->progress());
}

void UpgradeModel::updateLabel(QString name){
    qDebug()<< "UpgradeModel => updateLabel::"<< name ;
    RecordingModel::instance()-> insertRecording(name);
    // emit labelUpdated(name);
}


void UpgradeModel::updateProgress(int value){
    qDebug()<< "UpgradeModel => updateProgress::"<< value ;
    // qDebug()<< "UpgradeModel => updateProgress::"<< m_trans->progress() ;
     emit progressUpdated(value);
    // trans->progress()

}

void UpgradeModel::restartDevice(){
    // QProcess::execute("shutdown -r -t 0");
    QProcess::execute("reboot");
}

void UpgradeModel::cacheTransaction(){
    // m_trans = trans;
    cacheClear();
    // m_cancelButton->setEnabled(m_trans->isCancellable());
    // connect(m_cancelButton, SIGNAL(pressed()),
    //         m_trans, SLOT(cancel()));

    // Listen for changes to the transaction
    // connect(m_trans, SIGNAL(cancellableChanged(bool)),
    //         m_cancelButton, SLOT(setEnabled(bool)));
    // connect(m_trans, SIGNAL(statusChanged(QApt::TransactionStatus)),
    //         this, SLOT(onTransactionStatusChanged(QApt::TransactionStatus)));
    connect(m_trans, SIGNAL(progressChanged(int)),
            this, SLOT(cacheProgressChanged(int)));
    connect(m_trans, SIGNAL(downloadProgressChanged(QApt::DownloadProgress)),
            this, SLOT(cacheDownloadProgressChanged(QApt::DownloadProgress)));
    connect(m_trans, SIGNAL(downloadSpeedChanged(quint64)),
            this, SLOT(cacheUpdateDownloadSpeed(quint64)));
    connect(m_trans, SIGNAL(downloadETAChanged(quint64)),
            this, SLOT(cacheUpdateETA(quint64)));
}


void UpgradeModel::cacheAddItem(const QString &message)
{

     qDebug()<< "UpgradeModel => cacheAddItem::"<< message ;

    QStandardItem *n = new QStandardItem();
    n->setText(message);
    m_downloadModel->appendRow(n);
    // m_downloadView->scrollTo(m_downloadModel->indexFromItem(n));
}

void UpgradeModel::cacheClear()
{
    m_downloadModel->clear();
    m_downloads.clear();
    // m_totalProgress->setValue(0);
}

void UpgradeModel::resetNewVersion(){
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

void UpgradeModel::cacheDownloadProgressChanged(const QApt::DownloadProgress &progress){
    if (!m_downloads.contains(progress.uri())) {
        cacheAddItem(progress.uri());
        m_downloads.append(progress.uri());
    
    }
}

void UpgradeModel::cacheProgressChanged(int progress)
{
    qDebug() << "Upgrade => cacheProgressChanged ::: " << progress ; 
    if (progress > 100) {
        // m_totalProgress->setMaximum(0);
    } 
    // else if (progress > m_lastRealProgress) {
    //     // m_totalProgress->setMaximum(100);
    //     // m_totalProgress->setValue(progress);
    //     // m_lastRealProgress = progress;
    // }
}

void UpgradeModel::cacheUpdateDownloadSpeed(quint64 speed){
    QString downloadSpeed = i18n("Download rate: %1/s",
                                speed);
    qDebug()<< "UpgradeModel cacheUpdateDownloadSpeed::"<< downloadSpeed;
}

void UpgradeModel::cacheUpdateETA(quint64 ETA){
    QString timeRemaining;
    int ETAMilliseconds = ETA * 1000;

    if (ETAMilliseconds <= 0 || ETAMilliseconds > 14*24*60*60*1000) {
        // If ETA is less than zero or bigger than 2 weeks
        timeRemaining = i18n("Unknown time remaining");
    } else {
        // timeRemaining = i18n("%1 remaining", KGlobal::locale()->prettyFormatDuration(ETAMilliseconds));
        timeRemaining = "44";
    }

    qDebug()<< "UpgradeModel => cacheUpdateETA::"<< timeRemaining;
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
        qDebug()<<"更新文本:: "<<m_recordings.count();
        endInsertRows();
}

void RecordingModel::deleteRecording(const int index)
{
    beginRemoveRows({}, index, index);
    m_recordings.removeAt(index);
    endRemoveRows();
}


