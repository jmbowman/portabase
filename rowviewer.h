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

#include <qstringlist.h>
#include <qvaluelist.h>
#include "datatypes.h"
#include "pbdialog.h"

class QKeyEvent;
class QPushButton;
class QTextView;
class View;
class ViewDisplay;

class RowViewer: public PBDialog
{
    Q_OBJECT
public:
    RowViewer(ViewDisplay *parent = 0, const char *name = 0, WFlags f = 0);
    ~RowViewer();

    void viewRow(View *currentView, int rowIndex);

protected:
    void keyReleaseEvent(QKeyEvent *e);

private slots:
    void nextRow();
    void previousRow();
    void editRow();

private:
    void updateContent();
    QString prepareString(QString content);

private:
    View *view;
    ViewDisplay *display;
    int index;
    int rowCount;
    QPushButton *nextButton;
    QPushButton *prevButton;
    QTextView *tv;
};

#endif
