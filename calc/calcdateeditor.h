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

#ifndef CALCDATEEDITOR_H
#define CALCDATEEDITOR_H

#include <qdialog.h>
#include <qstringlist.h>

class CalcNode;
class DateWidget;
class QButtonGroup;
class QComboBox;

class CalcDateEditor: public QDialog
{
    Q_OBJECT
public:
    CalcDateEditor(const QStringList &colNames, int *colTypes,
                   QWidget *parent = 0, const char *name = 0, WFlags f = 0);
    ~CalcDateEditor();

    CalcNode *createNode();
    void setNode(CalcNode *node);
    void reset();
    void updateNode(CalcNode *node);

private:
    QButtonGroup *group;
    QComboBox *columnList;
    DateWidget *dateWidget;
};

#endif
