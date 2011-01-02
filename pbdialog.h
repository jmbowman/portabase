/*
 * pbdialog.h
 *
 * (c) 2003-2004,2008-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
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

#include <QStringList>
#include "qqutil/qqdialog.h"

class QDialogButtonBox;
class QPushButton;
class QVBoxLayout;

/**
 * Base class for many %PortaBase dialogs.  Handles standard validation for
 * names of views, sortings, filters and such.  Also supports easy addition
 * of a row of buttons for list management.
 */
class PBDialog: public QQDialog
{
    Q_OBJECT
public:
    PBDialog(QString title, QWidget *parent = 0, bool small=false);

    bool validateName(const QString &newName, const QString &oldName,
                      const QStringList &otherNames);

protected:
    QDialogButtonBox *finishLayout(bool okButton=true, bool cancelButton=true,
                                   int minWidth=-1, int minHeight=-1);
    void addEditButtons(bool movementOnly=false);

protected:
    QVBoxLayout *vbox; /**< The main layout */
    QPushButton *addButton; /**< Button for adding items to a list */
    QPushButton *editButton; /**< Button for editing items in a list */
    QPushButton *deleteButton; /**< Button for deleting items from a list */
    QPushButton *upButton; /**< Button for moving items up in a list */
    QPushButton *downButton; /**< Button for moving items down in a list */
};

#endif
