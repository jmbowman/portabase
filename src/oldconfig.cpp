/*
 * oldconfig.cpp
 *
 * (c) 2004,2009,2015 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file oldconfig.cpp
 * Source file for OldConfig
 */

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include "oldconfig.h"

/**
 * Constructor.  The name parameter can be either "portabase" or "qpe" (the
 * latter indicates data which was shared between applications on the Sharp
 * Zaurus).
 *
 * @param name The name of the settings category to retrieve
 */
OldConfig::OldConfig(const QString &name)
{
    QDir dir = (QDir::homePath() + "/.portabase");
    filename = dir.path() + "/" + name + ".conf";
    git = groups.end();
    read();
}

/**
 * Treat future requests for settings as coming from the specified group
 * until told to do otherwise.
 *
 * @param gname The name of the group to read from
 */
void OldConfig::setGroup(const QString &gname)
{
    QMap< QString, ConfigGroup>::Iterator it = groups.find(gname);
    if (it == groups.end()) {
        git = groups.insert(gname, ConfigGroup());
        return;
    }
    git = it;
}

/**
 * Read the settings file and parse the data it contains for later use.
 */
void OldConfig::read()
{
    fileExists = QFileInfo(filename).exists();
    if (!fileExists) {
        git = groups.end();
        return;
    }

    QFile f(filename);
    if (!f.open(QFile::ReadOnly)) {
        git = groups.end();
        return;
    }

    QTextStream s(&f);
    s.setCodec("UTF-8");

    QStringList list(s.readAll().split('\n'));
    f.close();

    for (QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
        if (!parse( *it )) {
            git = groups.end();
            return;
        }
    }
}

/**
 * Determine if an old configuration file exists to load settings from.
 *
 * @return True if the file exists, false otherwise
 */
bool OldConfig::exists() const
{
    return fileExists;
}

/**
 * Parse a single line from the settings file.
 *
 * @param line The line to be parsed
 * @return False if the line doesn't make sense in context, true otherwise
 */
bool OldConfig::parse(const QString &line)
{
    QString text = line.trimmed();
    if (text[0] == QChar('[')) {
        QString gname = text;
        gname = gname.remove(0, 1);
        if (gname[(int)gname.length() - 1] == QChar(']')) {
            gname = gname.remove(gname.length() - 1, 1);
        }
        git = groups.insert(gname, ConfigGroup());
    }
    else if (!text.isEmpty()) {
        if (git == groups.end()) {
            return false;
        }
        int eq = text.indexOf('=');
        if (eq == -1) {
            return false;
        }
        QString key = text.left(eq).trimmed();
        QString value = text.mid(eq+1).trimmed();
        (*git).insert(key, value);
    }
    return true;
}

/**
 * Migrate the specified group of old settings into the current QSettings
 * storage.
 *
 * @param group The name of the group of settings to migrate
 * @param settings The QSettings object to put the migrated settings into
 */
void OldConfig::migrate(const QString &group, QSettings &settings)
{
    QStringList boolEntries;
    boolEntries.append("MONDAY");
    boolEntries.append("SHOWSECONDS");
    boolEntries.append("AMPM");
    boolEntries.append("NoteWrap");
    boolEntries.append("WrapAnywhere");
    boolEntries.append("ConfirmDeletions");
    boolEntries.append("BooleanToggle");
    QStringList numEntries;
    numEntries.append("ShortOrder");
    numEntries.append("LongOrder");
    numEntries.append("Size");
    QStringList stringEntries;
    stringEntries.append("DocPath");
    stringEntries.append("Name");
    stringEntries.append("EvenRows");
    stringEntries.append("OddRows");
    stringEntries.append("LastDir");
    stringEntries.append("View");
    stringEntries.append("Separator");
    setGroup(group);
    settings.beginGroup(group);
    ConfigGroup::Iterator iter;
    for (iter = (*git).begin(); iter != (*git).end(); ++iter) {
        QString key = iter.key();
        QString value = iter.value();
        if (boolEntries.contains(key)) {
            settings.setValue(key, (bool)value.toInt());
        }
        else if (numEntries.contains(key)) {
            settings.setValue(key, value.toInt());
        }
        else {
            settings.setValue(key, value);
        }
    }
    settings.endGroup();
}
