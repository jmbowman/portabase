/*
 * sorteditor.h
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef SORTEDITOR_H
#define SORTEDITOR_H

#include <qdialog.h>

class Database;
class QLineEdit;
class QListView;
class QListViewItem;
class QPoint;
class QVBox;

class SortEditor: public QDialog
{
    Q_OBJECT
public:
    SortEditor(QWidget *parent = 0, const char *name = 0, WFlags f = 0);
    ~SortEditor();

    int edit(Database *subject, QString sortingName);
    void applyChanges();
    QString getName();

protected:
    void resizeEvent(QResizeEvent *event);

private:
    void updateTable();
    void selectRow(QString name);
    int isSorted(QString name);
    bool hasValidName();

private slots:
    void tableClicked(QListViewItem *item, const QPoint &point, int column);
    void moveUp();
    void moveDown();

private:
    QVBox *vbox;
    QLineEdit *nameBox;
    QListView *table;
    Database *db;
    QString originalName;
    QStringList colNames;
    QStringList sortCols;
    QStringList descCols;
    bool resized;
};

#endif
