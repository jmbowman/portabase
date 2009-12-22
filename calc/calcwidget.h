/*
 * calcwidget.h
 *
 * (c) 2003,2008 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file calcwidget.h
 * Header file for CalcWidget
 */

#ifndef CALCWIDGET_H
#define CALCWIDGET_H

#include <QStringList>
#include <QWidget>

class Database;
class QLabel;
class RowEditor;

/**
 * Widget for displaying the current value of a calculated column in the
 * RowEditor dialog.
 */
class CalcWidget: public QWidget
{
    Q_OBJECT
public:
    CalcWidget(Database *dbase, const QString &calcName,
               const QStringList &colNames, RowEditor *editor,
               QWidget *parent = 0);

    QString getValue();
    void setValue(const QString &value);

public slots:
    void calculate();

private:
    Database *db; /**< The database being edited */
    QString colName; /**< The name of the column this widget represents */
    QStringList columns; /**< Ordered list of all the database's column names */
    RowEditor *rowEditor; /**< The RowEditor dialog this widget is in */
    QLabel *display; /**< Display label for the calculation result */
};

#endif
