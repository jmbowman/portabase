/*
 * datewidget.cpp
 *
 * (c) 2002-2004,2008-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file datewidget.cpp
 * Source file for DateWidget
 */

#include <QAbstractButton>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include "database.h"
#include "datedialog.h"
#include "datewidget.h"
#include "factory.h"

/**
 * Constructor.
 *
 * @param parent This widget's parent widget
 */
DateWidget::DateWidget(QWidget *parent)
    : QWidget(parent)
{
    dateObj = QDate::currentDate();
    QHBoxLayout *layout = Factory::hBoxLayout(this, true);

    QAbstractButton *button = Factory::button(this);
    button->setIcon(QIcon(":/icons/calendar.png"));
    connect(button, SIGNAL(clicked()), this, SLOT(launchSelector()));
    layout->addWidget(button);

    display = new QLabel(toString(dateObj), this);
    display->setAlignment(Qt::AlignCenter);
    layout->addWidget(display, 1);

    noneButton = new QPushButton(tr("None"), this);
    connect(noneButton, SIGNAL(clicked()), this, SLOT(setToNone()));
    layout->addWidget(noneButton);
}

/**
 * Get the integer representation of the currently selected date.
 *
 * @return The date which was last set or selected
 */
int DateWidget::getDate()
{
    return dateObj.year() * 10000 + dateObj.month() * 100 + dateObj.day();
}

/**
 * Set the selected date to the specified value.
 *
 * @param date The integer representation of the date to display
 */
void DateWidget::setDate(int date)
{
    if (date == 0) {
        QDate today = QDate::currentDate();
        dateObj.setYMD(today.year(), today.month(), today.day());
    }
    else {
        int y = date / 10000;
        int m = (date - y * 10000) / 100;
        int d = date - y * 10000 - m * 100;
        dateObj.setYMD(y, m, d);
    }
    updateDisplay();
}

/**
 * Set the selected date to the specified value.
 *
 * @param date The date to display
 */
void DateWidget::setDate(const QDate &date)
{
    dateObj.setYMD(date.year(), date.month(), date.day());
    updateDisplay();
}

/**
 * Update the text label and clickability of the "None" button based on
 * the currently selected date.
 */
void DateWidget::updateDisplay()
{
    if (isNoneDate(dateObj)) {
        display->setText("               ");
        noneButton->setEnabled(false);
    }
    else {
        display->setText(toString(dateObj));
        noneButton->setEnabled(true);
    }
}

/**
 * Get a text representation of the provided date, suitable for display in
 * this widget's text label.
 *
 * @param date The date to be converted
 * @return A brief text form of the input date
 */
QString DateWidget::toString(const QDate &date)
{
#if defined(Q_WS_MAC)
    QString value("%1, %2");
    value = value.arg(QDate::longDayName(date.dayOfWeek()));
    return value.arg(date.toString(Qt::DefaultLocaleLongDate));
#else
    return date.toString(Qt::DefaultLocaleLongDate);
#endif
}

/**
 * Determine if the provided date is the value that PortaBase uses to
 * represent a "None" date or not.  The first day supported by QDate in older
 * versions of Qt is used for this purpose, because it is a valid QDate value
 * that very few people would actually want to enter in a database.
 *
 * @param date The date to be checked
 * @return True if the input date is 1752-09-14, false otherwise.
 */
bool DateWidget::isNoneDate(const QDate &date)
{
    return (date.year() == 1752 && date.month() == 9 && date.day() == 14);
}

/**
 * Set the selected date to the special "None" value (the first day supported
 * in earlier versions of QDate).  Called when the "None" button is clicked.
 */
void DateWidget::setToNone()
{
    dateObj.setYMD(1752, 9, 14);
    updateDisplay();
}

/**
 * Launches a DateDialog which is preset to the currently selected date,
 * and uses the new selection from the dialog as the new date selection if
 * appropriate.
 */
void DateWidget::launchSelector()
{
    QDate tempDate(dateObj.year(), dateObj.month(), dateObj.day());
    if (isNoneDate(tempDate)) {
        QDate today = QDate::currentDate();
        tempDate.setYMD(today.year(), today.month(), today.day());
    }
    DateDialog selector(tempDate, this);
    if (selector.exec()) {
        tempDate = selector.selectedDate();
        dateObj.setYMD(tempDate.year(), tempDate.month(), tempDate.day());
        updateDisplay();
    }
}
