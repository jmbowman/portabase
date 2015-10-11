/*
 * timewidget.h
 *
 * (c) 2002,2008-2009,2015 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file timewidget.h
 * Header file for TimeWidget
 */

#ifndef TIMEWIDGET_H
#define TIMEWIDGET_H

#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;

/**
 * An entry widget for time values.  Supports both 12-hour and 24-hour time
 * formats, depending on what's been selected in the application preferences.
 */
class TimeWidget: public QWidget
{
    Q_OBJECT
public:
    explicit TimeWidget(QWidget *parent = 0);

    QString getTime();
    void setTime(int time);
    void setTime(QTime &time);

private slots:
    void ampmToggle();
    void noneToggle();

private:
    QLineEdit *hourEdit; /**< Hour entry field */
    QLineEdit *minuteEdit; /**< Minute entry field */
    QLineEdit *secondEdit; /**< Second entry field */
    bool pm; /**< Is the selected time AM or PM? (if applicable) */
    QPushButton *ampmButton; /**< AM/PM selection button (0 if absent) */
    QPushButton *noneButton; /**< "None" button (for null time selection) */
};

#endif
