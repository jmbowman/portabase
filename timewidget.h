/*
 * timewidget.h
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef TIMEWIDGET_H
#define TIMEWIDGET_H

#include <qhbox.h>

class QLabel;
class QLineEdit;
class QPushButton;

class TimeWidget: public QHBox
{
    Q_OBJECT
public:
    TimeWidget(QWidget *parent = 0, const char *name = 0, WFlags f = 0);
    ~TimeWidget();

    QString getTime();
    void setTime(int time);
    void setTime(QTime &time);

private slots:
    void ampmToggle();
    void noneToggle();

private:
    QLineEdit *hourEdit;
    QLineEdit *minuteEdit;
    QLineEdit *secondEdit;
    bool pm;
    QPushButton *ampmButton;
    QPushButton *noneButton;
};

#endif
