/*
 * qqtoolbarstretch.cpp
 *
 * (c) 2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file qqutil/qqtoolbarstretch.cpp
 * Source file for QQToolBarStretch
 */

#include <QAction>
#include <QToolBar>
#include "qqtoolbarstretch.h"

/**
 * Constructor
 *
 * @param toolbar The toolbar this widget will be added to
 * @param action The action this widget is to flank
 */
QQToolBarStretch::QQToolBarStretch(QToolBar *toolbar, QAction *action)
    : QWidget(0), linkedAction(action)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    toolbarEntry = toolbar->addWidget(this);
    connect(action, SIGNAL(changed()), this, SLOT(actionChanged()));
}

/**
 * Listen for changes in the associated action, so this widget's visibility
 * can be updated to match.
 */
void QQToolBarStretch::actionChanged()
{
    toolbarEntry->setVisible(linkedAction->isVisible());
}
