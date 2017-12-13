/*
 * qqtreeview.cpp
 *
 * (c) 2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file qqutil/qqtreeview.cpp
 * Source file for QQTreeView
 */

#include "qqtreeview.h"

#if defined(Q_OS_ANDROID)

#include <QApplication>
#include <QEvent>
#include <QInputMethodQueryEvent>
#include <QKeyEvent>

/**
 * Constructor.
 *
 * @param parent This tree view's parent widget
 */
QQTreeView::QQTreeView(QWidget *parent)
    : QTreeView(parent)
{

}

/**
 * This is the main event handler; it handles event e.  Overridden to make
 * sure that queries for the software keyboard enter key icon are answered
 * correctly.
 *
 * @param e The event to be processed
 * @return True if the event was accepted, false otherwise
 */
bool QQTreeView::event(QEvent *e)
{
    if (e->type() == QEvent::InputMethodQuery) {
        // Setting this in the constructor doesn't stick
        setAttribute(Qt::WA_InputMethodEnabled, true);
    }
    return QTreeView::event(e);
}

/**
 * This method is used by the input method to query a set of properties of
 * the widget to be able to support complex input method operations such as
 * support for surrounding text and reconversions.  Overridden to show the
 * correct action button in Android's software keyboard.
 *
 * @param query The property being queried
 * @return The value of the requested property
 */
QVariant QQTreeView::inputMethodQuery(Qt::InputMethodQuery query) const
{
    if (query == Qt::ImEnterKeyType) {
        return Qt::EnterKeySearch;
    }
    return QTreeView::inputMethodQuery(query);
}

/**
 * Handler for keyboard key press events.  Ensures that pressing the
 * search button in incremental search mode goes to the next result instead
 * of launching the row viewer for the currently selected row.
 *
 * @param e The keyboard event which occurred
 */
void QQTreeView::keyPressEvent(QKeyEvent *e)
{
    int key = e->key();
    if (key == Qt::Key_Enter || key == Qt::Key_Return) {
        if (qApp->inputMethod()->isVisible()) {
            qApp->inputMethod()->hide();
            e->accept();
            return;
        }
    }
    QTreeView::keyPressEvent(e);
}

#endif // Q_OS_ANDROID
