/*
 * csvutils.cpp
 *
 * (c) 2002-2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qfile.h>
#include <qobject.h>
#include <qregexp.h>
#include <qtextstream.h>
#include "csvutils.h"
#include "database.h"

CSVUtils::CSVUtils() : m_textquote('"'), m_delimiter(','), types(0), calcCount(0)
{

}

CSVUtils::~CSVUtils()
{
    if (types != 0) {
        delete[] types;
    }
    for (int i = 0; i < calcCount; i++) {
        delete calcs[i];
    }
}

QStringList CSVUtils::parseFile(const QString &filename,
                                const QString &encoding, Database *db)
{
    initialize(db);
    QFile f(filename);
    QStringList returnVal;
    if (!f.open(IO_ReadOnly)) {
        returnVal.append(QObject::tr("Unable to open file"));
        return returnVal;
    }
    QTextStream input(&f);
    if (encoding == "Latin-1") {
        input.setEncoding(QTextStream::Latin1);
    }
    else {
        input.setEncoding(QTextStream::UnicodeUTF8);
    }
    rowNum = 1;
    message = "";
    enum { S_START, S_QUOTED_FIELD, S_MAYBE_END_OF_QUOTED_FIELD,
           S_END_OF_QUOTED_FIELD, S_MAYBE_NORMAL_FIELD,
           S_NORMAL_FIELD } state = S_START;
    QChar x;
    rowString = "";
    row.clear();
    QString field = "";
    // store IDs of added rows; if there's an error, delete them
    addedIds.clear();
    bool crLast = FALSE;
    while (!input.atEnd()) {
        input >> x; // read one char

        if (x == '\r') {
            // treat as '\n', and watch for a following real '\n'
            crLast = TRUE;
            x = '\n';
        }
        else if (crLast && x == '\n') {
            crLast = FALSE;
            continue;
        }
        else {
            crLast = FALSE;
        }
        rowString += x;
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
                    if (!addRow(db)) {
                        break;
                    }
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
                    if (!addRow(db)) {
                        break;
                    }
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
                    if (!addRow(db)) {
                        break;
                    }
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
                if (!addRow(db)) {
                    break;
                }
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
        addRow(db);
    }
    if (message != "") {
        // an error was encountered; delete any rows that were added
        int count = addedIds.count();
        for (int i = count - 1; i > -1; i--) {
            db->deleteRow(addedIds[i]);
        }
        returnVal.append(message);
        returnVal.append(rowString);
    }
    f.close();
    return returnVal;
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

void CSVUtils::initialize(Database *db)
{
    colNames = db->listColumns();
    if (types != 0) {
        delete[] types;
    }
    types = db->listTypes();
    colCount = colNames.count();
    endStringCount = 0;
    int i;
    for (i = colCount - 1; i > -1; i--) {
        int type = types[i];
        if (type != STRING && type != NOTE) {
            break;
        }
        endStringCount++;
    }
    for (i = 0; i < calcCount; i++) {
        delete calcs[i];
    }
    calcs.clear();
    calcDecimals.clear();
    for (i = 0; i < colCount; i++) {
        if (types[i] == CALC) {
            int decimals = 2;
            calcs.append(db->loadCalc(colNames[i], &decimals));
            calcDecimals.append(decimals);
        }
    }
    calcCount = calcs.count();
}

bool CSVUtils::addRow(Database *db)
{
    int countDiff = colCount - row.count();
    int i;
    if (countDiff > 0 && countDiff <= endStringCount) {
        // last columns may have been blank in Excel or whatever...
        for (i = 0; i < countDiff; i++) {
            row.append("");
        }
    }
    int rowId = 0;
    if (calcCount > 0) {
        // calculate the derived values in the added row
        int calcIndex = 0;
        for (i = 0; i < colCount; i++) {
            if (types[i] == CALC) {
                double value = calcs[calcIndex]->value(row, colNames);
                QStringList::iterator iter = row.at(i);
                iter = row.remove(iter);
                int decimals = calcDecimals[calcIndex];
                row.insert(iter, db->formatDouble(value, decimals));
                calcIndex++;
            }
        }
    }
    message = db->addRow(row, &rowId);
    if (message != "") {
        message = QObject::tr("Error in row %1").arg(rowNum) + "\n" + message;
        return FALSE;
    }
    addedIds.append(rowId);
    row.clear();
    rowString = "";
    rowNum++;
    return TRUE;
}
