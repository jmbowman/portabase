/*
 * rowviewer.h
 *
 * (c) 2002-2004,2009-2012,2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file rowviewer.h
 * Header file for RowViewer
 */

#ifndef ROWVIEWER_H
#define ROWVIEWER_H

#include <QStringList>
#include "datatypes.h"
#include "factory.h"
#include "pbdialog.h"

class Database;
class QComboBox;
class QKeyEvent;
class QAbstractButton;
class View;
class ViewDisplay;

/**
 * A dialog which displays a read-only version of a particular row's data.
 * Mimics in HTML the layout of the row editor, with column names on the left
 * and the row's values for them on the right.  It has buttons at the bottom
 * for navigating to adjacent rows (in the current filter and sorting) and
 * launching the row editor for the displayed row.
 */
class RowViewer: public PBDialog
{
    Q_OBJECT
public:
    RowViewer(Database *dbase, ViewDisplay *parent = 0);
    ~RowViewer();

    void viewRow(View *currentView, int rowIndex);

protected:
    void keyReleaseEvent(QKeyEvent *e);

private slots:
    void nextRow();
    void previousRow();
    void editRow();
    void copyRow();
    void deleteRow();
    void viewChanged(int index);

private:
    QColor alphaBlend(QColor base, QColor foreground);
    void updateContent();
    QString prepareString(const QString &content);

private:
    Database *db; /**< The database being viewed */
    View *view; /**< The database view selected in the main display */
    ViewDisplay *display; /**< The view display grid (used to launch the row editor) */
    QComboBox *viewBox; /**< Database view selection list */
    int index; /**< The current index among rows in the selected filter and sorting */
    int rowCount; /**< The total number of rows in the current filter */
    QAbstractButton *nextButton; /**< Button for viewing the next row */
    QAbstractButton *prevButton; /**< Button for viewing the previous row */
    HtmlDisplay *hd; /**< The HTML display widget used to show the row data */
    View *currentView; /**< The database view currently selected in this dialog */
};

#endif
