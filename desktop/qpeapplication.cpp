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
#include <qstringlist.h>
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
    if (trans->load(QPEApplication::translationFile())) {
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

QStringList QPEApplication::languageList()
{
    QString lang = QTextCodec::locale();
    QStringList langs;
    langs.append(lang);
    int i  = lang.find(".");
    if (i > 0) {
        lang = lang.left(i);
    }
    i = lang.find("_");
    if (i > 0) {
        langs.append(lang.left(i));
    }
    return langs;
}

QString QPEApplication::translationFile()
{
    QStringList langs = QPEApplication::languageList();
#if defined(Q_WS_WIN)
    QString path = qApp->applicationDirPath() + "/i18n/";
#else
    QString path = "/usr/share/portabase/i18n/";
#endif
    QString filename = "/portabase.qm";
    int count = langs.count();
    for (int i = 0; i < count; i++) {
        if (QFile::exists(path + langs[i] + filename)) {
            return path + langs[i] + filename;
        }
    }
    // No appropriate translation file, just use what's in the code
    return "";
}

QString QPEApplication::helpDir()
{
    QStringList langs = QPEApplication::languageList();
#if defined(Q_WS_WIN)
    QString path = qApp->applicationDirPath() + "/help/";
#else
    QString path = "/usr/share/portabase/help/";
#endif
    int count = langs.count();
    for (int i = 0; i < count; i++) {
        QDir dir(path + langs[i]);
        if (dir.exists()) {
            return path + langs[i] + "/";
        }
    }
    // Default to English, will usually be present
    return path + "en/";
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
