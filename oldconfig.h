/*
 * oldconfig.h
 *
 * (c) 2004,2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file oldconfig.h
 * Header file for OldConfig
 */

#ifndef OLDCONFIG_H
#define OLDCONFIG_H

#include <QMap>
#include <QSettings>
#include <QStringList>

/**
 * Parser for application settings from early versions of PortaBase for
 * desktop operating systems, which used Qt 2 and hence didn't have access
 * to the QSettings class.  Now only used to find these older settings if
 * ones in the new format aren't present, and migrate them over when the
 * newer version of PortaBase is first run in that environment.
 */
class OldConfig
{
public:
    /** A group of settings, mapped from name to value */
    typedef QMap<QString, QString> ConfigGroup;

    OldConfig(const QString &name);

    void setGroup(const QString &gname);
    QString readEntry(const QString &key, const QString &deflt = QString::null);
    bool exists();
    void migrate(const QString &group, QSettings &settings);

private:
    void read();
    bool parse(const QString &line);

private:
    QMap<QString, ConfigGroup> groups; /**< Mapping of group names to group content */
    QMap<QString, ConfigGroup>::Iterator git; /**< Iterator positioned at the current group */
    QString filename; /**< The path of the settings file to parse */
    bool fileExists; /**< True if such a configuration file exists */
};

#endif
