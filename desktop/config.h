/*
 * config.h
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <qmap.h>
#include <qstringlist.h>
// included because of broken MS VC++ linker...QMap<QString,QString>
#include <qxml.h>

class Config
{
public:
    typedef QMap< QString, QString > ConfigGroup;
    
    enum Domain { File, User };
    Config( const QString &name, Domain domain=User );
    ~Config();

    bool operator == ( const Config & other ) const { return (filename == other.filename); }
    bool operator != ( const Config & other ) const { return (filename != other.filename); }

    bool hasKey( const QString &key ) const;
    void setGroup( const QString &gname );
    void writeEntry( const QString &key, const QString &value );
    void writeEntry( const QString &key, int num );
#ifdef Q_HAS_BOOL_TYPE
    void writeEntry( const QString &key, bool b );
#endif
    void writeEntry( const QString &key, const QStringList &lst, const QChar &sep );
    
    QString readEntry( const QString &key, const QString &deflt = QString::null ) const;
    QString readEntryDirect( const QString &key, const QString &deflt = QString::null ) const;
    int readNumEntry( const QString &key, int deflt = -1 ) const;
    bool readBoolEntry( const QString &key, bool deflt = FALSE ) const;
    QStringList readListEntry( const QString &key, const QChar &sep ) const;

    // For compatibility, non-const versions.
    QString readEntry( const QString &key, const QString &deflt );
    QString readEntryDirect( const QString &key, const QString &deflt );
    int readNumEntry( const QString &key, int deflt );
    bool readBoolEntry( const QString &key, bool deflt );
    QStringList readListEntry( const QString &key, const QChar &sep );
    
    void write( const QString &fn = QString::null );
    
protected:
    void read();
    bool parse( const QString &line );

    QMap< QString, ConfigGroup > groups;
    QMap< QString, ConfigGroup >::Iterator git;
    QString filename;
    bool changed;
    static QString configFilename(const QString& name, Domain);
};

inline QString Config::readEntry( const QString &key, const QString &deflt ) const
{ return ((Config*)this)->readEntry(key,deflt); }
inline QString Config::readEntryDirect( const QString &key, const QString &deflt ) const
{ return ((Config*)this)->readEntryDirect(key,deflt); }
inline int Config::readNumEntry( const QString &key, int deflt ) const
{ return ((Config*)this)->readNumEntry(key,deflt); }
inline bool Config::readBoolEntry( const QString &key, bool deflt ) const
{ return ((Config*)this)->readBoolEntry(key,deflt); }
inline QStringList Config::readListEntry( const QString &key, const QChar &sep ) const
{ return ((Config*)this)->readListEntry(key,sep); }

#endif
