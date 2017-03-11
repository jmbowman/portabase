/*
 * sorteditor.h
 *
 * (c) 2002-2004,2009,2015,2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file sorteditor.h
 * Header file for SortEditor
 */

#ifndef SORTEDITOR_H
#define SORTEDITOR_H

#include <QModelIndex>
#include <QStringList>
#include "pbdialog.h"
#include "qqutil/qqlineedit.h"

class Database;
class QTreeWidget;
class QTreeWidgetItem;

/**
 * Dialog for editing a sorting.
 */
class SortEditor: public PBDialog
{
    Q_OBJECT
public:
    explicit SortEditor(QWidget *parent = 0);

    int edit(Database *subject, const QString &sortingName);
    void applyChanges();
    QString getName();

private:
    void updateTable(const QStringList &colNames);

private slots:
    void tableClicked(QTreeWidgetItem *item, int column);
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void moveUp();
    void moveDown();

private:
    QQLineEdit *nameBox; /**< Entry field for the sorting's name */
    QTreeWidget *table; /**< Display of the list of database columns */
    Database *db; /**< The database being edited */
    QString originalName; /**< The initial name of the filter being edited */
    QStringList sortCols; /**< Names of columns which are being sorted on */
    QStringList descCols; /**< Names of columns which are being sorted in descending order */
    bool updating; /** Indicator that model changes should be temporarily ignored */
};

#endif
