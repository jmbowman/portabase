/*
 * datedialog.h
 *
 * (c) 2009-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file datedialog.h
 * Header file for DateDialog
 */

#ifndef DATEDIALOG_H
#define DATEDIALOG_H

#include <QDate>
#include "pbdialog.h"

class QCalendarWidget;

/**
 * Date selection dialog.  Shows one month at a time, with options to
 * navigate to other months and years.
 */
class DateDialog: public PBDialog
{
    Q_OBJECT
public:
    DateDialog(const QDate &date, QWidget *parent = 0);
    
    QDate selectedDate();

private slots:
    void gotoToday(); /**< 'Today' button handler */
    void selectNone(); /**< 'None' button handler */

private:
    QCalendarWidget *calendar; /**< Calendar display widget */
    bool noneSelected; /**< True if the 'None' button was clicked */
};

#endif
