/*
 * vieweditor.h
 *
 * (c) 2002-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef VIEWEDITOR_H
#define VIEWEDITOR_H

#include <qstringlist.h>
#include "pbdialog.h"

class Database;
class QComboBox;
class QLineEdit;
class QListView;
class QListViewItem;
class QPoint;

class ViewEditor: public PBDialog
{
    Q_OBJECT
public:
    ViewEditor(QWidget *parent = 0, const char *name = 0);
    ~ViewEditor();

    int edit(Database *subject, const QString &viewName,
             QStringList currentCols, const QString &defaultSort,
             const QString &defaultFilter);
    void applyChanges();
    QString getName();

protected:
    void resizeEvent(QResizeEvent *event);

private:
    void updateTable();
    void selectRow(QString name);
    int isIncluded(QString name);
    bool hasValidName();

private slots:
    void tableClicked(QListViewItem *item, const QPoint &point, int column);
    void moveUp();
    void moveDown();

private:
    QLineEdit *nameBox;
    QComboBox *sortingBox;
    QComboBox *filterBox;
    QListView *table;
    Database *db;
    QString originalName;
    QStringList colNames;
    QStringList oldNames;
    QStringList includedNames;
    bool resized;
};

#endif
