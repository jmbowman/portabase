/*
 * numberwidget.h
 *
 * (c) 2003,2008-2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file numberwidget.h
 * Header file for NumberWidget
 */

#ifndef NUMBERWIDGET_H
#define NUMBERWIDGET_H

#include <QWidget>

class QLineEdit;

/**
 * An entry widget for numeric values.  On the right side is a button which
 * launches a calculator widget, the final value of which is used as the
 * text field's value.
 */
class NumberWidget: public QWidget
{
    Q_OBJECT
public:
    NumberWidget(int type, QWidget *parent = 0);

    QString getValue();
    void setValue(const QString &value);

private slots:
    void launchCalculator();

private:
    int dataType; /**< The type of number to edit, FLOAT or INT */
    QLineEdit *entryField; /**< The text field holding the current value */
};

#endif
