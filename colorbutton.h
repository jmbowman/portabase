/*
 * colorbutton.h
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <qpushbutton.h>

class ColorButton: public QPushButton
{
    Q_OBJECT
public:
    ColorButton(const QColor &color, QWidget *parent = 0, const char *name = 0);
    ~ColorButton();

    const QColor getColor();
    void setColor(const QColor &color);

private slots:
    void launchDialog();

private:
    QColor *currentColor;
    QColor *background;
};

#endif
