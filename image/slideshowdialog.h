/*
 * slideshowdialog.h
 *
 * (c) 2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef SLIDESHOWDIALOG_H
#define SLIDESHOWDIALOG_H

#include <qstringlist.h>
#include "pbdialog.h"

class ImageWidget;
class QComboBox;
class QSpinBox;
class View;

class SlideshowDialog: public PBDialog
{
    Q_OBJECT
public:
    SlideshowDialog(QStringList columns, View *view, QWidget *parent = 0,
                    const char *name = 0, WFlags f = 0);
    ~SlideshowDialog();

protected slots:
    void accept();

private:
    QComboBox *columnList;
    QSpinBox *delayBox;
    View *currentView;
    ImageWidget *fullScreen;
};

#endif
