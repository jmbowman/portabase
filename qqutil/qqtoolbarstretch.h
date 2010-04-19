/*
 * qqtoolbarstretch.h
 *
 * (c) 2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file qqutil/qqtoolbarstretch.h
 * Header file for QQToolBarStretch
 */

#ifndef QQTOOLBARSTRETCH_H
#define QQTOOLBARSTRETCH_H

#include <QWidget>

class QAction;
class QToolBar;

/**
 * A stretcher widget which sits to one side of a toolbar button on Maemo,
 * and shows and hides itself to match the action it's attached to.  Net
 * effect, the buttons are nicely spaced across the bottom of the screen.
 */
class QQToolBarStretch : public QWidget
{
    Q_OBJECT
public:
    QQToolBarStretch(QToolBar *toolbar, QAction *action);

private slots:
    void actionChanged();

private:
    QAction *linkedAction;
    QAction *toolbarEntry;
};

#endif // QQTOOLBARSTRETCH_H

