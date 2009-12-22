/*
 * calcdateeditor.h
 *
 * (c) 2003-2004,2008-2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file calctimeeditor.h
 * Header file for CalcTimeEditor
 */

#ifndef CALCTIMEEDITOR_H
#define CALCTIMEEDITOR_H

#include <QStringList>
#include "../pbdialog.h"

class CalcNode;
class Database;
class QButtonGroup;
class QComboBox;
class TimeWidget;

/**
 * Dialog for specifying a time node in the formula for a calculated column.
 * Can be either a time column reference or a constant value.
 */
class CalcTimeEditor: public PBDialog
{
    Q_OBJECT
public:
    CalcTimeEditor(Database *dbase, const QStringList &colNames, int *colTypes,
                   QWidget *parent = 0);

    CalcNode *createNode();
    void setNode(CalcNode *node);
    void reset();
    void updateNode(CalcNode *node);
    bool isValid();

private:
    Database *db; /**< The database which is being edited */
    QButtonGroup *group; /**< Grouping of the dialog's radio buttons */
    QComboBox *columnList; /**< Widget for selecting a time column */
    TimeWidget *timeWidget; /**< Entry widget for time constants */
};

#endif
