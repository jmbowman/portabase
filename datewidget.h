/*
 * datewidget.h
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef DATEWIDGET_H
#define DATEWIDGET_H

#include <qdatetime.h>
#include <qhbox.h>
#include <qstringlist.h>

class Database;
class QLabel;
class QPushButton;

class DateWidget: public QHBox
{
    Q_OBJECT
public:
    DateWidget(QWidget *parent = 0, const char *name = 0, WFlags f = 0);
    ~DateWidget();

    int getDate();
    void setDate(int date);
    void setDate(QDate &date);

private slots:
    void launchSelector();
    void setToNone();

private:
    bool isNoneDate(QDate &date);
    void updateDisplay();
    QString toString(QDate &date);

private:
    QDate dateObj;
    QLabel *display;
    QPushButton *noneButton;
    QStringList days;
    QStringList months;
};

#endif
