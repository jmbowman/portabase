/*
 * eventfilter.cpp
 *
 * (c) 2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file eventfilter.cpp
 * Source file for EventFilter
 */

#include <QFileOpenEvent>
#include "eventfilter.h"
#include "portabase.h"

/**
 * Constructor.
 *
 * @param mainWindow The application's main window (manages file opening)
 */
EventFilter::EventFilter(PortaBase *mainWindow) : pb(mainWindow)
{

}

/**
 * Look for file open events coming into the application, and handle them
 * appropriately while ignoring everything else.
 *
 * @param obj The object on which the event occurred
 * @param event The event which occurred
 * @return true if the event should be stopped, false otherwise
 */
bool EventFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FileOpen) {
        QString path = (static_cast<QFileOpenEvent *>(event))->file();
        pb->openFile(path);
        return true;
    }
    else {
        return QObject::eventFilter(obj, event);
    }
}
