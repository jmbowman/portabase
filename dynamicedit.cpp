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

#include <qapplication.h>
#include "dynamicedit.h"

DynamicEdit::DynamicEdit(QWidget *parent, const char *name)
  : QMultiLineEdit(parent, name), oldHeight(1)
{
    clearTableFlags(Tbl_autoVScrollBar|Tbl_autoHScrollBar);
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

void DynamicEdit::keyPressEvent(QKeyEvent *e)
{
    // tab moves between fields instead of adding a tab to the text
    int key = e->key();
    if (key == Qt::Key_Tab || key == Qt::Key_Backtab) {
        QTableView::event(e);
    }
    else {
        QMultiLineEdit::keyPressEvent(e);
    }
}

bool DynamicEdit::focusNextPrevChild(bool next)
{
    // bypass dummy implementation in QMultiLineEdit
    return QWidget::focusNextPrevChild(next);
}

// QMultiLineEdit implementation return value is too wide
QSize DynamicEdit::sizeHint() const
{
    constPolish();
    QFontMetrics fm(font());
    int h = fm.lineSpacing() * 5 + fm.height() + frameWidth() * 2;
    int w = fm.width('x') * 5;

    int maxh = maximumSize().height();
    if (maxh < QWIDGETSIZE_MAX) {
	h = maxh;
    }
    return QSize(w, h).expandedTo(QApplication::globalStrut());
}
