/*
 * csvutils.cpp
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qfile.h>
#include <qregexp.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include "csvutils.h"
#include "database.h"
#include "datatypes.h"
#include "portabase.h"

CSVUtils::CSVUtils() : m_textquote('"'), m_delimiter(',')
{

}

CSVUtils::~CSVUtils()
{

}

QString CSVUtils::parseFile(QString filename, Database *db)
{
    QFile f(filename);
    if (!f.open(IO_ReadOnly)) {
        return PortaBase::tr("Unable to open file");
    }
    QTextStream input(&f);
    input.setEncoding(QTextStream::Locale);
    int rowNum = 1;
    QString message = "";
    enum { S_START, S_QUOTED_FIELD, S_MAYBE_END_OF_QUOTED_FIELD,
           S_END_OF_QUOTED_FIELD, S_MAYBE_NORMAL_FIELD,
           S_NORMAL_FIELD } state = S_START;
    QChar x;
    QStringList row;
    QString field = "";
    // store IDs of added rows; if there's an error, delete them
    IntList addedIds;
    while (!input.atEnd()) {
        input >> x; // read one char

        if (x == '\r') {
            // eat '\r', to handle DOS/WINDOWS files correctly
            continue;
        }
        switch (state)
        {
        case S_START :
            if (x == m_textquote) {
                state = S_QUOTED_FIELD;
            }
            else if (x == m_delimiter) {
                row.append("");
            }
            else if (x == '\n') {
                if (row.count() == 0) {
                    // blank line, ignore it
                    continue;
                }
                else {
                    // row ended on a delimiter (empty cell)
                    row.append("");
                    field = "";
                    int rowId = 0;
                    message = db->addRow(row, &rowId);
                    if (message != "") {
                        message = PortaBase::tr("Error in row") + " "
                                  + QString::number(rowNum) + "\n" + message;
                        break;
                    }
                    addedIds.append(rowId);
                    row.clear();
                    rowNum++;
                }
            }
            else {
                field += x;
                state = S_MAYBE_NORMAL_FIELD;
            }
            break;
        case S_QUOTED_FIELD :
            if (x == m_textquote) {
                state = S_MAYBE_END_OF_QUOTED_FIELD;
            }
            else {
                field += x;
            }
            break;
        case S_MAYBE_END_OF_QUOTED_FIELD :
            if (x == m_textquote) {
                field += x;
                state = S_QUOTED_FIELD;
            }
            else if (x == m_delimiter || x == '\n') {
                row.append(field);
                field = "";
                if (x == '\n') {
                    int rowId = 0;
                    message = db->addRow(row, &rowId);
                    if (message != "") {
                        message = PortaBase::tr("Error in row") + " "
                                  + QString::number(rowNum) + "\n" + message;
                        break;
                    }
                    addedIds.append(rowId);
                    row.clear();
                    rowNum++;
                }
                state = S_START;
            }
            else {
                state = S_END_OF_QUOTED_FIELD;
            }
            break;
        case S_END_OF_QUOTED_FIELD :
            if (x == m_delimiter || x == '\n') {
                row.append(field);
                field = "";
                if (x == '\n') {
                    int rowId = 0;
                    message = db->addRow(row, &rowId);
                    if (message != "") {
                        message = PortaBase::tr("Error in row") + " "
                                  + QString::number(rowNum) + "\n" + message;
                        break;
                    }
                    addedIds.append(rowId);
                    row.clear();
                    rowNum++;
                }
                state = S_START;
            }
            else {
                state = S_END_OF_QUOTED_FIELD;
            }
            break;
        case S_MAYBE_NORMAL_FIELD :
            if (x == m_textquote) {
                field = "";
                state = S_QUOTED_FIELD;
                break;
            }
        case S_NORMAL_FIELD :
            if (x == m_delimiter) {
                row.append(field);
                field = "";
                state = S_START;
            }
            else if (x == '\n') {
                row.append(field);
                field = "";
                int rowId = 0;
                message = db->addRow(row, &rowId);
                if (message != "") {
                    message = PortaBase::tr("Error in row") + " "
                              + QString::number(rowNum) + "\n" + message;
                    break;
                }
                addedIds.append(rowId);
                row.clear();
                rowNum++;
            }
            else {
                field += x;
            }
        }
        if (message != "") {
            break;
        }
    }
    if (message == "" && row.count() > 0) {
        // last line doesn't end with '\n'
        row.append(field);
        int rowId = 0;
        message = db->addRow(row, &rowId);
        if (message != "") {
            message = PortaBase::tr("Error in row") + " "
                      + QString::number(rowNum) + "\n" + message;
        }
    }
    if (message != "") {
        // an error was encountered; delete any rows that were added
        int count = addedIds.count();
        for (int i = count - 1; i > -1; i--) {
            db->deleteRow(addedIds[i]);
        }
    }
    f.close();
    return message;
}

QString CSVUtils::encodeRow(QStringList row)
{
    QString result;
    int count = (int)row.count();
    for (int i = 0; i < count; i++) {
        result += encodeCell(row[i]);
        if (i < count - 1) {
            result += m_delimiter;
        }
    }
    return result + "\n";
}

QString CSVUtils::encodeCell(QString content)
{
    if (content.contains('"') == 0 && content.contains(',') == 0
            && content.contains('\n') == 0) {
        return content;
    }
    QString result("\"");
    result += content.replace(QRegExp("\""), "\"\"");
    return result + "\"";
}
