/*
 * qqutil/qqhelpbrowser.h
 *
 * (c) 2003-2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file qqutil/qqhelpbrowser.h
 * Header file for QQHelpBrowser
 */

#ifndef QQHELPBROWSER_H
#define QQHELPBROWSER_H

#include "qqdialog.h"

/**
 * A simple HTML display and browsing dialog designed for showing application
 * help files.  The displayed file is assumed to use UTF-8 encoding and
 * reside in the Qt resource system.
 */
class QQHelpBrowser: public QQDialog
{
    Q_OBJECT
public:
    QQHelpBrowser(const QString &resource, QWidget *parent=0);
};

#endif
