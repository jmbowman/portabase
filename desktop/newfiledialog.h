/*
 * newfiledialog.h
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef NEWFILEDIALOG_H
#define NEWFILEDIALOG_H

#include <qstring.h>

class DocLnk;

class NewFileDialog
{
public:
    NewFileDialog(const QString & extension, QWidget *parent = 0);
    ~NewFileDialog();

    int exec();
    DocLnk *doc();
    bool encryption();

private:
    QString ext;
    QWidget *parentWidget;
    QString filename;
    bool encrypted;
};

#endif
