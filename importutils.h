/*
 * importutils.h
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef IMPORTUTILS_H
#define IMPORTUTILS_H

class Database;
class QString;
class QStringList;

class ImportUtils
{
public:
    ImportUtils();
    ~ImportUtils();

    QString importMobileDB(QString filename, Database *db);

private:
    QStringList convertMobileDBRow(QStringList values, int *types);
};

#endif
