/*
 * xmlexport.h
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef XMLEXPORT_H
#define XMLEXPORT_H

#include <mk4.h>
#include <qstringlist.h>

class QFile;
class QString;
class QStringList;
class QTextStream;
class Database;

class XMLExport
{
public:
    XMLExport(Database *source, QString filename, QStringList currentCols);
    ~XMLExport();

    void addGlobalView(c4_View &global);
    void addView(QString name, c4_View view);
    void addDataView(c4_View &fullView, c4_View &filteredView, int *colIds,
                     int *colTypes, QStringList idStrings);

private:
    QString getPropName(const c4_Property prop);
    QString getValue(c4_View &view, QString name, char type, int row);

private:
    Database *db;
    QString indentUnit;
    QString indent;
    QStringList cols;
    QStringList fieldElements;
    QFile *file;
    QTextStream *output;
};

#endif
