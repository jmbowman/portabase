/*
 * calcnodeeditor.h
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef CALCNODEEDITOR_H
#define CALCNODEEDITOR_H

#include <qdialog.h>
#include <qstringlist.h>

class CalcNode;
class NumberWidget;
class QButtonGroup;
class QComboBox;
class QListBox;

class CalcNodeEditor: public QDialog
{
    Q_OBJECT
public:
    CalcNodeEditor(const QStringList &colNames, int *colTypes, bool showOps,
                   QWidget *parent = 0, const char *name = 0, WFlags f = 0);
    ~CalcNodeEditor();

    CalcNode *createNode();
    void setNode(CalcNode *node);
    void reset();
    void updateNode(CalcNode *node);
    bool isValid();

private slots:
    void columnSelected(int);
    void operationSelected();

private:
    QButtonGroup *group;
    QComboBox *columnList;
    NumberWidget *number;
    QListBox *opList;
};

#endif
