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
#include <qstringlist.h>
#include "importdialog.h"
#include "qpeapplication.h"
#include "../csverror.h"
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
    QString data = "";
    if (source == CSV_FILE) {
        QStringList result = db->importFromCSV(file);
        int count = result.count();
        if (count > 0) {
            error = result[0];
        }
        if (count > 1) {
            data = result[1];
        }
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
        if (data == "") {
            QMessageBox::warning(0, QObject::tr("PortaBase"), error);
        }
        else {
            CSVErrorDialog dialog(error, data, 0);
            dialog.exec();
        }
        return FALSE;
    }
    else {
        return TRUE;
    }
}
