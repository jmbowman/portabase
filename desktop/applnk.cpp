/*
 * applnk.cpp
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qfile.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include "applnk.h"
#include "qpeapplication.h"

static QString safeFileName(const QString& n)
{
    QString safename = n;
    safename.replace(QRegExp("[^0-9A-Za-z.]"),"_");
    safename.replace(QRegExp("^[^A-Za-z]*"),"");
    if (safename.isEmpty()) {
	safename = "_";
    }
    return safename;
}

AppLnk::AppLnk()
{

}

AppLnk::AppLnk(const QString &file)
{
    if (!file.isNull()) {
        QFileInfo info(file);
        mName = info.baseName();
        QString mExtension = info.extension(FALSE).lower();
        if (mExtension == "csv") {
            mType = "text/x-csv";
        }
        else if (mExtension == "pdb") {
            mType = "chemical/x-pdb";
        }
        else {
            mType = "application/portabase";
        }
        mFile = file;
    }
}

QString AppLnk::type() const
{
    return mType;
}

QString AppLnk::file() const
{
    if (mFile.isNull()) {
        QString ext = "." + mExtension;
	QString path = QPEApplication::documentDir() + safeFileName(mName);
	if (QFile::exists(path + ext)) {
            int n = 1;
	    QString nn = path + "_" + QString::number(n);
	    while (QFile::exists(nn + ext)) {
		n++;
		nn = path + "_" + QString::number(n);
	    }
	    path = nn;
	}
        path += ext;
	QFile f(path);
	if (!f.open(IO_WriteOnly)) {
	    path = QString::null;
        }
        return path;
    }
    else {
        return mFile;
    }
}

QString AppLnk::linkFile() const
{
    return QString("");
}

AppLnk::AppLnk(const AppLnk &copy)
{
    mName = copy.mName;
    mType = copy.mType;
    mFile = copy.mFile;
    mExtension = copy.mExtension;
}

AppLnk::~AppLnk()
{

}

void AppLnk::setName(const QString& docname)
{
    mName = docname;
}

void AppLnk::setFile(const QString& filename)
{
    mFile = filename;
}

void AppLnk::setType(const QString& type)
{
    mType = type;
    if (mType == "text/x-csv") {
        mExtension = "csv";
    }
    else if (mType == "chemical/x-pdb") {
        mExtension = "pdb";
    }
    else {
        mExtension = "pob";
    }
}

void AppLnk::removeFiles()
{
    QFile::remove(file());
}

DocLnk::DocLnk(const QString &file) :
    AppLnk(file)
{

}

DocLnk::DocLnk()
{

}

DocLnk::~DocLnk()
{

}
