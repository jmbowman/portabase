/*
 * timewidget.cpp
 *
 * (c) 2002-2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#if defined(DESKTOP)
#include "desktop/config.h"
#else
#include <qpe/config.h>
#endif

#include <qdatetime.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include "timewidget.h"

TimeWidget::TimeWidget(QWidget *parent, const char *name, WFlags f)
    : QHBox(parent, name, f)
{
    hourEdit = new QLineEdit(this);
    hourEdit->setMaxLength(2);
    hourEdit->setMaximumWidth(25);
    new QLabel(" : ", this);
    minuteEdit = new QLineEdit(this);
    minuteEdit->setMaxLength(2);
    minuteEdit->setMaximumWidth(25);
    new QLabel(" : ", this);
    secondEdit = new QLineEdit(this);
    secondEdit->setMaxLength(2);
    secondEdit->setMaximumWidth(25);
    Config qpeConfig("qpe");
    qpeConfig.setGroup("Time");
    if (qpeConfig.readBoolEntry("AMPM")) {
        ampmButton = new QPushButton(tr("AM"), this);
        pm = FALSE;
        connect(ampmButton, SIGNAL(clicked()), this, SLOT(ampmToggle()));
    }
    else {
        ampmButton = 0;
    }
    noneButton = new QPushButton(tr("None"), this);
    noneButton->setToggleButton(TRUE);
    connect(noneButton, SIGNAL(clicked()), this, SLOT(noneToggle()));
    setMaximumHeight(noneButton->height());
    QWidget *filler = new QWidget(this);
    setStretchFactor(filler, 1);
}

TimeWidget::~TimeWidget()
{

}

QString TimeWidget::getTime()
{
    if (noneButton->isOn()) {
        return "-1";
    }
    QString value = hourEdit->text() + ":" + minuteEdit->text() + ":";
    value += secondEdit->text();
    if (ampmButton) {
        if (pm) {
            value += " PM";
        }
        else {
            value += " AM";
        }
    }
    return value;
}

void TimeWidget::setTime(int time)
{
    int hour = 0;
    int minute = 0;
    int second = 0;
    if (time < 0) {
        QTime now = QTime::currentTime();
        hour = now.hour();
        minute = now.minute();
        second = now.second();
    }
    else {
        hour = time / 3600;
        minute = (time - hour * 3600) / 60;
        second = time - hour * 3600 - minute * 60;
    }
    bool none = FALSE;
    if (time == -1) {
        none = TRUE;
    }
    if (noneButton->isOn() != none) {
        noneButton->setOn(none);
        noneToggle();
    }
    if (ampmButton) {
        if (hour > 11) {
            pm = TRUE;
            ampmButton->setText(tr("PM"));
            if (hour > 12) {
                hour -= 12;
            }
        }
        else {
            pm = FALSE;
            ampmButton->setText(tr("AM"));
            if (hour == 0) {
                hour = 12;
            }
        }
    }
    QString hourString = QString::number(hour);
    if (!ampmButton) {
        hourString = hourString.rightJustify(2, '0');
    }
    hourEdit->setText(hourString);
    minuteEdit->setText(QString::number(minute).rightJustify(2, '0'));
    secondEdit->setText(QString::number(second).rightJustify(2, '0'));
}

void TimeWidget::setTime(QTime &time)
{
    setTime(time.hour() * 3600 + time.minute() * 60 + time.second());
}

void TimeWidget::noneToggle()
{
    if (noneButton->isOn()) {
        hourEdit->setEnabled(FALSE);
        minuteEdit->setEnabled(FALSE);
        secondEdit->setEnabled(FALSE);
        if (ampmButton) {
            ampmButton->setEnabled(FALSE);
        }
    }
    else {
        hourEdit->setEnabled(TRUE);
        minuteEdit->setEnabled(TRUE);
        secondEdit->setEnabled(TRUE);
        if (ampmButton) {
            ampmButton->setEnabled(TRUE);
        }
    }
}

void TimeWidget::ampmToggle()
{
    pm = !pm;
    if (pm) {
        ampmButton->setText("PM");
    }
    else {
        ampmButton->setText("AM");
    }
}
