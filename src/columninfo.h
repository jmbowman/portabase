/*
 * columninfo.h
 *
 * (c) 2011-2012,2015 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file
 * Header file for ColumnInfoDialog
 */

#ifndef COLUMN_INFO_H
#define COLUMN_INFO_H

#include "factory.h"
#include "pbdialog.h"

class QComboBox;
class QSpinBox;
class QTextEdit;
class View;

class Database;

/**
 * Column information dialog.  Shows statistics about the data in the
 * column and allows the column display width to be edited.  Can be opened
 * for a specific column or allow the user to select from a list of all
 * columns in the current view.
 */
class ColumnInfoDialog: public PBDialog
{
    Q_OBJECT
public:
    explicit ColumnInfoDialog(QWidget *parent = 0);

    bool launch(View *currentView, const QString &colName=QString::null);

private slots:
    void columnSelected(int index);
    void widthChanged(int newWidth);

private:
    View *view; /**< The currently-selected database view */
    QWidget *topRow; /**< The column selection row, which is sometimes hidden */
    QComboBox *columns; /**< List of all columns in the table */
    HtmlDisplay *display; /**< Display widget for column statistics */
    QSpinBox *colWidth; /**< Control for the column's display width */
    bool edited; /**< True if any column widths have been changed */
};
#endif
