/*
 * eventfilter.h
 *
 * (c) 2009,2015 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file eventfilter.h
 * Header file for EventFilter
 */

#ifndef EVENTFILTER_H
#define EVENTFILTER_H

#include <QObject>

class PortaBase;

/**
 * %Filter for application events which require special handling.  Currently
 * only used to handle file open requests from the Mac OS X Finder.
 * Implemented as a separate class to make sure it never gets events of its
 * own that it needs to distinguish from the incoming application events.
 */
class EventFilter : public QObject
{
    Q_OBJECT
public:
    explicit EventFilter(PortaBase *mainWindow);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    PortaBase *pb; /**< The application's main window */
};

#endif
