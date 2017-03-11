/*
 * pbdialog.h
 *
 * (c) 2003-2004,2008-2010,2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file pbdialog.h
 * Header file for PBDialog
 */

#ifndef PBDIALOG_H
#define PBDIALOG_H

#include <QMap>
#include <QStringList>
#include "qqutil/qqdialog.h"

class QAction;
class QDialogButtonBox;
class QHBoxLayout;
class QPushButton;

/**
 * Base class for many %PortaBase dialogs.  Handles standard validation for
 * names of views, sortings, filters and such.  Also supports easy addition
 * of a row of buttons for list management.
 */
class PBDialog: public QQDialog
{
    Q_OBJECT
public:
    PBDialog(QString title, QWidget *parent = 0, bool small=false, bool backButtonAccepts=true);

    bool validateName(const QString &newName, const QString &oldName,
                      const QStringList &otherNames);

protected:
    void addButton(QAction* action, QHBoxLayout *hbox);
    QDialogButtonBox *finishLayout(bool okButton=true, bool cancelButton=true,
                                   int minWidth=-1, int minHeight=-1);
    void addEditButtons(bool movementOnly=false);

private slots:
    void actionChanged();

protected:
    QAction *addAction; /**< Action for adding items to a list */
    QAction *editAction; /**< Action for editing items in a list */
    QAction *deleteAction; /**< Action for deleting items from a list */
    QAction *upAction; /**< Action for moving items up in a list */
    QAction *downAction; /**< Action for moving items down in a list */

private:
    QMap<QAction*, QPushButton*> actionButtonMap; /**< Mapping of actions to button widgets */
};

#endif
