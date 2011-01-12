/*
 * propertiesdialog.h
 *
 * (c) 2011 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file
 * Header file for PropertiesDialog
 */

#ifndef PROPERTIES_DIALOG_H
#define PROPERTIES_DIALOG_H

#include "pbdialog.h"

class Database;
class ViewDisplay;

/**
 * File properties dialog.  Shows information from the file system, counts
 * of rows, columns, filters, etc., and a button for launching the column
 * statistics dialog.
 */
class PropertiesDialog: public PBDialog
{
    Q_OBJECT
public:
    PropertiesDialog(const QString &filePath, Database *db,
                     ViewDisplay *viewer, QWidget *parent = 0);

private slots:
    void showColumnStatistics();

private:
    ViewDisplay *vd;
};
#endif
