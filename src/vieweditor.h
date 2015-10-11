/*
 * vieweditor.h
 *
 * (c) 2002-2004,2009,2015 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file vieweditor.h
 * Header file for ViewEditor
 */

#ifndef VIEWEDITOR_H
#define VIEWEDITOR_H

#include <QStringList>
#include "pbdialog.h"

class Database;
class QComboBox;
class QLineEdit;
class QTreeWidget;
class QTreeWidgetItem;

/**
 * Dialog for editing a database view.
 */
class ViewEditor: public PBDialog
{
    Q_OBJECT
public:
    explicit ViewEditor(QWidget *parent = 0);

    int edit(Database *subject, const QString &viewName,
             const QStringList &currentCols, const QString &defaultSort,
             const QString &defaultFilter);
    void applyChanges();
    QString getName();

private:
    void updateTable();

private slots:
    void moveUp();
    void moveDown();

private:
    QLineEdit *nameBox; /**< Entry field for the view's name */
    QComboBox *sortingBox; /**< Default sorting name selection list */
    QComboBox *filterBox; /**< Default filter name selection list */
    QTreeWidget *table; /**< Display of the list of database columns */
    Database *db; /**< The database being edited */
    QString originalName; /**< The initial name of the view being edited */
    QStringList oldNames; /**< The initial ordered list of columns in the view */
};

#endif
