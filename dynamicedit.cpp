/*
 * dynamicedit.cpp
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "dynamicedit.h"

DynamicEdit::DynamicEdit(QWidget *parent, const char *name)
  : QMultiLineEdit(parent, name), oldHeight(1)
{
    connect(this, SIGNAL(textChanged()), this, SLOT(adjustHeight()));
    setFixedVisibleLines(1);
}

bool DynamicEdit::adjustHeight()
{
    int newHeight = numLines();
    if (newHeight != oldHeight) {
        setFixedVisibleLines(newHeight);
        oldHeight = newHeight;
        return true;
    }
    return false;
}
