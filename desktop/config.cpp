/*
 * config.cpp
 *
 * (c) 2003-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "config.h"

Config::Config(const QString &name)
    : QSettings()
{
    beginGroup("/portabase/" + name);
}

Config::~Config()
{
    endGroup();
}

bool Config::exists()
{
    bool ok;
    // if this isn't there, nothing is
    setGroup("Font");
    QSettings::readEntry("Name", "", &ok);
    return ok;
}

bool Config::hasKey(const QString &key) const
{
    // Only designed to handle the one place PortaBase uses it (toolbar prefs)
    bool ok;
    QSettings::readListEntry(key, &ok);
    return ok;
}

void Config::setGroup(const QString &gname)
{
    if (!currentGroup.isEmpty()) {
        endGroup();
    }
    currentGroup = "/" + gname;
    beginGroup(currentGroup);
}

void Config::writeEntry(const QString &key, const QString &value)
{
    QSettings::writeEntry("/" + key, value);
}

void Config::writeEntry(const QString &key, int num)
{
    QSettings::writeEntry("/" + key, num);
}

void Config::writeEntry( const QString &key, bool b )
{
    QSettings::writeEntry("/" + key, b);
}

void Config::writeEntry(const QString &key, const QStringList &lst, const QChar &)
{
    QSettings::writeEntry("/" + key, lst);
}

QString Config::readEntry(const QString &key, const QString &deflt)
{
    return QSettings::readEntry("/" + key, deflt);
}

int Config::readNumEntry(const QString &key, int deflt)
{
    return QSettings::readNumEntry("/" + key, deflt);
}

bool Config::readBoolEntry(const QString &key, bool deflt)
{
    return QSettings::readBoolEntry("/" + key, deflt);
}

QStringList Config::readListEntry(const QString &key, const QChar &)
{
    return QSettings::readListEntry("/" + key);
}
