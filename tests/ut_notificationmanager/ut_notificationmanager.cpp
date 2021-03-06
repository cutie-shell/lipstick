/***************************************************************************
**
** Copyright (c) 2012 Jolla Ltd.
**
** This file is part of lipstick.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging
** of this file.
**
****************************************************************************/

#include <QtTest/QtTest>
#include "ut_notificationmanager.h"
#include "aboutsettings_stub.h"

#include "notificationmanager.h"
#include "notificationmanageradaptor_stub.h"
#include "lipsticknotification.h"
#include "categorydefinitionstore_stub.h"
#include "androidprioritystore_stub.h"
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QSqlError>
#include <mremoteaction.h>
#include <sys/statfs.h>

const static uint DISK_SPACE_NEEDED = 1024;

unsigned long diskSpaceAvailableKb;
bool diskSpaceChecked;
int statfs (const char *, struct statfs *st)
{
    st->f_bsize = 4096;
    st->f_bavail = (diskSpaceAvailableKb * 1024) / st->f_bsize;
    diskSpaceChecked = true;
    return 0;
}

// QDir stubs
bool qDirRemoveCalled;
bool QDir::remove(const QString &) {
    return qDirRemoveCalled = true;
}

// QSqlDatabase stubs
QString qSqlDatabaseAddDatabaseType = QString();
QSqlDatabase qSqlDatabaseInstance;
QSqlDatabase QSqlDatabase::addDatabase (const QString & type, const QString&)
{
    qSqlDatabaseAddDatabaseType = type;
    return qSqlDatabaseInstance;
}

QString qSqlDatabaseDatabaseName = QString();
void QSqlDatabase::setDatabaseName(const QString& name)
{
    qSqlDatabaseDatabaseName = name;
}

bool qSqlDatabaseOpenSucceeds = true;
bool qSqlIterateOpenSuccess = false;
int qSqlDatabaseOpenCalledCount = 0;
bool QSqlDatabase::open()
{
    bool success = qSqlDatabaseOpenSucceeds;

    // Change to negation for the next call
    if (qSqlIterateOpenSuccess) {
        qSqlDatabaseOpenSucceeds = !success;
    }
    qSqlDatabaseOpenCalledCount++;

    return success;
}

bool QSqlDatabase::isOpen() const
{
    return (qSqlDatabaseOpenCalledCount > 0);
}

QStringList qSqlDatabaseExec;
QSqlQuery QSqlDatabase::exec(const QString& query) const
{
    qSqlDatabaseExec << query;
    return QSqlQuery();
}

bool QSqlDatabase::transaction()
{
    return true;
}

bool qSqlDatabaseCommitCalled = false;
bool QSqlDatabase::commit()
{
    qSqlDatabaseCommitCalled = true;
    return true;
}

// QSqlQuery stubs
QStringList qSqlQueryExecQuery = QStringList();
int qSqlQueryIndex = -1;
QSqlQuery::QSqlQuery(const QString& query, QSqlDatabase)
{
    if (!query.isEmpty()) {
        qSqlQueryExecQuery << query;
    }
    qSqlQueryIndex = -1;
}

QSqlQuery::~QSqlQuery()
{
}

bool QSqlQuery::exec(const QString& query)
{
    qSqlQueryExecQuery << query;
    qSqlQueryIndex = -1;
    return true;
}

bool QSqlQuery::exec()
{
    return true;
}

QSqlError QSqlQuery::lastError() const
{
    return QSqlError();
}

QStringList qSqlQueryPrepare = QStringList();
bool QSqlQuery::prepare(const QString& query)
{
    qSqlQueryPrepare << query;
    return true;
}

QVariantList qSqlQueryAddBindValue = QVariantList();
void QSqlQuery::addBindValue(const QVariant &val, QSql::ParamType)
{
    qSqlQueryAddBindValue << val;
}

QHash<QString, int> qSqlRecordIndexOf;
QSqlRecord QSqlQuery::record() const
{
    qSqlRecordIndexOf.clear();
    if (qSqlQueryExecQuery.last() == "SELECT * FROM notifications") {
        qSqlRecordIndexOf.insert("id", 0);
        qSqlRecordIndexOf.insert("app_name", 1);
        qSqlRecordIndexOf.insert("app_icon", 2);
        qSqlRecordIndexOf.insert("summary", 3);
        qSqlRecordIndexOf.insert("body", 4);
        qSqlRecordIndexOf.insert("expire_timeout", 5);
        qSqlRecordIndexOf.insert("disambiguated_app_name", 6);
    } else if (qSqlQueryExecQuery.last() == "SELECT * FROM actions") {
        qSqlRecordIndexOf.insert("id", 0);
        qSqlRecordIndexOf.insert("action", 1);
    } else if (qSqlQueryExecQuery.last() == "SELECT * FROM hints") {
        qSqlRecordIndexOf.insert("id", 0);
        qSqlRecordIndexOf.insert("hint", 1);
        qSqlRecordIndexOf.insert("value", 2);
    } else if (qSqlQueryExecQuery.last() == "SELECT * FROM expiration") {
        qSqlRecordIndexOf.insert("id", 0);
        qSqlRecordIndexOf.insert("expire_at", 1);
    }
    return QSqlRecord();
}

typedef QHash<int, QVariant> QueryValues;
typedef QList<QueryValues> QueryValueList;
QHash<QString, QueryValueList> qSqlQueryValues;
bool QSqlQuery::next()
{
    if (qSqlQueryIndex < qSqlQueryValues.value(qSqlQueryExecQuery.last()).count() - 1) {
        qSqlQueryIndex++;
        return true;
    } else {
        return false;
    }
}

QVariant QSqlQuery::value(int i) const
{
    return qSqlQueryValues.value(qSqlQueryExecQuery.last()).at(qSqlQueryIndex).value(i);
}

int QSqlRecord::indexOf(const QString &name) const
{
    return qSqlRecordIndexOf.value(name, -1);
}

// QSqlTableModel stubs
QSet<QString> tableNamesModeled;
QMap<QSqlQueryModel*, QString> modelToTableName = QMap<QSqlQueryModel*, QString>();
void QSqlTableModel::setTable(const QString &tableName)
{
    modelToTableName[this] = tableName;
    tableNamesModeled.insert(tableName);
}

QHash<QString, int> notificationsTableFieldIndices;
QHash<QString, int> actionsTableFieldIndices;
QHash<QString, int> hintsTableFieldIndices;
QHash<QString, int> expirationTableFieldIndices;
int QSqlTableModel::fieldIndex(const QString &fieldName) const
{
    if (notificationsTableFieldIndices.empty()) {
        notificationsTableFieldIndices.insert("id", 0);
        notificationsTableFieldIndices.insert("app_name", 1);
        notificationsTableFieldIndices.insert("app_icon", 2);
        notificationsTableFieldIndices.insert("summary", 3);
        notificationsTableFieldIndices.insert("body", 4);
        notificationsTableFieldIndices.insert("expire_timeout", 5);
        notificationsTableFieldIndices.insert("disambiguated_app_name", 6);

        actionsTableFieldIndices.insert("id", 0);
        actionsTableFieldIndices.insert("action", 1);

        hintsTableFieldIndices.insert("id", 0);
        hintsTableFieldIndices.insert("hint", 1);
        hintsTableFieldIndices.insert("value", 2);

        expirationTableFieldIndices.insert("id", 0);
        expirationTableFieldIndices.insert("expire_at", 1);
    }

    int ret = -1;

    QString tableName = modelToTableName.value(const_cast<QSqlTableModel*>(this));
    if (tableName == "notifications") {
        if (notificationsTableFieldIndices.contains(fieldName)) {
            ret = notificationsTableFieldIndices.value(fieldName);
        }
    } else if (tableName == "actions") {
        if (actionsTableFieldIndices.contains(fieldName)) {
            ret = actionsTableFieldIndices.value(fieldName);
        }
    } else if (tableName == "hints") {
        if (hintsTableFieldIndices.contains(fieldName)) {
            ret = hintsTableFieldIndices.value(fieldName);
        }
    } else if (tableName == "expiration") {
        if (expirationTableFieldIndices.contains(fieldName)) {
            ret = expirationTableFieldIndices.value(fieldName);
        }
    }

    return ret;
}

// MRemoteAction stubs
QStringList mRemoteActionTrigger;
void MRemoteAction::trigger()
{
    mRemoteActionTrigger.append(toString());
}

void Ut_NotificationManager::init()
{
    qSqlQueryExecQuery.clear();
    qSqlQueryPrepare.clear();
    qSqlQueryAddBindValue.clear();
    qSqlQueryValues.clear();
    qSqlDatabaseAddDatabaseType.clear();
    qSqlDatabaseDatabaseName.clear();
    qSqlDatabaseOpenCalledCount = 0;
    qSqlDatabaseOpenSucceeds = true;
    qSqlIterateOpenSuccess = false;
    qDirRemoveCalled = false;
    qSqlDatabaseExec.clear();
    qSqlDatabaseCommitCalled = false;
    diskSpaceAvailableKb = DISK_SPACE_NEEDED + 100;
    diskSpaceChecked = true;
    mRemoteActionTrigger.clear();
}

void Ut_NotificationManager::cleanup()
{
    delete NotificationManager::instance_;
    NotificationManager::instance_ = 0;
}

void Ut_NotificationManager::testManagerIsSingleton()
{
    NotificationManager *manager1 = NotificationManager::instance();
    NotificationManager *manager2 = NotificationManager::instance();
    QVERIFY(manager1 != NULL);
    QCOMPARE(manager2, manager1);
}

void Ut_NotificationManager::testDatabaseConnectionSucceedsAndTablesAreOk()
{
    QHash<int, QVariant> versionValues;
    versionValues.insert(0, QVariant(2));
    qSqlQueryValues.clear();
    qSqlQueryValues["PRAGMA user_version"].append(versionValues);

    NotificationManager::instance();
    QCOMPARE(diskSpaceChecked, true);
    QCOMPARE(qSqlDatabaseAddDatabaseType, QString("QSQLITE"));
    QCOMPARE(qSqlDatabaseDatabaseName, QDir::homePath() + "/.local/share/system/privileged/Notifications/notifications.db");
    QCOMPARE(qSqlDatabaseOpenCalledCount, 1);
    QVERIFY(qSqlQueryExecQuery.count() > 5);
    QCOMPARE(qSqlQueryExecQuery.at(0), QString("PRAGMA journal_mode=WAL"));
    QCOMPARE(qSqlQueryExecQuery.at(1), QString("PRAGMA user_version"));
    QCOMPARE(qSqlQueryExecQuery.at(2), QString("SELECT * FROM actions"));
    QCOMPARE(qSqlQueryExecQuery.at(3), QString("SELECT * FROM hints"));
    QCOMPARE(qSqlQueryExecQuery.at(4), QString("SELECT * FROM expiration"));
    QCOMPARE(qSqlQueryExecQuery.at(5), QString("SELECT * FROM notifications"));
}

void Ut_NotificationManager::testDatabaseConnectionUpgradeToVersion2()
{
    tableNamesModeled.clear();
    QHash<int, QVariant> versionValues;
    versionValues.insert(0, QVariant(1));
    qSqlQueryValues.clear();
    qSqlQueryValues["PRAGMA user_version"].append(versionValues);

    // Check that the notifications tables is altered
    NotificationManager::instance();
    QCOMPARE(qSqlDatabaseAddDatabaseType, QString("QSQLITE"));
    QCOMPARE(qSqlDatabaseDatabaseName, QDir::homePath() + "/.local/share/system/privileged/Notifications/notifications.db");
    QCOMPARE(qSqlDatabaseOpenCalledCount, 1);
    QCOMPARE(qSqlQueryExecQuery.count(), 9);
    QCOMPARE(qSqlQueryExecQuery.at(0), QString("PRAGMA journal_mode=WAL"));
    QCOMPARE(qSqlQueryExecQuery.at(1), QString("PRAGMA user_version"));
    QCOMPARE(qSqlQueryExecQuery.at(2), QString("ALTER TABLE notifications ADD COLUMN disambiguated_app_name TEXT"));
    QCOMPARE(qSqlQueryExecQuery.at(3), QString("UPDATE notifications SET disambiguated_app_name = app_name"));
    QCOMPARE(qSqlQueryExecQuery.at(4), QString("PRAGMA user_version=2"));
    QCOMPARE(qSqlQueryExecQuery.at(5), QString("SELECT * FROM actions"));
    QCOMPARE(qSqlQueryExecQuery.at(6), QString("SELECT * FROM hints"));
    QCOMPARE(qSqlQueryExecQuery.at(7), QString("SELECT * FROM expiration"));
    QCOMPARE(qSqlQueryExecQuery.at(8), QString("SELECT * FROM notifications"));
    QCOMPARE(tableNamesModeled.contains("notifications"), true);
    QCOMPARE(tableNamesModeled.contains("actions"), true);
    QCOMPARE(tableNamesModeled.contains("hints"), true);
    QCOMPARE(tableNamesModeled.contains("expiration"), true);
}

void Ut_NotificationManager::testDatabaseConnectionSucceedsAndTablesAreNotOk()
{
    // Set up the tables so that the schema won't match
    notificationsTableFieldIndices.clear();
    actionsTableFieldIndices.clear();
    hintsTableFieldIndices.clear();
    expirationTableFieldIndices.clear();
    notificationsTableFieldIndices.insert("created", 0);
    actionsTableFieldIndices.insert("created", 0);
    hintsTableFieldIndices.insert("created", 0);
    expirationTableFieldIndices.insert("created", 0);
    QHash<int, QVariant> versionValues;
    versionValues.insert(0, QVariant(2));
    qSqlQueryValues.clear();
    qSqlQueryValues["PRAGMA user_version"].append(versionValues);

    // Check that the tables are dropped and recreated
    NotificationManager::instance();
    QCOMPARE(qSqlDatabaseAddDatabaseType, QString("QSQLITE"));
    QCOMPARE(qSqlDatabaseDatabaseName, QDir::homePath() + "/.local/share/system/privileged/Notifications/notifications.db");
    QCOMPARE(qSqlDatabaseOpenCalledCount, 1);
    QCOMPARE(qSqlQueryExecQuery.count(), 14);
    QCOMPARE(qSqlQueryExecQuery.at(0), QString("PRAGMA journal_mode=WAL"));
    QCOMPARE(qSqlQueryExecQuery.at(1), QString("PRAGMA user_version"));
    QCOMPARE(qSqlQueryExecQuery.at(2), QString("DROP TABLE notifications"));
    QCOMPARE(qSqlQueryExecQuery.at(3), QString("CREATE TABLE notifications (id INTEGER PRIMARY KEY, app_name TEXT, app_icon TEXT, summary TEXT, body TEXT, expire_timeout INTEGER, disambiguated_app_name TEXT)"));
    QCOMPARE(qSqlQueryExecQuery.at(4), QString("DROP TABLE actions"));
    QCOMPARE(qSqlQueryExecQuery.at(5), QString("CREATE TABLE actions (id INTEGER, action TEXT, PRIMARY KEY(id, action))"));
    QCOMPARE(qSqlQueryExecQuery.at(6), QString("DROP TABLE hints"));
    QCOMPARE(qSqlQueryExecQuery.at(7), QString("CREATE TABLE hints (id INTEGER, hint TEXT, value TEXT, PRIMARY KEY(id, hint))"));
    QCOMPARE(qSqlQueryExecQuery.at(8), QString("DROP TABLE expiration"));
    QCOMPARE(qSqlQueryExecQuery.at(9), QString("CREATE TABLE expiration (id INTEGER PRIMARY KEY, expire_at INTEGER)"));
    QCOMPARE(qSqlQueryExecQuery.at(10), QString("SELECT * FROM actions"));
    QCOMPARE(qSqlQueryExecQuery.at(11), QString("SELECT * FROM hints"));
    QCOMPARE(qSqlQueryExecQuery.at(12), QString("SELECT * FROM expiration"));
    QCOMPARE(qSqlQueryExecQuery.at(13), QString("SELECT * FROM notifications"));
    notificationsTableFieldIndices.clear();
    actionsTableFieldIndices.clear();
    hintsTableFieldIndices.clear();
    expirationTableFieldIndices.clear();
}

void Ut_NotificationManager::testFirstDatabaseConnectionFails()
{
    QHash<int, QVariant> versionValues;
    versionValues.insert(0, QVariant(2));
    qSqlQueryValues.clear();
    qSqlQueryValues["PRAGMA user_version"].append(versionValues);

    // Make the first database connection fail but the second to succeed
    qSqlDatabaseOpenSucceeds = false;
    qSqlIterateOpenSuccess = true;
    NotificationManager::instance();

    // Check that the old database is removed, the database opened twice and the database opened as expected on the second time
    QCOMPARE(qDirRemoveCalled, true);
    QCOMPARE(qSqlDatabaseOpenCalledCount, 2);
    QCOMPARE(qSqlQueryExecQuery.count(), 6);
}

void Ut_NotificationManager::testNotEnoughDiskSpaceToOpenDatabase()
{
    diskSpaceAvailableKb = DISK_SPACE_NEEDED - 100;

    // Check that the database is not opened when there is not enough space
    NotificationManager::instance();
    QCOMPARE(qSqlDatabaseOpenCalledCount, 0);
}

extern int MaxNotificationRestoreCount;

void Ut_NotificationManager::testNotificationsAreRestoredOnConstruction()
{
    const int oldValue = MaxNotificationRestoreCount;
    MaxNotificationRestoreCount = 2;

    // Make the database return two notifications with different values
    QHash<int, QVariant> notification1Values;
    QHash<int, QVariant> notification2Values;
    // Also include a notification which has expired
    QHash<int, QVariant> notification3Values;
    // Include a notification which will be excluded by the max restore count
    QHash<int, QVariant> notification4Values;
    notification1Values.insert(0, 1);
    notification1Values.insert(1, "appName1");
    notification1Values.insert(2, "appIcon1");
    notification1Values.insert(3, "summary1");
    notification1Values.insert(4, "body1");
    notification1Values.insert(5, 1);
    notification1Values.insert(6, "appName1");
    notification2Values.insert(0, 2);
    notification2Values.insert(1, "appName2");
    notification2Values.insert(2, "appIcon2");
    notification2Values.insert(3, "summary2");
    notification2Values.insert(4, "body2");
    notification2Values.insert(5, 2);
    notification2Values.insert(6, "appName2");
    notification3Values.insert(0, 3);
    notification3Values.insert(1, "appName3");
    notification3Values.insert(2, "appIcon3");
    notification3Values.insert(3, "summary3");
    notification3Values.insert(4, "body3");
    notification3Values.insert(5, 3);
    notification3Values.insert(6, "appName3");
    notification4Values.insert(0, 4);
    notification4Values.insert(1, "appName4");
    notification4Values.insert(2, "appIcon4");
    notification4Values.insert(3, "summary4");
    notification4Values.insert(4, "body4");
    notification4Values.insert(5, 4);
    notification4Values.insert(6, "appName4");
    QList<QHash<int, QVariant> > notificationValues;
    notificationValues << notification1Values << notification2Values << notification3Values << notification4Values;
    qSqlQueryValues["SELECT * FROM notifications"].append(notificationValues);
    QHash<uint, QHash<int, QVariant> > notificationValuesById;
    notificationValuesById.insert(1, notification1Values);
    notificationValuesById.insert(2, notification2Values);

    QHash<int, QVariant> notification1ActionIdentifier;
    QHash<int, QVariant> notification1ActionName;
    QHash<int, QVariant> notification2ActionIdentifier;
    QHash<int, QVariant> notification2ActionName;
    QHash<int, QVariant> notification3ActionIdentifier;
    QHash<int, QVariant> notification3ActionName;
    notification1ActionIdentifier.insert(0, 1);
    notification1ActionIdentifier.insert(1, "action1");
    notification1ActionName.insert(0, 1);
    notification1ActionName.insert(1, "Action 1");
    notification2ActionIdentifier.insert(0, 2);
    notification2ActionIdentifier.insert(1, "action2");
    notification2ActionName.insert(0, 2);
    notification2ActionName.insert(1, "Action 2");
    notification3ActionIdentifier.insert(0, 3);
    notification3ActionIdentifier.insert(1, "action3");
    notification3ActionName.insert(0, 3);
    notification3ActionName.insert(1, "Action 3");
    QList<QHash<int, QVariant> > notificationActions;
    notificationActions << notification1ActionIdentifier << notification1ActionName << notification2ActionIdentifier << notification2ActionName << notification3ActionIdentifier << notification3ActionName;
    qSqlQueryValues["SELECT * FROM actions"].append(notificationActions);
    QHash<uint, QStringList> notificationActionsById;
    notificationActionsById.insert(1, QStringList() << "action1" << "Action 1");
    notificationActionsById.insert(2, QStringList() << "action2" << "Action 2");

    const QDateTime timestamp(QDateTime::currentDateTimeUtc());
    const QDateTime earlyTimestamp(QDateTime::currentDateTimeUtc().addSecs(-1));
    QHash<int, QVariant> notification1Hint1;
    QHash<int, QVariant> notification1Hint2;
    QHash<int, QVariant> notification2Hint1;
    QHash<int, QVariant> notification2Hint2;
    QHash<int, QVariant> notification3Hint1;
    QHash<int, QVariant> notification3Hint2;
    QHash<int, QVariant> notification4Hint1;
    QHash<int, QVariant> notification4Hint2;
    notification1Hint1.insert(0, 1);
    notification1Hint1.insert(1, "hint1");
    notification1Hint1.insert(2, "value1");
    notification1Hint2.insert(0, 1);
    notification1Hint2.insert(1, "x-nemo-timestamp");
    notification1Hint2.insert(2, timestamp);
    notification2Hint1.insert(0, 2);
    notification2Hint1.insert(1, "hint2");
    notification2Hint1.insert(2, "value2");
    notification2Hint2.insert(0, 2);
    notification2Hint2.insert(1, "x-nemo-timestamp");
    notification2Hint2.insert(2, timestamp);
    notification3Hint1.insert(0, 3);
    notification3Hint1.insert(1, "hint3");
    notification3Hint1.insert(2, "value3");
    notification3Hint2.insert(0, 3);
    notification3Hint2.insert(1, "x-nemo-timestamp");
    notification3Hint2.insert(2, timestamp);
    notification4Hint1.insert(0, 4);
    notification4Hint1.insert(1, "hint4");
    notification4Hint1.insert(2, "value4");
    notification4Hint2.insert(0, 4);
    notification4Hint2.insert(1, "x-nemo-timestamp");
    notification4Hint2.insert(2, earlyTimestamp);
    QList<QHash<int, QVariant> > notificationHints;
    notificationHints << notification1Hint1 << notification1Hint2 << notification2Hint1 << notification2Hint2 << notification3Hint1 << notification3Hint2 << notification4Hint1 << notification4Hint2;
    qSqlQueryValues["SELECT * FROM hints"].append(notificationHints);
    QHash<uint, QList<QPair<QString, QVariant> > > notificationHintsById;
    notificationHintsById.insert(1, QList<QPair<QString, QVariant> >() << qMakePair(QString("hint1"), QVariant("value1")) << qMakePair(QString("x-nemo-timestamp"), QVariant(timestamp)));
    notificationHintsById.insert(2, QList<QPair<QString, QVariant> >() << qMakePair(QString("hint2"), QVariant("value2")) << qMakePair(QString("x-nemo-timestamp"), QVariant(timestamp)));

    QHash<int, QVariant> notification3Expiration;
    notification3Expiration.insert(0, 3);
    notification3Expiration.insert(1, QDateTime::currentDateTimeUtc().toMSecsSinceEpoch() - 1000);
    QList<QHash<int, QVariant> > notificationExpirations;
    notificationExpirations << notification3Expiration;
    qSqlQueryValues["SELECT * FROM expiration"].append(notificationExpirations);

    // Check that the notifications exist in the manager after construction and contain the expected values
    NotificationManager *manager = NotificationManager::instance();
    QList<uint> ids = manager->notificationIds();
    QCOMPARE(ids.count(), notificationValuesById.count());
    // The expired notification should not be reported
    QCOMPARE(ids.contains(3), false);
    // The excess notification should not be reported
    QCOMPARE(ids.contains(4), false);
    foreach (uint id, notificationValuesById.keys()) {
        QCOMPARE(id, notificationValuesById.value(id).value(0).toUInt());
        LipstickNotification *notification = manager->notification(id);
        QCOMPARE(notification->appName(), notificationValuesById.value(id).value(1).toString());
        QCOMPARE(notification->appIcon(), notificationValuesById.value(id).value(2).toString());
        QCOMPARE(notification->summary(), notificationValuesById.value(id).value(3).toString());
        QCOMPARE(notification->body(), notificationValuesById.value(id).value(4).toString());
        QCOMPARE(notification->expireTimeout(), notificationValuesById.value(id).value(5).toInt());
        QCOMPARE(notification->disambiguatedAppName(), notificationValuesById.value(id).value(6).toString());
        QCOMPARE(notification->actions(), notificationActionsById.value(id));
        QCOMPARE(notification->restored(), true);

        typedef QPair<QString, QVariant> HintData;
        foreach (const HintData &hint, notificationHintsById.value(id)) {
            QCOMPARE(notification->hints().value(hint.first), hint.second);
        }
    }

    MaxNotificationRestoreCount = oldValue;
}

void Ut_NotificationManager::testDatabaseCommitIsDoneOnDestruction()
{
    delete NotificationManager::instance();
    NotificationManager::instance_ = 0;

    QCOMPARE(qSqlDatabaseCommitCalled, true);
}

void Ut_NotificationManager::testCapabilities()
{
    // Check the supported capabilities includes all the Nemo hints
    QStringList capabilities = NotificationManager::instance()->GetCapabilities();
    QCOMPARE(capabilities.count(), 14);
    QCOMPARE((bool)capabilities.contains("body"), true);
    QCOMPARE((bool)capabilities.contains("actions"), true);
    QCOMPARE((bool)capabilities.contains("persistence"), true);
    QCOMPARE((bool)capabilities.contains("sound"), true);
    QCOMPARE((bool)capabilities.contains(LipstickNotification::HINT_ITEM_COUNT), true);
    QCOMPARE((bool)capabilities.contains(LipstickNotification::HINT_TIMESTAMP), true);
    QCOMPARE((bool)capabilities.contains(LipstickNotification::HINT_PREVIEW_BODY), true);
    QCOMPARE((bool)capabilities.contains(LipstickNotification::HINT_PREVIEW_SUMMARY), true);
    QCOMPARE((bool)capabilities.contains("x-nemo-remote-actions"), true);
    QCOMPARE((bool)capabilities.contains(LipstickNotification::HINT_USER_REMOVABLE), true);
    QCOMPARE((bool)capabilities.contains("x-nemo-get-notifications"), true);
}

void Ut_NotificationManager::testAddingNotification()
{
    NotificationManager *manager = NotificationManager::instance();

    // Check that notifications are inserted to a database, a timestamp is added and a signal about them are sent
    QSignalSpy modifiedSpy(manager, SIGNAL(notificationModified(uint)));
    QSignalSpy addedSpy(manager, SIGNAL(notificationAdded(uint)));
    QVariantHash hints;
    hints.insert("hint", "value");
    uint id = manager->Notify("appName", 0, "appIcon", "summary", "body", QStringList() << "action" << "Action", hints, 1);
    LipstickNotification *notification = manager->notification(id);
    QCOMPARE(disconnect(notification, SIGNAL(actionInvoked(QString)), manager, SLOT(invokeAction(QString))), true);
    QTRY_COMPARE(modifiedSpy.count(), 1);
    QCOMPARE(modifiedSpy.last().at(0).toUInt(), id);
    QCOMPARE(addedSpy.count(), 1);
    QCOMPARE(addedSpy.last().at(0).toUInt(), id);
    QCOMPARE(qSqlQueryPrepare.count(), 6);
    QCOMPARE(qSqlQueryPrepare.at(0), QString("INSERT INTO notifications VALUES (?, ?, ?, ?, ?, ?, ?)"));
    QCOMPARE(qSqlQueryPrepare.at(1), QString("INSERT INTO actions VALUES (?, ?)"));
    QCOMPARE(qSqlQueryPrepare.at(2), QString("INSERT INTO actions VALUES (?, ?)"));
    QCOMPARE(qSqlQueryPrepare.at(3), QString("INSERT INTO hints VALUES (?, ?, ?)"));
    QCOMPARE(qSqlQueryPrepare.at(4), QString("INSERT INTO hints VALUES (?, ?, ?)"));
    QCOMPARE(qSqlQueryPrepare.at(5), QString("INSERT INTO hints VALUES (?, ?, ?)"));
    QCOMPARE(qSqlQueryAddBindValue.count(), 20);
    QCOMPARE(qSqlQueryAddBindValue.at(0).toUInt(), id);
    QCOMPARE(qSqlQueryAddBindValue.at(1), QVariant("appName"));
    QCOMPARE(qSqlQueryAddBindValue.at(2), QVariant("appIcon"));
    QCOMPARE(qSqlQueryAddBindValue.at(3), QVariant("summary"));
    QCOMPARE(qSqlQueryAddBindValue.at(4), QVariant("body"));
    QCOMPARE(qSqlQueryAddBindValue.at(5).toInt(), 1);
    QCOMPARE(qSqlQueryAddBindValue.at(6), QVariant("appName"));
    QCOMPARE(qSqlQueryAddBindValue.at(7).toUInt(), id);
    QCOMPARE(qSqlQueryAddBindValue.at(8), QVariant("action"));
    QCOMPARE(qSqlQueryAddBindValue.at(9).toUInt(), id);
    QCOMPARE(qSqlQueryAddBindValue.at(10), QVariant("Action"));
    QStringList keys(QStringList() << "hint" << LipstickNotification::HINT_TIMESTAMP << LipstickNotification::HINT_PRIORITY);
    for (int i = 11; i <= 17; i += 3) {
        QCOMPARE(qSqlQueryAddBindValue.at(i).toUInt(), id);
        QString key(qSqlQueryAddBindValue.at(i + 1).toString());
        if (key == "hint") {
            QCOMPARE(qSqlQueryAddBindValue.at(i + 2), QVariant("value"));
            keys.removeAll(key);
        } else if (key == LipstickNotification::HINT_TIMESTAMP) {
            QCOMPARE(qSqlQueryAddBindValue.at(i + 2).type(), QVariant::String);
            keys.removeAll(key);
        } else if (key == LipstickNotification::HINT_PRIORITY) {
            QCOMPARE(qSqlQueryAddBindValue.at(i + 2).type(), QVariant::Int);
            keys.removeAll(key);
        }
    }
    QCOMPARE(keys.count(), 0);
    QCOMPARE(notification->appName(), QString("appName"));
    QCOMPARE(notification->appIcon(), QString("appIcon"));
    QCOMPARE(notification->summary(), QString("summary"));
    QCOMPARE(notification->body(), QString("body"));
    QCOMPARE(notification->actions().count(), 2);
    QCOMPARE(notification->actions().at(0), QString("action"));
    QCOMPARE(notification->actions().at(1), QString("Action"));
    QCOMPARE(notification->hints().value("hint"), QVariant("value"));
    QCOMPARE(notification->hints().value(LipstickNotification::HINT_TIMESTAMP).type(), QVariant::String);
    QCOMPARE(notification->restored(), false);
}

void Ut_NotificationManager::testUpdatingExistingNotification()
{
    NotificationManager *manager = NotificationManager::instance();

    uint id = manager->Notify("appName", 0, "appIcon", "summary", "body", QStringList(), QVariantHash(), 1);
    qSqlQueryPrepare.clear();
    qSqlQueryAddBindValue.clear();

    QSignalSpy modifiedSpy(manager, SIGNAL(notificationModified(uint)));
    QSignalSpy addedSpy(manager, SIGNAL(notificationAdded(uint)));
    uint newId = manager->Notify("newAppName", id, "newAppIcon", "newSummary", "newBody", QStringList() << "action", QVariantHash(), 2);
    QCOMPARE(newId, id);
    LipstickNotification *notification = manager->notification(id);
    QCOMPARE(disconnect(notification, SIGNAL(actionInvoked(QString)), manager, SLOT(invokeAction(QString))), true);
    QTRY_COMPARE(modifiedSpy.count(), 1);
    QCOMPARE(modifiedSpy.last().at(0).toUInt(), id);
    QCOMPARE(addedSpy.count(), 0);
    QCOMPARE(qSqlQueryPrepare.count(), 8);
    QCOMPARE(qSqlQueryPrepare.at(0), QString("DELETE FROM notifications WHERE id=?"));
    QCOMPARE(qSqlQueryPrepare.at(1), QString("DELETE FROM actions WHERE id=?"));
    QCOMPARE(qSqlQueryPrepare.at(2), QString("DELETE FROM hints WHERE id=?"));
    QCOMPARE(qSqlQueryPrepare.at(3), QString("DELETE FROM expiration WHERE id=?"));
    QCOMPARE(qSqlQueryPrepare.at(4), QString("INSERT INTO notifications VALUES (?, ?, ?, ?, ?, ?, ?)"));
    QCOMPARE(qSqlQueryPrepare.at(5), QString("INSERT INTO actions VALUES (?, ?)"));
    QCOMPARE(qSqlQueryPrepare.at(6), QString("INSERT INTO hints VALUES (?, ?, ?)"));
    QCOMPARE(qSqlQueryPrepare.at(7), QString("INSERT INTO hints VALUES (?, ?, ?)"));
    QCOMPARE(qSqlQueryAddBindValue.count(), 19);
    QCOMPARE(qSqlQueryAddBindValue.at(0).toUInt(), id);
    QCOMPARE(qSqlQueryAddBindValue.at(1).toUInt(), id);
    QCOMPARE(qSqlQueryAddBindValue.at(2).toUInt(), id);
    QCOMPARE(qSqlQueryAddBindValue.at(3).toUInt(), id);
    QCOMPARE(qSqlQueryAddBindValue.at(4).toUInt(), id);
    QCOMPARE(qSqlQueryAddBindValue.at(5), QVariant("newAppName"));
    QCOMPARE(qSqlQueryAddBindValue.at(6), QVariant("newAppIcon"));
    QCOMPARE(qSqlQueryAddBindValue.at(7), QVariant("newSummary"));
    QCOMPARE(qSqlQueryAddBindValue.at(8), QVariant("newBody"));
    QCOMPARE(qSqlQueryAddBindValue.at(9).toInt(), 2);
    QCOMPARE(qSqlQueryAddBindValue.at(10), QVariant("newAppName"));
    QCOMPARE(qSqlQueryAddBindValue.at(11).toUInt(), id);
    QCOMPARE(qSqlQueryAddBindValue.at(12), QVariant("action"));
    QStringList keys(QStringList() << LipstickNotification::HINT_TIMESTAMP << LipstickNotification::HINT_PRIORITY);
    for (int i = 13; i <= 16; i += 3) {
        QCOMPARE(qSqlQueryAddBindValue.at(i).toUInt(), id);
        QString key(qSqlQueryAddBindValue.at(i + 1).toString());
        if (key == LipstickNotification::HINT_TIMESTAMP) {
            QCOMPARE(qSqlQueryAddBindValue.at(i + 2).type(), QVariant::String);
            keys.removeAll(key);
        } else if (key == LipstickNotification::HINT_PRIORITY) {
            QCOMPARE(qSqlQueryAddBindValue.at(i + 2).type(), QVariant::Int);
            keys.removeAll(key);
        }
    }
    QCOMPARE(keys.count(), 0);
    QCOMPARE(notification->appName(), QString("newAppName"));
    QCOMPARE(notification->appIcon(), QString("newAppIcon"));
    QCOMPARE(notification->summary(), QString("newSummary"));
    QCOMPARE(notification->body(), QString("newBody"));
    QCOMPARE(notification->disambiguatedAppName(), QString("newAppName"));
    QCOMPARE(notification->actions().count(), 1);
    QCOMPARE(notification->actions().at(0), QString("action"));
    QCOMPARE(notification->hints().value(LipstickNotification::HINT_TIMESTAMP).type(), QVariant::String);
}

void Ut_NotificationManager::testUpdatingInexistingNotification()
{
    NotificationManager *manager = NotificationManager::instance();
    QSignalSpy modifiedSpy(manager, SIGNAL(notificationModified(uint)));
    QSignalSpy addedSpy(manager, SIGNAL(notificationAdded(uint)));
    uint id = manager->Notify("appName", 1, "appIcon", "summary", "body", QStringList(), QVariantHash(), 1);
    QCOMPARE(id, (uint)0);
    QTest::qWait(1100);
    QCOMPARE(modifiedSpy.count(), 0);
    QCOMPARE(addedSpy.count(), 0);
    QCOMPARE(qSqlQueryPrepare.count(), 0);
}

void Ut_NotificationManager::testRemovingExistingNotification()
{
    NotificationManager *manager = NotificationManager::instance();
    uint id = manager->Notify("appName", 0, "appIcon", "summary", "body", QStringList(), QVariantHash(), 1);
    qSqlQueryPrepare.clear();
    qSqlQueryAddBindValue.clear();

    QSignalSpy removedSpy(manager, SIGNAL(notificationRemoved(uint)));
    QSignalSpy closedSpy(manager, SIGNAL(NotificationClosed(uint,uint)));
    manager->CloseNotification(id);
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.last().at(0).toUInt(), id);
    QCOMPARE(closedSpy.count(), 1);
    QCOMPARE(closedSpy.last().at(0).toUInt(), id);
    QCOMPARE(closedSpy.last().at(1).toInt(), (int)NotificationManager::CloseNotificationCalled);
    QCOMPARE(qSqlQueryPrepare.count(), 4);
    QCOMPARE(qSqlQueryPrepare.at(0), QString("DELETE FROM notifications WHERE id=?"));
    QCOMPARE(qSqlQueryPrepare.at(1), QString("DELETE FROM actions WHERE id=?"));
    QCOMPARE(qSqlQueryPrepare.at(2), QString("DELETE FROM hints WHERE id=?"));
    QCOMPARE(qSqlQueryPrepare.at(3), QString("DELETE FROM expiration WHERE id=?"));
    QCOMPARE(qSqlQueryAddBindValue.count(), 4);
    QCOMPARE(qSqlQueryAddBindValue.at(0).toUInt(), id);
    QCOMPARE(qSqlQueryAddBindValue.at(1).toUInt(), id);
    QCOMPARE(qSqlQueryAddBindValue.at(2).toUInt(), id);
    QCOMPARE(qSqlQueryAddBindValue.at(3).toUInt(), id);
}

void Ut_NotificationManager::testRemovingInexistingNotification()
{
    NotificationManager *manager = NotificationManager::instance();
    QSignalSpy removedSpy(manager, SIGNAL(notificationRemoved(uint)));
    QSignalSpy closedSpy(manager, SIGNAL(NotificationClosed(uint,uint)));
    manager->CloseNotification(1);
    QCOMPARE(removedSpy.count(), 0);
    QCOMPARE(closedSpy.count(), 0);
    QCOMPARE(qSqlQueryPrepare.count(), 0);
}

void Ut_NotificationManager::testServerInformation()
{
    // Check that the server information uses application information from qApp
    QString name, vendor, version, spec_version;
    qApp->setApplicationName("testApp");
    qApp->setApplicationVersion("1.2.3");
    QString testVendor("test vendor");
    gAboutSettingsStub->stubSetReturnValue(OperatingSystemName, testVendor);

    name = NotificationManager::instance()->GetServerInformation(vendor, version, spec_version);
    QCOMPARE(name, qApp->applicationName());
    QCOMPARE(vendor, testVendor);
    QCOMPARE(version, qApp->applicationVersion());
    QCOMPARE(spec_version, QString("1.2"));
}

void Ut_NotificationManager::testModifyingCategoryDefinitionUpdatesNotifications()
{
    NotificationManager *manager = NotificationManager::instance();

    // Check the signal connection
    QCOMPARE(disconnect(manager->categoryDefinitionStore, SIGNAL(categoryDefinitionModified(QString)), manager, SLOT(updateNotificationsWithCategory(QString))), true);

    QSignalSpy multiModifiedSpy(manager, SIGNAL(notificationsModified(QList<uint>)));

    // Add two notifications, one with category "category1" and one with category "category2"
    QVariantHash hints1;
    QVariantHash hints2;
    hints1.insert(LipstickNotification::HINT_CATEGORY, "category1");
    hints1.insert(LipstickNotification::HINT_PREVIEW_BODY, "previewBody1");
    hints1.insert(LipstickNotification::HINT_PREVIEW_SUMMARY, "previewSummary1");
    hints2.insert(LipstickNotification::HINT_CATEGORY, "category2");
    hints2.insert(LipstickNotification::HINT_PREVIEW_BODY, "previewBody2");
    hints2.insert(LipstickNotification::HINT_PREVIEW_SUMMARY, "previewSummary2");
    uint id1 = manager->Notify("app1", 0, QString(), QString(), QString(), QStringList(), hints1, 0);
    uint id2 = manager->Notify("app2", 0, QString(), QString(), QString(), QStringList(), hints2, 0);

    QTRY_COMPARE(multiModifiedSpy.count(), 1);
    QList<uint> modifiedIds(multiModifiedSpy.last().at(0).value<QList<uint> >());
    QCOMPARE(modifiedIds.count(), 2);
    QVERIFY(modifiedIds.contains(id1));
    QVERIFY(modifiedIds.contains(id2));

    // Updating notifications with category "category2" should only update the notification with that category
    QSignalSpy modifiedSpy(manager, SIGNAL(notificationModified(uint)));
    manager->updateNotificationsWithCategory("category2");
    QTRY_COMPARE(modifiedSpy.count(), 1);
    QCOMPARE(modifiedSpy.last().at(0).toUInt(), id2);

    // The updated notifications should be marked as restored until modified
    QCOMPARE(manager->notification(id1)->restored(), false);
    QCOMPARE(manager->notification(id2)->restored(), true);
}

void Ut_NotificationManager::testUninstallingCategoryDefinitionRemovesNotifications()
{
    NotificationManager *manager = NotificationManager::instance();

    // Check the signal connection
    QCOMPARE(disconnect(manager->categoryDefinitionStore, SIGNAL(categoryDefinitionUninstalled(QString)), manager, SLOT(removeNotificationsWithCategory(QString))), true);

    // Add two notifications, one with category "category1" and one with category "category2"
    QVariantHash hints1;
    QVariantHash hints2;
    hints1.insert(LipstickNotification::HINT_CATEGORY, "category1");
    hints2.insert(LipstickNotification::HINT_CATEGORY, "category2");
    uint id1 = manager->Notify("app1", 0, QString(), QString(), QString(), QStringList(), hints1, 0);
    uint id2 = manager->Notify("app2", 0, QString(), QString(), QString(), QStringList(), hints2, 0);

    // Removing notifications with category "category2" should only remove the notification with that category
    QSignalSpy removedSpy(manager, SIGNAL(notificationRemoved(uint)));
    manager->removeNotificationsWithCategory("category2");
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.last().at(0).toUInt(), id2);
    QVERIFY(manager->notification(id1) != 0);
    QCOMPARE(manager->notification(id2), (LipstickNotification *)0);
}

void Ut_NotificationManager::testActionIsInvokedIfDefined()
{
    // Add two notifications, only the first one with an action named "action1"
    NotificationManager *manager = NotificationManager::instance();
    uint id1 = manager->Notify("app1", 0, QString(), QString(), QString(), QStringList() << "action1" << "Action 1", QVariantHash(), 0);
    uint id2 = manager->Notify("app2", 0, QString(), QString(), QString(), QStringList() << "action2" << "Action 2", QVariantHash(), 0);
    LipstickNotification *notification1 = manager->notification(id1);
    LipstickNotification *notification2 = manager->notification(id2);
    connect(this, SIGNAL(actionInvoked(QString)), notification1, SIGNAL(actionInvoked(QString)));
    connect(this, SIGNAL(actionInvoked(QString)), notification2, SIGNAL(actionInvoked(QString)));

    // Make both notifications emit the actionInvoked() signal for action "action1"; only the first one contains it and should be invoked
    QSignalSpy spy(manager, SIGNAL(ActionInvoked(uint, QString)));
    emit actionInvoked("action1");
    QCoreApplication::processEvents();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.last().at(0).toUInt(), id1);
    QCOMPARE(spy.last().at(1).toString(), QString("action1"));
}

void Ut_NotificationManager::testActionIsNotInvokedIfIncomplete()
{
    // Add two notifications, the first one with an incomplete action named "action1"
    NotificationManager *manager = NotificationManager::instance();
    uint id1 = manager->Notify("app1", 0, QString(), QString(), QString(), QStringList() << "action1", QVariantHash(), 0);
    uint id2 = manager->Notify("app2", 0, QString(), QString(), QString(), QStringList() << "action2" << "Action 2", QVariantHash(), 0);
    LipstickNotification *notification1 = manager->notification(id1);
    LipstickNotification *notification2 = manager->notification(id2);
    connect(this, SIGNAL(actionInvoked(QString)), notification1, SIGNAL(actionInvoked(QString)));
    connect(this, SIGNAL(actionInvoked(QString)), notification2, SIGNAL(actionInvoked(QString)));

    // Make both notifications emit the actionInvoked() signal for action "action1"; no action should be invoked
    QSignalSpy spy(manager, SIGNAL(ActionInvoked(uint, QString)));
    emit actionInvoked("action1");
    QCoreApplication::processEvents();
    QCOMPARE(spy.count(), 0);
}

void Ut_NotificationManager::testRemoteActionIsInvokedIfDefined()
{
    // Add a notifications with an action named "action"
    NotificationManager *manager = NotificationManager::instance();
    QVariantHash hints;
    hints.insert(QString(LipstickNotification::HINT_REMOTE_ACTION_PREFIX) + "action", "a b c d");
    uint id = manager->Notify("app", 0, QString(), QString(), QString(), QStringList(), hints, 0);
    LipstickNotification *notification = manager->notification(id);
    connect(this, SIGNAL(actionInvoked(QString)), notification, SIGNAL(actionInvoked(QString)));

    // Invoking the notification should trigger the remote action
    emit actionInvoked("action");
    QCoreApplication::processEvents();
    QCOMPARE(mRemoteActionTrigger.count(), 1);
    QCOMPARE(mRemoteActionTrigger.last(), hints.value(QString(LipstickNotification::HINT_REMOTE_ACTION_PREFIX) + "action").toString());
}

void Ut_NotificationManager::testInvokingActionClosesNotificationIfUserRemovable()
{
    // Add three notifications with user removability not set, set to true and set to false
    NotificationManager *manager = NotificationManager::instance();
    QVariantHash hints1;
    QVariantHash hints2;
    QVariantHash hints3;
    QVariantHash hints6;
    hints2.insert(LipstickNotification::HINT_USER_REMOVABLE, true);
    hints3.insert(LipstickNotification::HINT_USER_REMOVABLE, false);
    hints6.insert(LipstickNotification::HINT_RESIDENT, true); // 'resident' hint also prevents automatic closure
    uint id1 = manager->Notify("app1", 0, QString(), QString(), QString(), QStringList(), hints1, 0);
    uint id2 = manager->Notify("app2", 0, QString(), QString(), QString(), QStringList(), hints2, 0);
    uint id3 = manager->Notify("app3", 0, QString(), QString(), QString(), QStringList(), hints3, 0);
    uint id6 = manager->Notify("app6", 0, QString(), QString(), QString(), QStringList(), hints6, 0);
    connect(this, SIGNAL(actionInvoked(QString)), manager->notification(id1), SIGNAL(actionInvoked(QString)));
    connect(this, SIGNAL(actionInvoked(QString)), manager->notification(id2), SIGNAL(actionInvoked(QString)));
    connect(this, SIGNAL(actionInvoked(QString)), manager->notification(id3), SIGNAL(actionInvoked(QString)));
    connect(this, SIGNAL(actionInvoked(QString)), manager->notification(id6), SIGNAL(actionInvoked(QString)));

    // Make all notifications emit the actionInvoked() signal for action "action"; removable notifications should get removed
    QSignalSpy removedSpy(manager, SIGNAL(notificationRemoved(uint)));
    QSignalSpy closedSpy(manager, SIGNAL(NotificationClosed(uint,uint)));
    emit actionInvoked("action");
    QCoreApplication::processEvents();
    QCOMPARE(removedSpy.count(), 2);
    QCOMPARE(removedSpy.at(0).at(0).toUInt(), id1);
    QCOMPARE(removedSpy.at(1).at(0).toUInt(), id2);
    QCOMPARE(closedSpy.count(), 2);
    QCOMPARE(closedSpy.at(0).at(0).toUInt(), id1);
    QCOMPARE(closedSpy.at(0).at(1).toInt(), (int)NotificationManager::NotificationDismissedByUser);
    QCOMPARE(closedSpy.at(1).at(0).toUInt(), id2);
    QCOMPARE(closedSpy.at(1).at(1).toInt(), (int)NotificationManager::NotificationDismissedByUser);
}

void Ut_NotificationManager::testListingNotifications()
{
    NotificationManager *manager = NotificationManager::instance();

    // Add three notifications, two for application appName1 and one for appName2
    QVariantHash hints1;
    QVariantHash hints2;
    QVariantHash hints3;
    hints1.insert(LipstickNotification::HINT_CATEGORY, "category1");
    hints2.insert(LipstickNotification::HINT_CATEGORY, "category2");
    hints3.insert(LipstickNotification::HINT_CATEGORY, "category3");
    uint id1 = manager->Notify("appName1", 0, "appIcon1", "summary1", "body1", QStringList() << "action1", hints1, 1);
    uint id2 = manager->Notify("appName1", 0, "appIcon2", "summary2", "body2", QStringList() << "action2", hints2, 2);
    uint id3 = manager->Notify("appName2", 0, "appIcon3", "summary3", "body3", QStringList() << "action3", hints3, 3);

    Q_UNUSED(id1)
    Q_UNUSED(id2)
    Q_UNUSED(id3)
#ifdef TODO
    // Check that only notifications for the given application are returned and that they contain all the information
    QDBusArgument notifications = manager->GetNotifications("appName1");
    QCOMPARE(notifications.count(), 2);
    QCOMPARE(notifications.at(0).appName(), QString("appName1"));
    QCOMPARE(notifications.at(1).appName(), QString("appName1"));
    QCOMPARE(notifications.at(0).id(), id1);
    QCOMPARE(notifications.at(1).id(), id2);
    QCOMPARE(notifications.at(0).appIcon(), QString("appIcon1"));
    QCOMPARE(notifications.at(1).appIcon(), QString("appIcon2"));
    QCOMPARE(notifications.at(0).summary(), QString("summary1"));
    QCOMPARE(notifications.at(1).summary(), QString("summary2"));
    QCOMPARE(notifications.at(0).body(), QString("body1"));
    QCOMPARE(notifications.at(1).body(), QString("body2"));
    QCOMPARE(notifications.at(0).actions(), QStringList() << "action1");
    QCOMPARE(notifications.at(1).actions(), QStringList() << "action2");
    QCOMPARE(notifications.at(0).category(), QVariant("category1"));
    QCOMPARE(notifications.at(1).category(), QVariant("category2"));
    QCOMPARE(notifications.at(0).expireTimeout(), 1);
    QCOMPARE(notifications.at(1).expireTimeout(), 2);

    notifications = manager->GetNotifications("appName2");
    QCOMPARE(notifications.count(), 1);
    QCOMPARE(notifications.at(0).appName(), QString("appName2"));
    QCOMPARE(notifications.at(0).id(), id3);
    QCOMPARE(notifications.at(0).appIcon(), QString("appIcon3"));
    QCOMPARE(notifications.at(0).summary(), QString("summary3"));
    QCOMPARE(notifications.at(0).body(), QString("body3"));
    QCOMPARE(notifications.at(0).actions(), QStringList() << "action3");
    QCOMPARE(notifications.at(0).category(), QVariant("category3"));
    QCOMPARE(notifications.at(0).expireTimeout(), 3);
#endif
}

void Ut_NotificationManager::testRemoveUserRemovableNotifications()
{
    NotificationManager *manager = NotificationManager::instance();
    QVariantHash hints1;
    QVariantHash hints2;
    QVariantHash hints3;
    QVariantHash hints6;
    hints2.insert(LipstickNotification::HINT_USER_REMOVABLE, true);
    hints3.insert(LipstickNotification::HINT_USER_REMOVABLE, false);
    hints6.insert(LipstickNotification::HINT_RESIDENT, true);
    uint id1 = manager->Notify("app1", 0, QString(), QString(), QString(), QStringList(), hints1, 0);
    uint id2 = manager->Notify("app2", 0, QString(), QString(), QString(), QStringList(), hints2, 0);
    manager->Notify("app3", 0, QString(), QString(), QString(), QStringList(), hints3, 0);
    uint id6 = manager->Notify("app6", 0, QString(), QString(), QString(), QStringList(), hints6, 0);

    QSignalSpy removedSpy(manager, SIGNAL(notificationRemoved(uint)));
    QSignalSpy closedSpy(manager, SIGNAL(NotificationClosed(uint,uint)));
    manager->removeUserRemovableNotifications();

    QSet<uint> removedIds;
    for (int i = 0; i < removedSpy.count(); i++) {
        removedIds.insert(removedSpy.at(i).at(0).toUInt());
    }
    QCOMPARE(removedIds.count(), 3);
    QCOMPARE(removedIds.contains(id1), true);
    QCOMPARE(removedIds.contains(id2), true);
    QCOMPARE(removedIds.contains(id6), true);

    QSet<uint> closedIds;
    for (int i = 0; i < closedSpy.count(); i++) {
        closedIds.insert(closedSpy.at(i).at(0).toUInt());
        QCOMPARE(closedSpy.at(i).at(1).toInt(), (int)NotificationManager::NotificationDismissedByUser);
    }
    QCOMPARE(closedIds.count(), 3);
    QCOMPARE(closedIds.contains(id1), true);
    QCOMPARE(closedIds.contains(id2), true);
    QCOMPARE(closedIds.contains(id6), true);
}

void Ut_NotificationManager::testRemoveRequested()
{
    NotificationManager *manager = NotificationManager::instance();
    uint id1 = manager->Notify("app1", 0, QString(), QString(), QString(), QStringList() << "action1" << "Action 1", QVariantHash(), 0);
    LipstickNotification *notification1 = manager->notification(id1);
    connect(this, SIGNAL(removeRequested()), notification1, SIGNAL(removeRequested()));

    QSignalSpy removedSpy(manager, SIGNAL(notificationRemoved(uint)));
    QSignalSpy closedSpy(manager, SIGNAL(NotificationClosed(uint,uint)));
    emit removeRequested();
    QCoreApplication::processEvents();
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.last().at(0).toUInt(), id1);
    QCOMPARE(closedSpy.count(), 1);
    QCOMPARE(closedSpy.last().at(0).toUInt(), id1);
    QCOMPARE(closedSpy.last().at(1).toUInt(), static_cast<uint>(NotificationManager::NotificationDismissedByUser));
}

void Ut_NotificationManager::testImmediateExpiration()
{
    QVariantHash hints;
    hints.insert(LipstickNotification::HINT_TRANSIENT, "true");

    NotificationManager *manager = NotificationManager::instance();
    uint id1 = manager->Notify("app1", 0, QString(), QString(), QString(), QStringList(), hints, 0);

    QSignalSpy removedSpy(manager, SIGNAL(notificationRemoved(uint)));
    QSignalSpy closedSpy(manager, SIGNAL(NotificationClosed(uint, uint)));
    manager->markNotificationDisplayed(id1);
    QCoreApplication::processEvents();
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.last().at(0).toUInt(), id1);
    QCOMPARE(closedSpy.count(), 1);
    QCOMPARE(closedSpy.last().at(0).toUInt(), id1);
    QCOMPARE(closedSpy.last().at(1).toUInt(), static_cast<uint>(NotificationManager::NotificationExpired));
}

void Ut_NotificationManager::testDelayedExpiration()
{
    NotificationManager *manager = NotificationManager::instance();
    uint id1 = manager->Notify("app1", 0, QString(), QString(), QString(), QStringList(), QVariantHash(), 200);
    uint id2 = manager->Notify("app2", 0, QString(), QString(), QString(), QStringList(), QVariantHash(), 100);

    QueryValues expirationValues1, expirationValues2;
    expirationValues1.insert(0, id1);
    expirationValues1.insert(1, QDateTime::currentDateTimeUtc().toMSecsSinceEpoch() + 200);
    expirationValues2.insert(0, id2);
    expirationValues2.insert(1, QDateTime::currentDateTimeUtc().toMSecsSinceEpoch() + 100);

    qSqlQueryValues.clear();
    qSqlQueryValues["SELECT * FROM expiration"] = QueryValueList() << expirationValues1;

    QSignalSpy removedSpy(manager, SIGNAL(notificationRemoved(uint)));
    QSignalSpy closedSpy(manager, SIGNAL(NotificationClosed(uint, uint)));
    manager->markNotificationDisplayed(id1);
    QCoreApplication::processEvents();
    QCOMPARE(removedSpy.count(), 0);
    QCOMPARE(closedSpy.count(), 0);

    qSqlQueryValues["SELECT * FROM expiration"] = QueryValueList() << expirationValues1 << expirationValues2;
    manager->markNotificationDisplayed(id2);

    QCoreApplication::processEvents();
    QCOMPARE(removedSpy.count(), 0);
    QCOMPARE(closedSpy.count(), 0);

    QTRY_COMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.last().at(0).toUInt(), id2);
    QCOMPARE(closedSpy.count(), 1);
    QCOMPARE(closedSpy.last().at(0).toUInt(), id2);
    QCOMPARE(closedSpy.last().at(1).toUInt(), static_cast<uint>(NotificationManager::NotificationExpired));

    qSqlQueryValues["SELECT * FROM expiration"] = QueryValueList() << expirationValues1;

    QTRY_COMPARE(removedSpy.count(), 2);
    QCOMPARE(removedSpy.last().at(0).toUInt(), id1);
    QCOMPARE(closedSpy.count(), 2);
    QCOMPARE(closedSpy.last().at(0).toUInt(), id1);
    QCOMPARE(closedSpy.last().at(1).toUInt(), static_cast<uint>(NotificationManager::NotificationExpired));
}

QTEST_MAIN(Ut_NotificationManager)
