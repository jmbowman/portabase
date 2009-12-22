/*
 * calcdateeditor.h
 *
 * (c) 2003-2004,2008 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file calcdateeditor.h
 * Header file for CalcDateEditor
 */

#ifndef CALCDATEEDITOR_H
#define CALCDATEEDITOR_H

#include <QStringList>
#include "../pbdialog.h"

class CalcNode;
class DateWidget;
class QButtonGroup;
class QComboBox;

/**
 * Dialog for specifying a date node in the formula for a calculated column.
 * Can be either a date column reference or a constant value.
 */
class CalcDateEditor: public PBDialog
{
    Q_OBJECT
public:
    CalcDateEditor(const QStringList &colNames, int *colTypes,
                   QWidget *parent = 0);

    CalcNode *createNode();
    void setNode(CalcNode *node);
    void reset();
    void updateNode(CalcNode *node);

private:
    QButtonGroup *group; /**< Radio button group (Column/Constant) */
    QComboBox *columnList; /**< Widget for selecting a date column */
    DateWidget *dateWidget; /**< Widget for specifying a date constant */
};

#endif
