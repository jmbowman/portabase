/*
 * dynamicedit.cpp
 *
 * (c) 2003,2008-2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file dynamicedit.cpp
 * Source file for DynamicEdit
 */

#include <QApplication>
#include <QFontMetrics>
#include "dynamicedit.h"

/**
 * Constructor.
 *
 * @param parent This widget's parent widget, if any
 */
DynamicEdit::DynamicEdit(QWidget *parent)
  : QTextEdit(parent)
{
    setTabChangesFocus(true);
    setFixedHeight(sizeHint().height());
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(this, SIGNAL(textChanged()), this, SLOT(adjustHeight()));
    setAcceptRichText(false);
    setLineWrapMode(QTextEdit::NoWrap);
    setWordWrapMode(QTextOption::NoWrap);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

/**
 * Determine the preferred size of this widget based on its current text
 * content.
 *
 * @return The current preferred size
 */
QSize DynamicEdit::sizeHint() const
{
    QString content = toPlainText();
    if (content.isEmpty()) {
        QSize size = qApp->fontMetrics().size(0, "Ag");
        size.setWidth(100);
        size.setHeight(size.height());
        return size;
    }
    qreal rheight = document()->size().rheight();
    int height = qRound(rheight);
    // Want to round up, not down
    if (rheight > height) {
        height++;
    }
    return QSize(100, height);
}

/**
 * Recalculate the height required to contain all the lines of text entered
 * so far.  Called whenever the content of the text field changes.
 */
void DynamicEdit::adjustHeight()
{
    setFixedHeight(sizeHint().height());
    updateGeometry();
}
