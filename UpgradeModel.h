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

#ifndef UPGRADEMODEL_H
#define UPGRADEMODEL_H

#include <QObject>
#include <qapt/globals.h>
#include <QStandardItemModel>
#include <QAbstractListModel>
#include <QGuiApplication>

namespace QApt {
    class Backend;
    class Package;
    class Transaction;
    class DownloadProgress;
}

/**
 * This class serves as the main window for Muon.  It handles the
 * menus, toolbars, and status bars.
 *
 * @short Main window class
 * @author Jonathan Thomas <echidnaman@kubuntu.org>
 */
class UpgradeModel : public QObject
{
    Q_OBJECT
public:
    UpgradeModel(QObject *parent = nullptr);
    ~UpgradeModel();

    void setCurrentVersion(QString version);
    QString getCurrentVersion();
    void registSysDlgAction();

    // QSize sizeHint() const Q_DECL_OVERRIDE;
    // bool queryClose() Q_DECL_OVERRIDE;

private:
    QApt::Backend *m_backend;
    QApt::Package *m_package;
    QApt::Transaction *m_trans;

    QStandardItemModel *m_downloadModel;
    QStringList m_downloads;

    int updateStatus; 

    bool m_reloading;
    QString currentVersion;

signals:
    void labelUpdated(QString type);
    void progressUpdated(int value);
    void upgradeFinished(int result);
    void upgradeStatusChanged(int status);

public Q_SLOTS:
    void init();
    void initObject();
    Q_INVOKABLE void updateCache();
    void commitAction();
    Q_INVOKABLE void upgrade();
    Q_INVOKABLE void resetNewVersion();
    QString loadSetting(QString key , QString defaultValue ); 
    QString saveSetting(QString key , QString value ) ;
    void onTransactionStatusChanged(QApt::TransactionStatus status);
    void updateStatusBar();
    void updateUpgradeProcess();
    void updateLabel(QString name);
    void updateProgress(int value);
    void slotReceiveDbusCancel();


    // void onTransactionStatusChanged(QApt::TransactionStatus status);
    void cacheProgressChanged(int progress);
    void cacheDownloadProgressChanged(const QApt::DownloadProgress &progress);
    void cacheUpdateDownloadSpeed(quint64 speed);
    void cacheUpdateETA(quint64 ETA);
    void cacheAddItem(const QString &message);
    void cacheClear();
    void cacheTransaction();
    void restartDevice();

    
};

/*****************************************/

class Recording : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY propertyChanged)
public:
    explicit Recording(QObject *parent = nullptr,const QString &fileName = {});
    ~Recording();

    QString fileName() const
    {
        return m_fileName;
    }

    void setFileName(const QString &fileName);

private:
    QString m_fileName;    
signals:
    void propertyChanged();    
};    


/*****************************************/
class RecordingModel;
static RecordingModel *s_recordingModel = nullptr;
class RecordingModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        RecordingRole = Qt::UserRole
    };

    static RecordingModel* instance()
    {
        if (!s_recordingModel) {
            s_recordingModel = new RecordingModel(qApp);
        }
        return s_recordingModel;
    }


    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    Q_INVOKABLE void insertRecording(QString fileName);
    Q_INVOKABLE void deleteRecording(const int index);

private:
    explicit RecordingModel(QObject *parent = nullptr);
    ~RecordingModel();

    QList<Recording*> m_recordings;    
};

#endif // UPGRADEMODEL_H