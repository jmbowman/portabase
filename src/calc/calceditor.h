/*
 * calceditor.h
 *
 * (c) 2003-2004,2008-2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file calceditor.h
 * Header file for CalcEditor
 */

#ifndef CALCEDITOR_H
#define CALCEDITOR_H

#include <QMap>
#include <QStringList>
#include "../pbdialog.h"

class CalcDateEditor;
class CalcNode;
class CalcNodeEditor;
class CalcTimeEditor;
class Database;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTreeWidget;
class QTreeWidgetItem;

typedef QMap<QTreeWidgetItem*,CalcNode*> NodeMap;

/**
 * Dialog for editing the properties of a calculated column.  Allows the
 * user to specify the formula to be used, as well as the number of decimal
 * places to use when displaying the results of the calculation.
 */
class CalcEditor: public PBDialog
{
    Q_OBJECT
public:
    CalcEditor(Database *dbase, const QString &calcName,
               const QStringList &colNames, int *colTypes,
               QWidget *parent = 0);
    void load(CalcNode *root, int decimals);
    CalcNode *getRootNode();
    int getDecimals();

private slots:
    void addNode();
    void editNode();
    void deleteNode();
    void moveUp();
    void moveDown();
    void updateButtons();

private:
    void updateEquation();
    void addNode(QTreeWidgetItem *parent, CalcNode *node);
    void removeFromMap(QTreeWidgetItem *item);
    void swap(QTreeWidgetItem *top);

private:
    Database *db; /**< The database being edited */
    QLineEdit *equation; /**< Display for the calculation formula */
    QSpinBox *decimalsBox; /**< Field for specifying the number of decimal places to show */
    QTreeWidget *tree; /**< Tree display of the calculation components */
    NodeMap nodeMap; /**< Mapping of display tree items to calculation nodes */
    CalcNodeEditor *nodeEditor; /**< Sub-dialog for adding new nodes */
    CalcNodeEditor *valueEditor; /**< Sub-dialog for editing nodes whose type can vary */
    CalcDateEditor *dateEditor; /**< Sub-dialog for editing nodes which must be dates */
    CalcTimeEditor *timeEditor; /**< Sub-dialog for editing nodes which must be times */
};

#endif
