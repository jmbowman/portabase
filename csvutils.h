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

class Database;
class QString;
class QStringList;

class CSVUtils
{
public:
    CSVUtils();
    ~CSVUtils();

    QString parseFile(QString filename, Database *db);
    QString encodeRow(QStringList row);
    QString encodeCell(QString content);

private:
    QChar m_textquote;
    QChar m_delimiter;
};

#endif
