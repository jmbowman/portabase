/*
 * applnk.h
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __APPLNK_H__
#define __APPLNK_H__

#include <qstringlist.h>

class AppLnk
{
public:
    AppLnk();
    AppLnk(const QString &file);
    AppLnk(const AppLnk &copy); // copy constructor
    virtual ~AppLnk();

    QString name() const { return mName; }
    QString type() const;
    QString file() const;
    QString linkFile() const;

    void removeFiles();

    void setName( const QString& docname );
    void setFile( const QString& filename );
    void setType( const QString& mimetype );

protected:
    QString mName;
    QString mType;
    QString mFile;
    QString mExtension;
};

class DocLnk : public AppLnk
{
public:
    DocLnk();
    DocLnk(const DocLnk &o) : AppLnk(o) { }
    DocLnk(const QString &file);
    virtual ~DocLnk();
};

#endif // __APPLNK_H__
