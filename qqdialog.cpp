/*
 * qqdialog.cpp
 *
 * (c) 2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#if !defined(Q_WS_QWS)
#include "desktop/resource.h"
#endif

#include "qqdialog.h"

QQDialog::QQDialog(QString title, QWidget *parent, const char *name, bool modal, WFlags f)
    : QDialog(parent, name, modal, f)
{
    if (title.isEmpty()) {
        setCaption(tr("PortaBase") + QQDialog::titleSuffix);
    }
    else {
        setCaption(title + " - " + tr("PortaBase") + QQDialog::titleSuffix);
    }
}

#if defined(Q_WS_WIN)
// non-commercial Qt prefixes the titlebar text with "[Non-Commercial] - "
// unless this is present
const QString QQDialog::titleSuffix = " - Qt";
#else
const QString QQDialog::titleSuffix = "";
#endif

QQDialog::~QQDialog()
{

}

void QQDialog::finishConstruction(bool fullscreen, int minWidth,
                                  int minHeight)
{
#if defined(Q_WS_QWS)
    if (fullscreen) {
        showMaximized();
    }
#else
    QWidget *parent = parentWidget();
    if (fullscreen && parent) {
        if (minWidth != 0) {
            int width = (minWidth == -1) ? parent->width() / 2 : minWidth;
            setMinimumWidth(width);
        }
        if (minHeight != 0) {
            int height = (minHeight == -1) ? parent->height() : minHeight;
            setMinimumHeight(height);
        }
    }
    else {
        if (minWidth != -1) {
            setMinimumWidth(minWidth);
        }
        if (minHeight != -1) {
            setMinimumHeight(minHeight);
        }
    }
    setIcon(Resource::loadPixmap("portabase"));
#endif
}
