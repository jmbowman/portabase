/*
 * rowviewer.h
 *
 * (c) 2002-2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef ROWVIEWER_H
#define ROWVIEWER_H

#include <qdialog.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include "datatypes.h"

class Database;
class QKeyEvent;
class QPushButton;
class QTextView;
class View;

class RowViewer: public QDialog
{
    Q_OBJECT
public:
    RowViewer(QWidget *parent = 0, const char *name = 0, WFlags f = 0);
    ~RowViewer();

    void viewRow(Database *subject, View *currentView, int rowIndex);

protected:
    void keyReleaseEvent(QKeyEvent *e);

private slots:
    void nextRow();
    void previousRow();

private:
    void updateContent();
    QString prepareString(QString content);

private:
    Database *db;
    View *view;
    int index;
    int rowCount;
    QStringList colNames;
    int *colTypes;
    QPushButton *nextButton;
    QPushButton *prevButton;
    QTextView *tv;
};

#endif
