/*
 * calcwidget.h
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef CALCWIDGET_H
#define CALCWIDGET_H

#include <qhbox.h>
#include <qstringlist.h>

class Database;
class QLabel;
class RowEditor;

class CalcWidget: public QHBox
{
    Q_OBJECT
public:
    CalcWidget(Database *dbase, const QString &calcName,
               const QStringList &colNames, RowEditor *editor,
               QWidget *parent = 0, const char *name = 0, WFlags f = 0);
    ~CalcWidget();

    QString getValue();
    void setValue(const QString &value);

public slots:
    void calculate();

private:
    Database *db;
    QString colName;
    QStringList columns;
    RowEditor *rowEditor;
    QLabel *display;
};

#endif
