/*
 * qqdialog.h
 *
 * (c) 2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef QQDIALOG_H
#define QQDIALOG_H

#if !defined(Q_WS_QWS)
// get rid of "What's This" button
#define DEFAULT_FLAGS WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu
#else
#define DEFAULT_FLAGS 0
#endif

#include <qdialog.h>

class QQDialog: public QDialog
{
    Q_OBJECT
public:
    QQDialog(QString title, QWidget *parent = 0, const char *name = 0,
             bool modal = FALSE, WFlags f = DEFAULT_FLAGS);
    ~QQDialog();

protected:
    void finishConstruction(bool fullscreen=TRUE, int minWidth=-1,
                            int minHeight=-1);

public:
    static const QString titleSuffix;
};

#endif
