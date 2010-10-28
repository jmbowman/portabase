/*
 * importdialog.cpp
 *
 * (c) 2003-2004,2008-2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file importdialog.cpp
 * Source file for ImportDialog
 */

#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QStringList>
#include "importdialog.h"
#include "csverror.h"
#include "database.h"
#include "importutils.h"
#include "qqutil/qqmenuhelper.h"

/**
 * Constructor.
 *
 * @param sourceType The type of file to open and import from
 * @param parent The widget to use as the parent of any dialogs launched
 */
ImportDialog::ImportDialog(DataSource sourceType, QWidget *parent)
  : QObject(), parentWidget(parent), source(sourceType)
{

}

/**
 * Launch the open file dialog (and a second dialog to select the text
 * encoding, if necessary).
 *
 * @return True if a file was was selected, false otherwise
 */
bool ImportDialog::exec()
{
    QString filter;
    if (source == CSV || source == OptionList) {
        filter = QString::null;
    }
    else if (source == MobileDB) {
        filter = tr("MobileDB files") + " (*.pdb)";
    }
    else if (source == XML) {
        filter = tr("XML files") + " (*.xml)";
    }
    else {
        filter = tr("Images") + " (*.jpg *.jpeg *.png)";
    }
    QSettings settings;
    QString lastDir = QQMenuHelper::getLastDir(&settings);
    QString file = QFileDialog::getOpenFileName(parentWidget,
                 tr("Choose a file"), lastDir, filter);
    if (file.isEmpty()) {
        return false;
    }
    else {
        QFileInfo info(file);
        settings.setValue("Files/LastDir", info.absolutePath());
        path = info.absoluteFilePath();
    }
    if (source == CSV || source == OptionList) {
        QStringList encodings;
        encodings.append("UTF-8");
        encodings.append("Latin-1");
        bool ok;
        encoding = QInputDialog::getItem(parentWidget, tr("Import"),
                                         tr("Text encoding") + ":",
                                         encodings, 0, false, &ok);
        if (!ok) {
            return false;
        }
    }
    return true;
}

/**
 * Import data from the selected file.  Should only be called if exec()
 * returned true.
 *
 * @param db The database to import into
 * @return True if data was imported, false otherwise
 */
bool ImportDialog::import(Database *db)
{
    QString error = "";
    QString data = "";
    if (source == CSV) {
        QStringList result = db->importFromCSV(path, encoding);
        int count = result.count();
        if (count > 0) {
            error = result[0];
        }
        if (count > 1) {
            data = result[1];
        }
    }
    else if (source == OptionList) {
        ImportUtils utils;
        error = utils.importTextLines(path, encoding, &options);
    }
    else if (source == MobileDB) {
        ImportUtils utils;
        error = utils.importMobileDB(path, db);
    }
    else if (source == XML) {
        ImportUtils utils;
        error = utils.importXML(path, db);
    }
    if (!error.isEmpty()) {
        if (data.isEmpty()) {
            QMessageBox::warning(0, qApp->applicationName(), error);
        }
        else {
            CSVErrorDialog dialog(error, data, 0);
            dialog.exec();
        }
        return false;
    }
    else {
        return true;
    }
}

/**
 * Get the list of enumeration options that were imported, if applicable.
 *
 * @return A list of enumeration option values
 */
QStringList ImportDialog::getOptions()
{
    return options;
}

/**
 * Get the absolute path of the file that was imported, if any.
 *
 * @return The imported file's absolute path
 */
QString ImportDialog::getPath()
{
    return path;
}
