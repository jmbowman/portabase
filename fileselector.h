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

#include <qpe/fileselector.h>
#include <qstringlist.h>

class PBFileSelector : public FileSelector
{
    Q_OBJECT
public:
    PBFileSelector(const QString &dirPath, const QStringList &recentFiles,
                   const QString &mimefilter, QWidget *parent,
                   const char *name=0);
    ~PBFileSelector();

    QString currentDir();
    QStringList recent();
    bool duplicate();
    bool rename();
    void initFile(const DocLnk &doc);

private:
    void configureDocLnk(DocLnk &doclnk, const QString &name);
};

#endif
