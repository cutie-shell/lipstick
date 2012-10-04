/***************************************************************************
**
** Copyright (C) 2012 Jolla Ltd.
** Contact: Robin Burchell <robin.burchell@jollamobile.com>
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
#ifndef UT_NOTIFICATIONHINTS_H
#define UT_NOTIFICATIONHINTS_H

#include <QObject>

class Ut_NotificationHints : public QObject
{
    Q_OBJECT

private slots:
    void testGettersAndSetters();
    void testDBusSerialization();
};

#endif