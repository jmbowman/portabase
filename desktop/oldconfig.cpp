/*
 * oldconfig.cpp
 *
 * (c) 2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>

#include "config.h"
#include "oldconfig.h"

QString OldConfig::configFilename(const QString& name, Domain d)
{
    switch (d) {
	case File:
	    return name;
	case User: {
	    QDir dir = (QDir::homeDirPath() + "/.portabase");
	    return dir.path() + "/" + name + ".conf";
	}
    }
    return name;
}

OldConfig::OldConfig(const QString &name, Domain domain)
    : filename(configFilename(name,domain))
{
    git = groups.end();
    read();
}

OldConfig::~OldConfig()
{

}

bool OldConfig::exists()
{
    return QFileInfo(filename).exists();
}

void OldConfig::setGroup(const QString &gname)
{
    QMap< QString, ConfigGroup>::Iterator it = groups.find(gname);
    if (it == groups.end()) {
        git = groups.insert(gname, ConfigGroup());
        return;
    }
    git = it;
}

QString OldConfig::readEntry(const QString &key, const QString &deflt)
{
    if (git == groups.end()) {
        return deflt;
    }
    ConfigGroup::ConstIterator it = (*git).find(key);
    if (it != ( *git ).end()) {
        return *it;
    }
    else {
        return deflt;
    }
}

void OldConfig::read()
{
    if (!QFileInfo(filename).exists()) {
        git = groups.end();
        return;
    }

    QFile f(filename);
    if (!f.open(IO_ReadOnly)) {
        git = groups.end();
        return;
    }

    QTextStream s(&f);
    s.setEncoding(QTextStream::UnicodeUTF8);

    QStringList list = QStringList::split('\n', s.read());
    f.close();

    for (QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
        if (!parse( *it )) {
            git = groups.end();
            return;
        }
    }
}

bool OldConfig::parse(const QString &l)
{
    QString line = l.stripWhiteSpace();
    if (line[0] == QChar('[')) {
        QString gname = line;
        gname = gname.remove(0, 1);
        if (gname[(int)gname.length() - 1] == QChar(']')) {
            gname = gname.remove(gname.length() - 1, 1);
        }
        git = groups.insert(gname, ConfigGroup());
    }
    else if (!line.isEmpty()) {
        if (git == groups.end()) {
            return FALSE;
        }
        int eq = line.find('=');
        if (eq == -1) {
            return FALSE;
        }
        QString key = line.left(eq).stripWhiteSpace();
        QString value = line.mid(eq+1).stripWhiteSpace();
        (*git).insert(key, value);
    }
    return TRUE;
}

void OldConfig::migrate(const QString &group, Config &conf)
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
    conf.setGroup(group);
    ConfigGroup::Iterator iter;
    for (iter = (*git).begin(); iter != (*git).end(); ++iter) {
        QString key = iter.key();
        QString value = iter.data();
        if (boolEntries.contains(key)) {
            conf.writeEntry(key, (bool)value.toInt());
        }
        else if (numEntries.contains(key)) {
            conf.writeEntry(key, value.toInt());
        }
        else {
            conf.writeEntry(key, value);
        }
    }
}
