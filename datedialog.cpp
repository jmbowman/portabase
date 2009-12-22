/*
 * datedialog.cpp
 *
 * (c) 2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file datedialog.cpp
 * Source file for DateDialog
 */

#include <QCalendarWidget>
#include <QDialogButtonBox>
#include <QLayout>
#include <QPushButton>
#include <QSettings>
#include <QTextCharFormat>

#include "datedialog.h"

/**
 * Constructor.
 *
 * @param date The date to initially select in the calendar
 * @param parent This dialog's parent widget
 */
DateDialog::DateDialog(const QDate &date, QWidget *parent)
  : PBDialog(tr("Select a date"), parent)
{
    if (!date.isValid()) {
        reject();
    }
    calendar = new QCalendarWidget(this);
    // date limits are for backwards compatibility
    calendar->setMinimumDate(QDate(1752, 9, 15));
    calendar->setMaximumDate(QDate(7000, 12, 31));
    calendar->setGridVisible(true);
    calendar->setSelectedDate(date);
    QDate today = QDate::currentDate();
    QTextCharFormat format = calendar->dateTextFormat(today);
    format.setBackground(Qt::cyan);
    calendar->setDateTextFormat(QDate::currentDate(), format);
    QSettings settings;
    if (settings.value("DateTime/MONDAY", false).toBool()) {
        calendar->setFirstDayOfWeek(Qt::Monday);
    }
    vbox->addWidget(calendar);

    QDialogButtonBox *buttonBox = finishLayout();
    QPushButton *todayButton = new QPushButton(tr("Today"));
    buttonBox->addButton(todayButton, QDialogButtonBox::ActionRole);
    connect(todayButton, SIGNAL(clicked()), this, SLOT(gotoToday()));
}

/**
 * Get the currently selected date.
 *
 * @return The selected date
 */
QDate DateDialog::selectedDate()
{
    return calendar->selectedDate();
}

/**
 * Select today's date on the calendar.  Called when the "Today" button is
 * clicked.
 */
void DateDialog::gotoToday()
{
    calendar->setSelectedDate(QDate::currentDate());
}