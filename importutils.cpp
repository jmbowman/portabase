/*
 * importutils.cpp
 *
 * (c) 2002-2003,2008-2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file importutils.cpp
 * Source file for ImportUtils
 */

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QRegExp>
#include <QStringList>
#include <QTextStream>
#include <QXmlInputSource>
#include <QXmlSimpleReader>
#include "database.h"
#include "datatypes.h"
#include "importutils.h"
#include "mobiledb.h"
#include "xmlimport.h"

/**
 * Constructor.
 */
ImportUtils::ImportUtils()
{

}

/**
 * Import data from a MobileDB file into a new PortaBase file.
 *
 * @param filename The path to the MobileDB file to be imported
 * @param db The PortaBase database to import into
 * @return An error message if there was a problem importing the data, an
 *         empty string otherwise
 */
QString ImportUtils::importMobileDB(const QString &filename, Database *db)
{
    MobileDBFile mdb(filename);
    if (!mdb.read()) {
        return QObject::tr("Not a valid MobileDB file");
    }
    QStringList names = mdb.field_labels();
    int colCount = names.count();
    int rowCount = mdb.row_count();
    int *types = new int[colCount];
    QStringList mdbTypes = mdb.field_types();
    const int *fieldLengths = mdb.field_lengths();
    int *widths = new int[colCount];
    int enumCount = 0;
    int i;
    for (i = 0; i < colCount; i++) {
        widths[i] = (fieldLengths[i] * 3) / 2;
        QString type = mdbTypes[i];
        char firstLetter = type[0].toLatin1();
        QString defaultVal = "0";
        if (firstLetter == 'I') {
            types[i] = INTEGER;
        }
        else if (firstLetter == 'F') {
            types[i] = FLOAT;
        }
        else if (firstLetter == 'B') {
            types[i] = BOOLEAN;
            if (type.contains('!')) {
                defaultVal = "1";
            }
        }
        else if (firstLetter == 'D') {
            types[i] = DATE;
            if (!type.contains('!')) {
                defaultVal = "17520914";
            }
        }
        else if (firstLetter == 'd') {
            types[i] = TIME;
            if (!type.contains('!')) {
                defaultVal = "-1";
            }
        }
        else if (firstLetter == 'L') {
            types[i] = FIRST_ENUM + enumCount;
            enumCount++;
            QStringList options;
            type = type.right(type.length() - 1);
            int sepIndex = type.indexOf(';');
            while (sepIndex != -1) {
                options.append(type.left(sepIndex));
                type = type.right(type.length() - sepIndex - 1);
                sepIndex = type.indexOf(';');
            }
            options.append(type);
            db->addEnum(names[i], options);
            defaultVal = options[0];
        }
        else {
            types[i] = STRING;
            defaultVal = "";
        }
        db->addColumn(i, names[i], types[i], defaultVal);
    }
    db->updateDataFormat();
    for (i = 0; i < rowCount; i++) {
        QStringList newRow = convertMobileDBRow(mdb.row(i), types);
        int rowColCount = newRow.count();
        if (rowColCount != colCount) {
            // may be some kind of note...ignore it
            continue;
        }
        QString message = db->addRow(newRow);
        if (!message.isEmpty()) {
            message = QObject::tr("Error in row %1").arg(i + 1)
                      + "\n" + message;
            return message;
        }
    }
    db->addView("_all", db->listColumns(), "_none", "_none");
    db->setViewColWidths(widths);
    delete[] types;
    delete[] widths;
    return "";
}

/**
 * Convert a row of MobileDB data fields into values suitable for use
 * as a PortaBase data row.
 *
 * @param values A row of values from a MobileDB file
 * @param types The column type IDs of the PortaBase file being imported into
 * @return A row of data formatted correctly for importing into PortaBase
 */
QStringList ImportUtils::convertMobileDBRow(QStringList values, int *types)
{
    QStringList result;
    int colCount = values.count();
    for (int i = 0; i < colCount; i++) {
        int type = types[i];
        QString value = values[i];
        if (type == INTEGER) {
            // sometimes (always?) stored like "145."
            value = value.replace(QRegExp("\\."), "");
        }
        else if (type == BOOLEAN) {
            if (value == "1" || value == "true" || value == "yes"
                    || value == "ja") {
                value = "1";
            }
            else {
                value = "0";
            }
        }
        else if (type == DATE) {
            bool ok;
            int days = value.toInt(&ok);
            if (ok) {
                QDate date(1904, 1, 1);
                date = date.addDays(days);
                if (date.isValid()) {
                    value = QString::number(date.year() * 10000
                                            + date.month() * 100 + date.day());
                }
                else {
                    value = "17520914";
                }
            }
            else {
                value = "17520914";
            }
        }
        result.append(value);
    }
    return result;
}

/**
 * Import data from an XML file into a new PortaBase file.  The XML file must
 * be in the format described at
 * http://portabase.sourceforge.net/portabase_xml.html.
 *
 * @param filename The path to the XML file to be imported
 * @param db The PortaBase database to import into
 * @return An error message if there was a problem importing the data, an
 *         empty string otherwise
 */
QString ImportUtils::importXML(const QString &filename, Database *db)
{
    QFileInfo info(filename);
    db->setImportBasePath(info.absolutePath() + "/");
    XMLImport importer(db);
    QFile file(filename);
    QXmlInputSource source(&file);
    QXmlSimpleReader reader;
    reader.setContentHandler(&importer);
    reader.setErrorHandler(&importer);
    reader.parse(source);
    return importer.formattedError();
}

/**
 * Import lines of text from a file, typically for use as the options of an
 * enumerated column type.
 *
 * @param filename The path of the file to import from
 * @param encoding The text encoding to read the file as, such as "Latin-1"
 *                 or "UTF-8"
 * @param options The list to be populated with the imported text lines
 * @return An error message if there was a problem importing the data, an
 *         empty string otherwise
 */
QString ImportUtils::importTextLines(const QString &filename, const QString &encoding, QStringList *options)
{
    QFile f(filename);
    if (!f.open(QFile::ReadOnly | QFile::Text)) {
        return QObject::tr("Unable to open file");
    }
    QTextStream input(&f);
    input.setCodec(encoding.toLatin1());
    while (!input.atEnd()) {
        options->append(input.readLine());
    }
    f.close();
    return "";
}
