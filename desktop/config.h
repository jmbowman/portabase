/*
 * config.h
 *
 * (c) 2003-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <qsettings.h>
#include <qstringlist.h>

class Config : public QSettings
{
public:
    Config(const QString &name);
    ~Config();

    bool exists();
    bool hasKey(const QString &key) const;
    void setGroup(const QString &gname);
    void writeEntry(const QString &key, const QString &value);
    void writeEntry(const QString &key, int num);
    void writeEntry(const QString &key, bool b);
    void writeEntry(const QString &key, const QStringList &lst, const QChar &sep);
    
    QString readEntry(const QString &key, const QString &deflt = QString::null) const;
    int readNumEntry(const QString &key, int deflt = -1) const;
    bool readBoolEntry(const QString &key, bool deflt = FALSE) const;
    QStringList readListEntry(const QString &key, const QChar &sep) const;

    // For compatibility, non-const versions.
    QString readEntry(const QString &key, const QString &deflt);
    int readNumEntry(const QString &key, int deflt);
    bool readBoolEntry(const QString &key, bool deflt);
    QStringList readListEntry(const QString &key, const QChar &sep);

private:
    QString currentGroup;
};

inline QString Config::readEntry( const QString &key, const QString &deflt ) const
{ return ((Config*)this)->readEntry(key,deflt); }
inline int Config::readNumEntry( const QString &key, int deflt ) const
{ return ((Config*)this)->readNumEntry(key,deflt); }
inline bool Config::readBoolEntry( const QString &key, bool deflt ) const
{ return ((Config*)this)->readBoolEntry(key,deflt); }
inline QStringList Config::readListEntry( const QString &key, const QChar &sep ) const
{ return ((Config*)this)->readListEntry(key,sep); }

#endif
