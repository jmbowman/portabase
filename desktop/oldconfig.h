/*
 * oldconfig.h
 *
 * (c) 2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef OLDCONFIG_H
#define OLDCONFIG_H

#include <qmap.h>
#include <qstringlist.h>
// included because of broken MS VC++ linker...QMap<QString,QString>
#include <qxml.h>

class Config;

class OldConfig
{
public:
    typedef QMap<QString, QString> ConfigGroup;

    enum Domain { File, User };
    OldConfig(const QString &name, Domain domain=User);
    ~OldConfig();

    bool exists();
    void setGroup(const QString &gname);
    QString readEntry(const QString &key, const QString &deflt = QString::null);
    void migrate(const QString &group, Config &conf);

private:
    void read();
    bool parse(const QString &line);
    static QString configFilename(const QString& name, Domain);

private:
    QMap<QString, ConfigGroup> groups;
    QMap<QString, ConfigGroup>::Iterator git;
    QString filename;
};

#endif
