/*
 * qpeapplication.h
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __QPE_APPLICATION_H__
#define __QPE_APPLICATION_H__

#include <qapplication.h>

class QPEApplication : public QApplication
{
    Q_OBJECT
public:
    QPEApplication(int& argc, char **argv, Type=GuiClient);
    ~QPEApplication();

    static QString iconDir();
    static QString helpDir();
    static QString documentDir();
    static void setDocumentDir(QString path);

    void showMainDocumentWidget(QWidget*, bool nomax=TRUE);
};

#endif
