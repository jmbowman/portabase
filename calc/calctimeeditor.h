/*
 * calcdateeditor.h
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef CALCTIMEEDITOR_H
#define CALCTIMEEDITOR_H

#include <qstringlist.h>
#include "../pbdialog.h"

class CalcNode;
class Database;
class QButtonGroup;
class QComboBox;
class TimeWidget;

class CalcTimeEditor: public PBDialog
{
    Q_OBJECT
public:
    CalcTimeEditor(Database *dbase, const QStringList &colNames, int *colTypes,
                   QWidget *parent = 0, const char *name = 0, WFlags f = 0);
    ~CalcTimeEditor();

    CalcNode *createNode();
    void setNode(CalcNode *node);
    void reset();
    void updateNode(CalcNode *node);
    bool isValid();

private:
    Database *db;
    QButtonGroup *group;
    QComboBox *columnList;
    TimeWidget *timeWidget;
};

#endif
