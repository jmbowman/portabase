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
  : QTextEdit(parent, name), oldHeight(0)
{
    setTabChangesFocus(TRUE);
    connect(this, SIGNAL(textChanged()), this, SLOT(adjustHeight()));
    setTextFormat(Qt::PlainText);
    setWordWrap(QTextEdit::NoWrap);
    setHScrollBarMode(QScrollView::AlwaysOff);
    setVScrollBarMode(QScrollView::AlwaysOff);
    adjustHeight();
}

bool DynamicEdit::adjustHeight()
{
    int newHeight = heightForWidth(width());
    if (newHeight != oldHeight) {
        setMinimumHeight(newHeight);
        setMaximumHeight(newHeight);
        oldHeight = newHeight;
        return true;
    }
    return false;
}
