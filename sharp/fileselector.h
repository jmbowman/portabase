/*
 * pbfileselector.h
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef PBFILESELECTOR_H
#define PBFILESELECTOR_H

#include <qpe/applnk.h>
#include <sl/slfileselector.h>
#include <qfileinfo.h>
#include <qstringlist.h>

class PBFileSelector : public SlFileSelector
{
    Q_OBJECT
public:
    PBFileSelector(const QString &dirPath, const QStringList &recentFiles,
                   const QString &mimefilter, QWidget *parent,
                   const char *name=0);
    ~PBFileSelector();

    void reread();
    const DocLnk *selected();
    void initFile(const DocLnk &doc);

signals:
    void fileSelected(const DocLnk &);

private slots:
    void passSelection(const QFileInfo &file);

private:
    DocLnk selection;
};

#endif
