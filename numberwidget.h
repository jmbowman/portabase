/*
 * numberwidget.h
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef NUMBERWIDGET_H
#define NUMBERWIDGET_H

#include <qhbox.h>

class QLineEdit;

class NumberWidget: public QHBox
{
    Q_OBJECT
public:
    NumberWidget(int type, QWidget *parent = 0, const char *name = 0, WFlags f = 0);
    ~NumberWidget();

    QString getValue();
    void setValue(const QString &value);

private slots:
    void launchCalculator();

private:
    int dataType;
    QLineEdit *entryField;
};

#endif
