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
#ifndef UT_NOTIFICATIONLISTMODEL_H
#define UT_NOTIFICATIONLISTMODEL_H

#include <QObject>

class Ut_NotificationListModel : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void testSignalConnections();
    void testModelPopulatesOnConstruction();
    void testNotificationIsOnlyAddedIfNotAlreadyAdded();
    void testNotificationIsNotAddedIfNoSummaryOrBody_data();
    void testNotificationIsNotAddedIfNoSummaryOrBody();
    void testAlreadyAddedNotificationIsRemovedIfNoLongerAddable();
    void testNotificationRemoval();
    void testNotificationOrdering();
    void testNotificationUpdate();
    void testRemoteActions();
};

#endif
