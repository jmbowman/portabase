/*
 * xmlexport.h
 *
 * (c) 2003,2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file xmlexport.h
 * Header file for XMLExport
 */

#ifndef XMLEXPORT_H
#define XMLEXPORT_H

#include <mk4.h>
#include <QStringList>
#include "image/imageutils.h"

class QFile;
class QTextStream;
class Database;

/**
 * Utility class for exporting the full content of a PortaBase file to an XML
 * file.  Begins writing to the file during the constructor, and finishes in
 * the destructor.
 */
class XMLExport
{
public:
    XMLExport(Database *source, const QString &filename,
              const QStringList &currentCols);
    ~XMLExport();

    void addGlobalView(c4_View &global);
    void addView(const QString &name, c4_View view);
    void addDataView(c4_View &fullView, c4_View &filteredView, int *colIds,
                     int *colTypes, const QStringList &idStrings,
                     const QStringList &colNames);

private:
    QString getPropName(const c4_Property prop);
    QString getValue(c4_View &view, const QString &name, char type, int row);

private:
    Database *db; /**< The database being exported from */
    QString indentUnit; /**< The unit of indentation to use for nested elements */
    QString indent; /**< The indentation string for the current location in the document */
    QStringList cols; /**< Ordered list of column names in the current view */
    QStringList fieldElements; /**< List of field value element names, in data type order */
    QFile *file; /**< The file being written to */
    QTextStream *output; /**< The text stream being used to write into the file */
    ImageUtils utils; /**< Image utilities instance to use for exporting image fields */
};

#endif
