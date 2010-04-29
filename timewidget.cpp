/*
 * timewidget.cpp
 *
 * (c) 2002-2004,2008-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file timewidget.cpp
 * Source file for TimeWidget
 */

#include <QDateTime>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include "factory.h"
#include "timewidget.h"

/**
 * Constructor.
 *
 * @param parent This widget's parent widget
 */
TimeWidget::TimeWidget(QWidget *parent)
    : QWidget(parent)
{
#if defined(Q_WS_MAEMO_5)
    int maxWidth = 70;
#else
    int maxWidth = 25;
#endif
    QHBoxLayout *layout = Factory::hBoxLayout(this, true);
    hourEdit = new QLineEdit(this);
    hourEdit->setMaxLength(2);
    hourEdit->setMaximumWidth(maxWidth);
    layout->addWidget(hourEdit);
    layout->addWidget(new QLabel(" : ", this));
    minuteEdit = new QLineEdit(this);
    minuteEdit->setMaxLength(2);
    minuteEdit->setMaximumWidth(maxWidth);
    layout->addWidget(minuteEdit);
    layout->addWidget(new QLabel(" : ", this));
    secondEdit = new QLineEdit(this);
    secondEdit->setMaxLength(2);
    secondEdit->setMaximumWidth(maxWidth);
    layout->addWidget(secondEdit);
    QSettings settings;
    QString timeFormat = settings.value("DateTime/TimeFormat",
                                        "hh:mm AP").toString();
    if (timeFormat.contains("AP")) {
        ampmButton = new QPushButton(tr("AM"), this);
        layout->addWidget(ampmButton);
        pm = false;
        connect(ampmButton, SIGNAL(clicked()), this, SLOT(ampmToggle()));
    }
    else {
        ampmButton = 0;
    }
    noneButton = new QPushButton(tr("None"), this);
    noneButton->setCheckable(true);
    layout->addWidget(noneButton);
    connect(noneButton, SIGNAL(clicked()), this, SLOT(noneToggle()));
    setMaximumHeight(noneButton->sizeHint().height());
    layout->addWidget(new QWidget(this), 1);
}

/**
 * Get the currently entered time.
 *
 * @return The text representation of the selected time
 */
QString TimeWidget::getTime()
{
    if (noneButton->isChecked()) {
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

/**
 * Set the time to display.  Uses PortaBase's internal integer representation
 * of time values (number of seconds past midnight, or -1 for blank).
 *
 * @param time The time to display in this widget
 */
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
    bool none = false;
    if (time == -1) {
        none = true;
    }
    if (noneButton->isChecked() != none) {
        noneButton->setChecked(none);
        noneToggle();
    }
    if (ampmButton) {
        if (hour > 11) {
            pm = true;
            ampmButton->setText(tr("PM"));
            if (hour > 12) {
                hour -= 12;
            }
        }
        else {
            pm = false;
            ampmButton->setText(tr("AM"));
            if (hour == 0) {
                hour = 12;
            }
        }
    }
    QString hourString = QString::number(hour);
    if (!ampmButton) {
        hourString = hourString.rightJustified(2, '0');
    }
    hourEdit->setText(hourString);
    minuteEdit->setText(QString::number(minute).rightJustified(2, '0'));
    secondEdit->setText(QString::number(second).rightJustified(2, '0'));
}

/**
 * Set the time to be displayed in this widget.
 *
 * @param time The time to be displayed
 */
void TimeWidget::setTime(QTime &time)
{
    setTime(time.hour() * 3600 + time.minute() * 60 + time.second());
}

/**
 * Toggle the status of this widget between null and non-null.  Called
 * automatically when the "None" button is clicked.
 */
void TimeWidget::noneToggle()
{
    if (noneButton->isChecked()) {
        hourEdit->setEnabled(false);
        minuteEdit->setEnabled(false);
        secondEdit->setEnabled(false);
        if (ampmButton) {
            ampmButton->setEnabled(false);
        }
    }
    else {
        hourEdit->setEnabled(true);
        minuteEdit->setEnabled(true);
        secondEdit->setEnabled(true);
        if (ampmButton) {
            ampmButton->setEnabled(true);
        }
    }
}

/**
 * Toggle the selected time between AM and PM.  Called automatically when
 * the AM/PM display button is clicked.
 */
void TimeWidget::ampmToggle()
{
    pm = !pm;
    if (pm) {
        ampmButton->setText(tr("PM"));
    }
    else {
        ampmButton->setText(tr("AM"));
    }
}
