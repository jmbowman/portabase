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

#include <qregexp.h>
#include <qstringlist.h>
#include "csvutils.h"

CSVUtils::CSVUtils() : m_textquote('"'), m_delimiter(',')
{

}

CSVUtils::~CSVUtils()
{

}

QStringList CSVUtils::getRow(QString line)
{
    QStringList result;
    enum { S_START, S_QUOTED_FIELD, S_MAYBE_END_OF_QUOTED_FIELD,
           S_END_OF_QUOTED_FIELD, S_MAYBE_NORMAL_FIELD,
           S_NORMAL_FIELD } state = S_START;
    QChar x;
    QString field = "";
    int length = line.length();
    for (int i = 0; i < length; i++) {
        x = line.at(i);

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
                result.append("");
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
            else if (x == m_delimiter) {
                result.append(field);
                field = "";
                state = S_START;
            }
            else {
                state = S_END_OF_QUOTED_FIELD;
            }
            break;
        case S_END_OF_QUOTED_FIELD :
            if (x == m_delimiter) {
                result.append(field);
                field = "";
                state = S_START;
            }
            else {
                state = S_END_OF_QUOTED_FIELD;
            }
            break;
        case S_MAYBE_NORMAL_FIELD :
            if (x == m_textquote)
            {
                field = "";
                state = S_QUOTED_FIELD;
                break;
            }
        case S_NORMAL_FIELD :
            if (x == m_delimiter) {
                result.append(field);
                field = "";
                state = S_START;
            }
            else {
                field += x;
            }
        }
    }
    if (state != S_START) {
        result.append(field);
    }
    return result;
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
    if (content.contains('"') == 0 && content.contains(',') == 0) {
        return content;
    }
    QString result("\"");
    result += content.replace(QRegExp("\""), "\"\"");
    return result + "\"";
}
