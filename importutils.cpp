/*
 * importutils.cpp
 *
 * (c) 2002-2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qdatetime.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qobject.h>
#include <qregexp.h>
#include <qstringlist.h>
#include <qxml.h>
#include "database.h"
#include "datatypes.h"
#include "importutils.h"
#include "mobiledb.h"
#include "xmlimport.h"

ImportUtils::ImportUtils()
{

}

ImportUtils::~ImportUtils()
{

}

QString ImportUtils::importMobileDB(QString filename, Database *db)
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
        char firstLetter = type[0].latin1();
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
            int sepIndex = type.find(';');
            while (sepIndex != -1) {
                options.append(type.left(sepIndex));
                type = type.right(type.length() - sepIndex - 1);
                sepIndex = type.find(';');
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
        if (message != "") {
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

QString ImportUtils::importXML(QString filename, Database *db)
{
    XMLImport importer(db);
    QFile file(filename);
    QXmlInputSource source(file);
    QXmlSimpleReader reader;
    reader.setContentHandler(&importer);
    reader.setErrorHandler(&importer);
    reader.parse(source);
    return importer.formattedError();
}
