/*
 * qpeapplication.cpp
 *
 * (c) 2003-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qdir.h>
#include <qfile.h>
#include <qtextcodec.h>
#include <qtranslator.h>
#include <stdlib.h>
#include "config.h"
#include "qpeapplication.h"

QPEApplication::QPEApplication(int& argc, char **argv, Type t)
    : QApplication(argc, argv, t)
{
    connect(this, SIGNAL(lastWindowClosed()), this, SLOT(quit()));
    QTranslator *trans = new QTranslator(this);
    if (trans->load(helpDir() + "portabase.qm")) {
        installTranslator(trans);
    }
    else {
        delete trans;
    }
}

QPEApplication::~QPEApplication()
{

}

QString QPEApplication::iconDir()
{
#if defined(Q_WS_WIN)
    return qApp->applicationDirPath() + "/icons/";
#else
    return "/usr/share/portabase/icons/";
#endif
}

QString QPEApplication::helpDir()
{
    QString locale = QTextCodec::locale();
    int i  = locale.find(".");
    if (i > 0) {
        locale = locale.left(i);
    }
    i = locale.find("_");
    if (i > 0) {
        locale = locale.left(i);
    }
    locale += "/";
#if defined(Q_WS_WIN)
    QString path = qApp->applicationDirPath() + "/";
#else
    QString path = "/usr/share/portabase/html/";
#endif
    QDir dir(path + locale);
    if (dir.exists()) {
        path += locale;
    }
    return path;
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
