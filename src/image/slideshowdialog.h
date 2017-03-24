/*
 * slideshowdialog.h
 *
 * (c) 2004,2009-2010,2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file slideshowdialog.h
 * Header file for SlideshowDialog
 */

#ifndef SLIDESHOWDIALOG_H
#define SLIDESHOWDIALOG_H

#include <QStringList>
#include "../pbdialog.h"
#include "../qqutil/qqspinbox.h"

class QComboBox;
class View;

/**
 * Dialog which shows the available options for an image slideshow, and then
 * begins it if requested.  Primarily allows selecting the column of images
 * to iterate over and the number of seconds to wait between each image.
 */
class SlideshowDialog: public PBDialog
{
    Q_OBJECT
public:
    SlideshowDialog(QStringList columns, View *view, QWidget *parent = 0);

protected slots:
    void accept();

private:
    QComboBox *columnList; /**< List of the available image columns */
    QQSpinBox *delayBox; /**< The current delay in seconds */
    View *currentView; /**< The database view currently in use */
};

#endif
