/*
 * qpeapplication.cpp
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
#include <stdlib.h>
#include "config.h"
#include "qpeapplication.h"

QPEApplication::QPEApplication(int& argc, char **argv, Type t)
    : QApplication(argc, argv, t)
{
    connect(this, SIGNAL(lastWindowClosed()), this, SLOT(quit()));
}

QPEApplication::~QPEApplication()
{

}

QString QPEApplication::iconDir()
{
#if defined(Q_WS_WIN)
    const char *windir = getenv("WINDIR");
    if (!windir) {
	return "./icons/";
    }
    QString iniPath = QString(windir) + "/portabase.ini";
#else
    QString iniPath = "/etc/portabase.conf";
#endif
    if (!QFile::exists(iniPath)) {
        return "./icons/";
    }
    Config conf(iniPath, Config::File);
    conf.setGroup("General");
    return conf.readEntry("IconPath", ".") + "/";
}

QString QPEApplication::helpDir()
{
#if defined(Q_WS_WIN)
    const char *windir = getenv("WINDIR");
    if (!windir) {
	return "./";
    }
    QString iniPath = QString(windir) + "/portabase.ini";
#else
    QString iniPath = "/etc/portabase.conf";
#endif
    if (!QFile::exists(iniPath)) {
        return "./";
    }
    Config conf(iniPath, Config::File);
    conf.setGroup("General");
    return conf.readEntry("HelpPath", ".") + "/";
}

QString QPEApplication::documentDir()
{
    Config conf("portabase");
    conf.setGroup("General");
    QString path = conf.readEntry("DocPath", QDir::homeDirPath()) + "/";
    QDir dir(path);
    if (!dir.exists()) {
        path = QDir::homeDirPath() + "/";
    }
    return path;
}

void QPEApplication::setDocumentDir(QString path)
{
    Config conf("portabase");
    conf.setGroup("General");
    conf.writeEntry("DocPath", path);
}

void QPEApplication::showMainDocumentWidget(QWidget* mw, bool nomaximize)
{
    if (!nomaximize) {
        mw->showMaximized();
    }
    else {
        mw->show();
    }
}
