/*
 * csvutils.h
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef CSVUTILS_H
#define CSVUTILS_H

#include <qstringlist.h>
#include "calc/calcnode.h"
#include "datatypes.h"

class Database;
class QString;

class CSVUtils
{
public:
    CSVUtils();
    ~CSVUtils();

    QStringList parseFile(const QString &filename, const QString &encoding,
                          Database *db);
    QString encodeRow(QStringList row);
    QString encodeCell(QString content);

private:
    void initialize(Database *db);
    bool addRow(Database *db);

private:
    QChar m_textquote;
    QChar m_delimiter;
    QStringList colNames;
    int colCount;
    int endStringCount;
    int rowNum;
    QString message;
    QString rowString;
    QStringList row;
    IntList addedIds;
    int *types;
    CalcNodeList calcs;
    IntList calcDecimals;
    int calcCount;
};

#endif
