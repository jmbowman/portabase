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

#include <qdialog.h>

class QCheckBox;
class QLineEdit;

class NewFileDialog: public QDialog
{
    Q_OBJECT
public:
    NewFileDialog(QWidget *parent = 0, const char *name = 0, WFlags f = 0);
    ~NewFileDialog();

    QString name();
    bool encryption();

private:
    QLineEdit *nameBox;
    QCheckBox *encrypt;
};

#endif
