/*
 * colorbutton.cpp
 *
 * (c) 2003-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "colorbutton.h"

#if defined(Q_WS_QWS)
#include "colordialog.h"
#else
#include <qcolordialog.h>
#endif

ColorButton::ColorButton(const QColor &color, QWidget *parent, const char *name)
    : QPushButton(parent, name)
{
    currentColor = new QColor(color);
    background = new QColor(parent->backgroundColor());
    setPalette(QPalette(color, *background));
    connect(this, SIGNAL(clicked()), this, SLOT(launchDialog()));
}

ColorButton::~ColorButton()
{
    delete currentColor;
    delete background;
}

const QColor ColorButton::getColor()
{
    return *currentColor;
}

void ColorButton::setColor(const QColor &color)
{
    delete currentColor;
    currentColor = new QColor(color);
    setPalette(QPalette(color, *background));
}

void ColorButton::launchDialog()
{
#if defined(Q_WS_QWS)
    QColor color = ColorDialog::getColor(*currentColor, this);
#else
    QColor color = QColorDialog::getColor(*currentColor, this);
#endif
    if (color.isValid()) {
        setColor(color);
    }
}
