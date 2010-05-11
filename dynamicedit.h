/*
 * dynamicedit.h
 *
 * (c) 2003,2008-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file dynamicedit.h
 * Header file for DynamicEdit
 */

#ifndef DYNAMICEDIT_H
#define DYNAMICEDIT_H

#include <QTextEdit>

/**
 * A Text entry widget that starts as a single line, but dynamically resizes
 * vertically as newlines are entered and removed.
 */
class DynamicEdit : public QTextEdit
{
    Q_OBJECT
public:
    DynamicEdit(QWidget *parent = 0);
    
    QSize sizeHint() const;

private slots:
    void adjustHeight();

private:
    QTextDocument *sampleDoc; /**< Sample text doc for height measurement */
};

#endif
