/*
 * pbdialog.h
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef PBDIALOG_H
#define PBDIALOG_H

#include <qdialog.h>
#include <qlayout.h>
#include <qpushbutton.h>

class PBDialog: public QDialog
{
    Q_OBJECT
public:
    PBDialog(QString title, QWidget *parent = 0, const char *name = 0,
             WFlags f = 0);
    ~PBDialog();

protected:
    void finishLayout(bool okButton=TRUE, bool cancelButton=TRUE,
                      bool fullscreen=TRUE, int minWidth=-1,
                      int minHeight=-1);
    void addEditButtons(bool movementOnly=FALSE);

public:
    static const QString titleSuffix;

protected:
    QVBoxLayout *vbox;
    QPushButton *addButton;
    QPushButton *editButton;
    QPushButton *deleteButton;
    QPushButton *upButton;
    QPushButton *downButton;
};

#endif
