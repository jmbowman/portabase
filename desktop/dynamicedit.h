/*
 * dynamicedit.h
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef DYNAMICEDIT_H
#define DYNAMICEDIT_H

#include <qtextedit.h>

class DynamicEdit : public QTextEdit
{
    Q_OBJECT

public:
    DynamicEdit(QWidget *parent = 0, const char *name = 0);

private slots:
    bool adjustHeight();

private:
    int oldHeight;
};

#endif
