/*
 * importdialog.cpp
 *
 * (c) 2002-2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qpe/fileselector.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qstringlist.h>
#include "csverror.h"
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
    else if (sourceType == OPTION_LIST) {
        caption = tr("Import from text file");
        mimeType = "text/plain";
    }
    else {
        caption = tr("Import from XML file");
        mimeType = "text/xml";
    }

    setCaption(caption + " - " + tr("PortaBase"));
    QVBoxLayout *vbox = new QVBoxLayout(this);
    if (sourceType == CSV_FILE || sourceType == OPTION_LIST) {
        QHBox *hbox = new QHBox(this);
        vbox->addWidget(hbox);
        new QLabel(tr("Text encoding"), hbox);
        encodings = new QComboBox(FALSE, hbox);
        encodings->insertItem("UTF-8");
        encodings->insertItem("Latin-1");
    }
    selector = new FileSelector(mimeType, this, "importselector",
                                FALSE, FALSE);
    vbox->addWidget(selector);
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
    QString error = "";
    QString data = "";
    if (source == CSV_FILE) {
        QStringList result = db->importFromCSV(file, encodings->currentText());
        int count = result.count();
        if (count > 0) {
            error = result[0];
        }
        if (count > 1) {
            data = result[1];
        }
    }
    else if (source == OPTION_LIST) {
        ImportUtils utils;
        error = utils.importTextLines(file, encodings->currentText(),
                                      &options);
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
        if (data == "") {
            QMessageBox::warning(this, tr("PortaBase"), error);
        }
        else {
            CSVErrorDialog dialog(error, data, this);
            dialog.exec();
        }
        return FALSE;
    }
    return TRUE;
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

QStringList ImportDialog::getOptions()
{
    return options;
}
