/*
 * filemanager.h
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <qstring.h>

class DocLnk;

class FileManager
{
public:
    FileManager();
    ~FileManager();

    bool saveFile(const DocLnk&, const QString &text);
};

#endif
