/*
 * datewidget.cpp
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qpe/resource.h>
#include <qdatetime.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include "QtaDatePicker.h"
#include "database.h"
#include "datewidget.h"

DateWidget::DateWidget(QWidget *parent, const char *name, WFlags f)
    : QHBox(parent, name, f)
{
    dateObj = QDate::currentDate();
    QPushButton *button = new QPushButton(this);
    button->setPixmap(Resource::loadPixmap("portabase/calendar"));
    connect(button, SIGNAL(clicked()), this, SLOT(launchSelector()));
    display = new QLabel(dateObj.toString(), this);
    int height = button->height();
    button->setMaximumWidth(height);
    setMaximumHeight(height);
    noneButton = new QPushButton(tr("None"), this);
    connect(noneButton, SIGNAL(clicked()), this, SLOT(setToNone()));
}

DateWidget::~DateWidget()
{

}

int DateWidget::getDate()
{
    return dateObj.year() * 10000 + dateObj.month() * 100 + dateObj.day();
}

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

void DateWidget::setDate(QDate &date)
{
    dateObj.setYMD(date.year(), date.month(), date.day());
    updateDisplay();
}

void DateWidget::updateDisplay()
{
    if (isNoneDate(dateObj)) {
        display->setText("               ");
        noneButton->setEnabled(FALSE);
    }
    else {
        display->setText(dateObj.toString());
        noneButton->setEnabled(TRUE);
    }
}

bool DateWidget::isNoneDate(QDate &date)
{
    return (date.year() == 1752 && date.month() == 9 && date.day() == 14);
}

void DateWidget::setToNone()
{
    dateObj.setYMD(1752, 9, 14);
    updateDisplay();
}

void DateWidget::launchSelector()
{
    QDate tempDate(dateObj.year(), dateObj.month(), dateObj.day());
    if (isNoneDate(tempDate)) {
        QDate today = QDate::currentDate();
        tempDate.setYMD(today.year(), today.month(), today.day());
    }
    QDatePicker selector(&tempDate);
    if (selector.exec()) {
        dateObj.setYMD(tempDate.year(), tempDate.month(), tempDate.day());
        updateDisplay();
    }
}
