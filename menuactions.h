/*
 * menuactions.h
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef MENUACTIONS_H
#define MENUACTIONS_H

#include <qiconset.h>
#include <qmap.h>
#include <qobject.h>
#include <qstring.h>
// included because of broken MS VC++ linker...
#include <qxml.h>
#include "dbeditor.h"

class QAction;

class MenuActions : public QObject
{
    Q_OBJECT
public:
    typedef QMap< QString, QString > PhraseMap;
    typedef QMap< QString, int > AccelMap;

    MenuActions(QObject *parent=0, const char *name=0);
    ~MenuActions();

    QString text(const QString &base);
    QString menuText(const QString &base);
    QAction *action(const QString &base, bool toggle=FALSE);
    QAction *action(const QString &base, const QIconSet &icon);

private:
    int accel(const QString &base);

private:
    PhraseMap textMap;
    PhraseMap menuTextMap;
    AccelMap accelMap;
};

#endif
