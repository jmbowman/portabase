/*
 * qqtreeview.h
 *
 * (c) 2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file qqutil/qqtreeview.h
 * Header file for QQTreeView
 */


#include <QTreeView>

#if !defined(ANDROID)
typedef QTreeView QQTreeView;
#else
#ifndef QQTREEVIEW_H
#define QQTREEVIEW_H

class QEvent;
class QKeyEvent;

/**
 * Subclass of QTreeView which shows a search icon as the enter key on the
 * Android software keyboard.  Used to trigger an incremental search on
 * values in the first column of the current view and filter, instead of
 * showing the Row Viewer for the current record as the default "Enter" key
 * would do.
 */
class QQTreeView: public QTreeView
{
    Q_OBJECT
public:
    explicit QQTreeView(QWidget *parent = 0);

    bool event(QEvent *e);
    QVariant inputMethodQuery(Qt::InputMethodQuery query) const;

protected:
    void keyPressEvent(QKeyEvent *e);
};
#endif // QQTREEVIEW_H
#endif // ANDROID
