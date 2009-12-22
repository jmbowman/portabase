/*
 * datewidget.h
 *
 * (c) 2002,2008-2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
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
 * row of sub-widgets: a button which launches a date selection dialog,
 * a text label showing the currently selected date, and a button to reset
 * the date selection to "None" (blank).  Used by CalcDateEditor,
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
    DateWidget(QWidget *parent = 0);

    int getDate();
    void setDate(int date);
    void setDate(const QDate &date);

private slots:
    void launchSelector();
    void setToNone();

private:
    bool isNoneDate(const QDate &date);
    void updateDisplay();
    QString toString(const QDate &date);

private:
    QDate dateObj; /**< The date currently being shown */
    QLabel *display; /**< The text label showing the date */
    QPushButton *noneButton; /**< Button to set the date to "None" */
    QStringList days; /**< List of day-of-week abbreviations */
    QStringList months; /**< List of month names */
};

#endif
