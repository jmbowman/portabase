/*
 * importdialog.cpp
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qmessagebox.h>
#include <qobject.h>
#include "importdialog.h"
#include "qpeapplication.h"
#include "../database.h"
#include "../importutils.h"

ImportDialog::ImportDialog(int sourceType, Database *subject, QWidget *parent) : db(subject), parentWidget(parent), source(sourceType)
{

}

ImportDialog::~ImportDialog()
{

}

bool ImportDialog::exec()
{
    QString filter;
    if (source == CSV_FILE) {
        filter = QObject::tr("Text files with comma separated values")
                     + " (*.csv)";
    }
    else if (source == MOBILEDB_FILE) {
        filter = QObject::tr("MobileDB files") + " (*.pdb)";
    }
    else {
        filter = QObject::tr("XML files") + " (*.xml)";
    }
    QString file = QFileDialog::getOpenFileName(QPEApplication::documentDir(),
                                                filter, parentWidget,
                                                "import dialog",
                                                QObject::tr("Choose a file"));
    if (file.isEmpty()) {
        return FALSE;
    }
    else {
        QFileInfo info(file);
        QPEApplication::setDocumentDir(info.dirPath(TRUE));
    }
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
    if (error != "") {
        QMessageBox::warning(0, QObject::tr("PortaBase"), error);
        return FALSE;
    }
    else {
        return TRUE;
    }
}
