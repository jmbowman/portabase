/*
 * datewidget.cpp
 *
 * (c) 2002-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qdatetime.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include "database.h"
#include "datewidget.h"

#if defined(Q_WS_QWS)
#include <qpe/resource.h>
#else
#include "desktop/resource.h"
#endif

#if QT_VERSION >= 300
#include "desktop/QtaDatePicker.h"
#else
#include "QtaDatePicker.h"
#endif

DateWidget::DateWidget(QWidget *parent, const char *name, WFlags f)
    : QHBox(parent, name, f)
{
    days.append(tr("Mon"));
    days.append(tr("Tue"));
    days.append(tr("Wed"));
    days.append(tr("Thu"));
    days.append(tr("Fri"));
    days.append(tr("Sat"));
    days.append(tr("Sun"));
    months.append(tr("Jan"));
    months.append(tr("Feb"));
    months.append(tr("Mar"));
    months.append(tr("Apr"));
    months.append(tr("May"));
    months.append(tr("Jun"));
    months.append(tr("Jul"));
    months.append(tr("Aug"));
    months.append(tr("Sep"));
    months.append(tr("Oct"));
    months.append(tr("Nov"));
    months.append(tr("Dec"));
    dateObj = QDate::currentDate();
    QPushButton *button = new QPushButton(this);
    button->setPixmap(Resource::loadPixmap("portabase/calendar"));
    connect(button, SIGNAL(clicked()), this, SLOT(launchSelector()));
    display = new QLabel(toString(dateObj), this);
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
        display->setText(toString(dateObj));
        noneButton->setEnabled(TRUE);
    }
}

QString DateWidget::toString(QDate &date)
{
    QString result = tr("%1 %2 %3 %4",
                        "1=day of week, 2=month name, 3=day of month, 4=year");
    result = result.arg(days[date.dayOfWeek() - 1]);
    result = result.arg(months[date.month() - 1]);
    result = result.arg(date.day());
    return result.arg(date.year());
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
    QDatePicker selector(&tempDate, this);
    if (selector.exec()) {
        dateObj.setYMD(tempDate.year(), tempDate.month(), tempDate.day());
        updateDisplay();
    }
}
