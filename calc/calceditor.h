/*
 * calceditor.h
 *
 * (c) 2003-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef CALCEDITOR_H
#define CALCEDITOR_H

#include <qmap.h>
#include <qstringlist.h>
#include "../pbdialog.h"

class CalcDateEditor;
class CalcNode;
class CalcNodeEditor;
class CalcTimeEditor;
class Database;
class QLineEdit;
class QListView;
class QListViewItem;
class QPushButton;
class QSpinBox;

typedef QMap<QListViewItem*,CalcNode*> NodeMap;

class CalcEditor: public PBDialog
{
    Q_OBJECT
public:
    CalcEditor(Database *dbase, const QString &calcName,
               const QStringList &colNames, int *colTypes,
               QWidget *parent = 0, const char *name = 0);
    ~CalcEditor();
    void load(CalcNode *root, int decimals);
    CalcNode *getRootNode();
    int getDecimals();

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void addNode();
    void editNode();
    void deleteNode();
    void moveUp();
    void moveDown();
    void updateButtons(QListViewItem *item);

private:
    void updateEquation();
    void addNode(QListViewItem *parent, CalcNode *node);
    void removeFromMap(QListViewItem *item);
    void swap(QListViewItem *top, QListViewItem *bottom);

private:
    Database *db;
    QLineEdit *equation;
    QSpinBox *decimalsBox;
    QListView *tree;
    NodeMap nodeMap;
    CalcNodeEditor *nodeEditor;
    CalcNodeEditor *valueEditor;
    CalcDateEditor *dateEditor;
    CalcTimeEditor *timeEditor;
};

#endif
