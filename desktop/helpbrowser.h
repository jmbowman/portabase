/*
 * helpbrowser.h
 *
 * (c) 2003-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef HELPBROWSER_H
#define HELPBROWSER_H

#include "pbdialog.h"

class QTextBrowser;

class HelpBrowser: public PBDialog
{
    Q_OBJECT
public:
    HelpBrowser(QWidget *parent = 0, const char *name = 0);
    ~HelpBrowser();

private slots:
    void back();
    void forward();

private:
    QTextBrowser *content;
};

#endif
