/*
 * importutils.h
 *
 * (c) 2002,2008-2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file importutils.h
 * Header file for ImportUtils
 */

#ifndef IMPORTUTILS_H
#define IMPORTUTILS_H

class Database;
class QString;
class QStringList;

/**
 * A collection of utility methods for importing data from different kinds
 * of files.
 */
class ImportUtils
{
public:
    ImportUtils();

    QString importMobileDB(const QString &filename, Database *db);
    QString importXML(const QString &filename, Database *db);
    QString importTextLines(const QString &filename, const QString &encoding,
                            QStringList *options);

private:
    QStringList convertMobileDBRow(QStringList values, int *types);
};

#endif
