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

#include <sl/slfiledialog.h>

class DocLnk;

class NewFileDialog: public SlFileDialog
{
    Q_OBJECT
public:
    NewFileDialog(const QString &extension, QWidget *parent = 0,
                  const char *name = 0, WFlags f = 0);
    ~NewFileDialog();

    DocLnk *doc();
    bool encryption();
    int exec();

protected slots:
    void accept();

private:
    QString mimeType();

private:
    QString ext;
    bool encrypted;
};

#endif
