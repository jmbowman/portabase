/*
 * calcnodeeditor.h
 *
 * (c) 2003-2004,2008 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file calcnodeeditor.h
 * Header file for CalcNodeEditor
 */

#ifndef CALCNODEEDITOR_H
#define CALCNODEEDITOR_H

#include <QStringList>
#include "../pbdialog.h"

class CalcNode;
class NumberWidget;
class QButtonGroup;
class QComboBox;
class QListWidget;

/**
 * Dialog for editing the content of a calculation node.  Allows selection of
 * a column reference, a constant value, and/or an operation (depending on the
 * node's context in the overall calculation).  Date nodes are handled by
 * CalcDateEditor instead, and time nodes by CalcTimeEditor.
 */
class CalcNodeEditor: public PBDialog
{
    Q_OBJECT
public:
    CalcNodeEditor(const QStringList &colNames, int *colTypes, bool showOps,
                   QWidget *parent = 0);

    CalcNode *createNode();
    void setNode(CalcNode *node);
    void reset();
    void updateNode(CalcNode *node);
    bool isValid();

private slots:
    void columnSelected(int);
    void operationSelected();

private:
    QButtonGroup *group; /**< Grouping of the dialog's radio buttons */
    QComboBox *columnList; /**< Combo box listing selectable column references */
    NumberWidget *number; /**< Entry widget for numerical constants */
    QListWidget *opList; /**< List of available operations */
};

#endif
