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
#include "importutils.h"

ImportDialog::ImportDialog(int sourceType, Database *subject, QWidget *parent,
  const char *name, WFlags f) : QDialog(parent, name, TRUE, f), db(subject), importDone(FALSE)
{
    source = sourceType;
    QString caption;
    QString mimeType;
    if (sourceType == CSV_FILE) {
        caption = tr("Import rows from CSV file");
        mimeType = "text/x-csv";
    }
    else if (sourceType == MOBILEDB_FILE) {
        caption = tr("Import from MobileDB file");
        // the SL-5500 has this associated with .pdb...not sure about
        // Japanese models, and some ebook users replace it...sigh
        mimeType = "chemical/x-pdb";
    }
    else {
        caption = tr("Import from XML file");
        mimeType = "text/xml";
    }
    setCaption(caption + " - " + tr("PortaBase"));
    selector = new FileSelector(mimeType, this, "importselector",
                                FALSE, FALSE );
    connect(selector, SIGNAL(fileSelected(const DocLnk &)), this,
            SLOT(import(const DocLnk &)));
    showMaximized();
}

ImportDialog::~ImportDialog()
{

}

void ImportDialog::import(const DocLnk &f)
{
    if (import(f.file())) {
        accept();
    }
    else {
        reject();
    }
}

bool ImportDialog::import(const QString &file)
{
    QString error;
    if (source == CSV_FILE) {
        error = db->importFromCSV(file);
    }
    else if (source == MOBILEDB_FILE) {
        ImportUtils utils;
        error = utils.importMobileDB(file, db);
    }
    else {
        ImportUtils utils;
        error = utils.importXML(file, db);
    }
    importDone = TRUE;
    if (error != "") {
        QMessageBox::warning(this, tr("PortaBase"), error);
        return FALSE;
    }
    return TRUE;
}

void ImportDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    selector->resize(size());
}

int ImportDialog::exec()
{
    int result = QDialog::exec();
    if (result && !importDone) {
        // "OK" was clicked...see if there is a selected file
        const DocLnk *f = selector->selected();
        if (!f) {
            return QDialog::Rejected;
        }
        if (!import(f->file())) {
            result = QDialog::Rejected;
        }
        delete f;
    }
    return result;
}
