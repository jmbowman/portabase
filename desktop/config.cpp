/*
 * config.cpp
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qmessagebox.h>
#if QT_VERSION <= 230 && defined(QT_NO_CODECS)
#include <qtextcodec.h>
#endif
#include <qtextstream.h>

#include <sys/stat.h>
#include "config.h"

QString Config::configFilename(const QString& name, Domain d)
{
    switch (d) {
	case File:
	    return name;
	case User: {
	    QDir dir = (QDir::homeDirPath() + "/.portabase");
	    if (!dir.exists()) {
		QDir::home().mkdir(".portabase");
            }
	    return dir.path() + "/" + name + ".conf";
	}
    }
    return name;
}

Config::Config( const QString &name, Domain domain )
    : filename( configFilename(name,domain) )
{
    git = groups.end();
    read();
}

Config::~Config()
{
    if ( changed )
	write();
}

bool Config::hasKey( const QString &key ) const
{
    if ( groups.end() == git )
	return FALSE;
    ConfigGroup::ConstIterator it = ( *git ).find( key );
    return it != ( *git ).end();
}

void Config::setGroup( const QString &gname )
{
    QMap< QString, ConfigGroup>::Iterator it = groups.find( gname );
    if ( it == groups.end() ) {
	git = groups.insert( gname, ConfigGroup() );
	changed = TRUE;
	return;
    }
    git = it;
}

void Config::writeEntry( const QString &key, const QString &value )
{
    if ( git == groups.end() ) {
	qWarning( "no group set" );
	return;
    }
    if ( (*git)[key] != value ) {
	( *git ).insert( key, value );
	changed = TRUE;
    }
}

void Config::writeEntry( const QString &key, int num )
{
    QString s;
    s.setNum( num );
    writeEntry( key, s );
}

#ifdef Q_HAS_BOOL_TYPE
void Config::writeEntry( const QString &key, bool b )
{
    QString s;
    s.setNum( ( int )b );
    writeEntry( key, s );
}
#endif

void Config::writeEntry( const QString &key, const QStringList &lst, const QChar &sep )
{
    QString s;
    QStringList::ConstIterator it = lst.begin();
    for ( ; it != lst.end(); ++it )
	s += *it + sep;
    writeEntry( key, s );
}

QString Config::readEntry( const QString &key, const QString &deflt )
{
    return readEntryDirect( key, deflt );
}

QString Config::readEntryDirect( const QString &key, const QString &deflt )
{
    if ( git == groups.end() ) {
	//qWarning( "no group set" );
	return deflt;
    }
    ConfigGroup::ConstIterator it = ( *git ).find( key );
    if ( it != ( *git ).end() )
	return *it;
    else
	return deflt;
}

int Config::readNumEntry( const QString &key, int deflt )
{
    QString s = readEntry( key );
    if ( s.isEmpty() )
	return deflt;
    else
	return s.toInt();
}

bool Config::readBoolEntry( const QString &key, bool deflt )
{
    QString s = readEntry( key );
    if ( s.isEmpty() )
	return deflt;
    else
	return (bool)s.toInt();
}

QStringList Config::readListEntry( const QString &key, const QChar &sep )
{
    QString s = readEntry( key );
    if ( s.isEmpty() )
	return QStringList();
    else
	return QStringList::split( sep, s );
}

void Config::write( const QString &fn )
{
    QString strNewFile;
    if ( !fn.isEmpty() )
	filename = fn;
    strNewFile = filename + ".new";

    QFile f( strNewFile );
    if ( !f.open( IO_WriteOnly|IO_Raw ) ) {
	qWarning( "could not open for writing `%s'", strNewFile.latin1() );
	git = groups.end();
	return;
    }

    QString str;
    QCString cstr;
    QMap< QString, ConfigGroup >::Iterator g_it = groups.begin();

    for ( ; g_it != groups.end(); ++g_it ) {
 	str += "[" + g_it.key() + "]\n";
 	ConfigGroup::Iterator e_it = ( *g_it ).begin();
 	for ( ; e_it != ( *g_it ).end(); ++e_it )
 	    str += e_it.key() + " = " + *e_it + "\n";
    }
    cstr = str.utf8();

    int total_length;
    total_length = f.writeBlock( cstr.data(), cstr.length() );
    if ( total_length != int(cstr.length()) ) {
	QMessageBox::critical( 0, QObject::tr("Out of Space"),
			       QObject::tr("There was a problem creating\nConfiguration Information \nfor this program.\n\nPlease free up some space and\ntry again.") );
	f.close();
	QFile::remove( strNewFile );
	return;
    }

    f.close();
    // now rename the file...
    if (QFile::exists(filename)) {
        // on Windows, existing file must be deleted first
        QFile::remove(filename);
    }
    if ( rename( strNewFile, filename ) < 0 ) {
	qWarning( "problem renaming the file %s to %s", strNewFile.latin1(),
		  filename.latin1() );
	QFile::remove( strNewFile );
    }
}

void Config::read()
{
    changed = FALSE;

    if ( !QFileInfo( filename ).exists() ) {
	git = groups.end();
	return;
    }

    QFile f( filename );
    if ( !f.open( IO_ReadOnly ) ) {
	git = groups.end();
	return;
    }

    QTextStream s( &f );
#if QT_VERSION <= 230 && defined(QT_NO_CODECS)
    // The below should work, but doesn't in Qt 2.3.0
    s.setCodec( QTextCodec::codecForMib( 106 ) );
#else
    s.setEncoding( QTextStream::UnicodeUTF8 );
#endif

    QStringList list = QStringList::split('\n', s.read() );
    f.close();

    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        if ( !parse( *it ) ) {
            git = groups.end();
            return;
        }
    }
}

bool Config::parse( const QString &l )
{
    QString line = l.stripWhiteSpace();
    if ( line[ 0 ] == QChar( '[' ) ) {
	QString gname = line;
	gname = gname.remove( 0, 1 );
	if ( gname[ (int)gname.length() - 1 ] == QChar( ']' ) )
	    gname = gname.remove( gname.length() - 1, 1 );
	git = groups.insert( gname, ConfigGroup() );
    } else if ( !line.isEmpty() ) {
	if ( git == groups.end() )
	    return FALSE;
	int eq = line.find( '=' );
	if ( eq == -1 )
	    return FALSE;
	QString key = line.left(eq).stripWhiteSpace();
	QString value = line.mid(eq+1).stripWhiteSpace();
	( *git ).insert( key, value );
    }
    return TRUE;
}
