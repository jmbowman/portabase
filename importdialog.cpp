/*
 * importdialog.cpp
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qpe/fileselector.h>
#include <qmessagebox.h>
#include "database.h"
#include "importdialog.h"

ImportDialog::ImportDialog(Database *subject, QWidget *parent,
    const char *name, WFlags f) : QDialog(parent, name, TRUE, f), db(subject)
{
    setCaption(tr("Import rows from CSV file") + " - " + tr("PortaBase"));
    csvSelector = new FileSelector("text/x-csv", this, "csvselector",
                                   FALSE, FALSE );
    connect(csvSelector, SIGNAL(fileSelected(const DocLnk &)), this,
            SLOT(import(const DocLnk &)));
    showMaximized();
}

ImportDialog::~ImportDialog()
{

}

void ImportDialog::import(const DocLnk &f)
{
    QString error = db->importFromCSV(f.file());
    if (error != "") {
        QMessageBox::warning(this, tr("PortaBase"), error);
        reject();
    }
    else {
        accept();
    }
}

void ImportDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    csvSelector->resize(size());
}
