/*
 * datewidget.h
 *
 * (c) 2002,2008-2010,2015 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file datewidget.h
 * Header file for DateWidget
 */

#ifndef DATEWIDGET_H
#define DATEWIDGET_H

#include <QDateTime>
#include <QWidget>

class Database;
class QLabel;
class QPushButton;

/**
 * A widget for selecting and displaying a single date.  It consists of a
 * row of sub-widgets: a text label showing the currently selected date and a
 * button which launches a date selection dialog.  Used by CalcDateEditor,
 * ConditionEditor, and RowEditor.
 * <p>
 * Note that some of this class's methods use an integer representation for
 * dates; in these cases, the date is encoded using the integer's digits
 * like YYYYMMDD.  For example, February 29th of 2000 would be the number
 * 20000229.
 */
class DateWidget: public QWidget
{
    Q_OBJECT
public:
    explicit DateWidget(QWidget *parent = 0);

    int getDate();
    void setDate(int date);
    void setDate(const QDate &date);

private slots:
    void launchSelector();

private:
    void updateDisplay();
    QString toString(const QDate &date);

private:
    QDate dateObj; /**< The date currently being shown */
    QLabel *display; /**< The text label showing the date */
};

#endif
