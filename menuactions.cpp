/*
 * menuactions.cpp
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qaction.h>
#include "menuactions.h"

// kind of silly class used to make sure menus get translated correctly
// without lots of manual tweaking of the translation files
MenuActions::MenuActions(QObject *parent, const char *name)
    : QObject(parent, name)
{
    textMap.insert("File", tr("File"));
    textMap.insert("View", tr("View"));
    textMap.insert("Row", tr("Row"));
    textMap.insert("Sort", tr("Sort"));
    textMap.insert("Filter", tr("Filter"));
    textMap.insert("Help", tr("Help"));
    textMap.insert("New", tr("New"));
    textMap.insert("Open", tr("Open"));
    textMap.insert("Open Recent", tr("Open Recent"));
    textMap.insert("Save", tr("Save"));
    textMap.insert("Change Password", tr("Change Password"));
    textMap.insert("Import", tr("Import") + "...");
    textMap.insert("Export", tr("Export") + "...");
    textMap.insert("Preferences", tr("Preferences"));
    textMap.insert("Quit", tr("Quit"));
    textMap.insert("Help Contents", tr("Help Contents"));
    textMap.insert("About PortaBase", tr("About PortaBase"));
    textMap.insert("About Qt", tr("About Qt"));
    textMap.insert("Quick Filter", tr("Quick Filter"));
    textMap.insert("Add", tr("Add"));
    textMap.insert("Edit", tr("Edit"));
    textMap.insert("Delete", tr("Delete"));
    textMap.insert("Delete File", tr("Delete") + "...");
    textMap.insert("Rename", tr("Rename") + "...");
    textMap.insert("Refresh", tr("Refresh"));
    textMap.insert("All Columns", tr("All Columns"));
    textMap.insert("All Rows", tr("All Rows"));
    textMap.insert("Copy", tr("Copy") + "...");
    textMap.insert("Delete Rows In Filter", tr("Delete Rows In Filter"));
    textMap.insert("Edit Columns", tr("Edit Columns"));
    textMap.insert("Edit Enums", tr("Edit Enums"));
    textMap.insert("Close", tr("Close"));
#ifdef DESKTOP
    menuTextMap.insert("File", tr("&File"));
    menuTextMap.insert("View", tr("&View"));
    menuTextMap.insert("Row", tr("&Row"));
    menuTextMap.insert("Sort", tr("&Sort"));
    menuTextMap.insert("Filter", tr("Fi&lter"));
    menuTextMap.insert("Help", tr("&Help"));
    menuTextMap.insert("New", tr("&New"));
    menuTextMap.insert("Open", tr("&Open"));
    menuTextMap.insert("Open Recent", tr("Open &Recent"));
    menuTextMap.insert("Save", tr("&Save"));
    menuTextMap.insert("Change Password", tr("C&hange Password"));
    menuTextMap.insert("Import", tr("&Import") + "...");
    menuTextMap.insert("Export", tr("E&xport") + "...");
    menuTextMap.insert("Preferences", tr("&Preferences"));
    menuTextMap.insert("Quit", tr("&Quit"));
    menuTextMap.insert("Help Contents", tr("Help &Contents"));
    menuTextMap.insert("About PortaBase", tr("&About PortaBase"));
    menuTextMap.insert("About Qt", tr("About &Qt"));
    menuTextMap.insert("Quick Filter", tr("&Quick Filter"));
    menuTextMap.insert("Add", tr("&Add"));
    menuTextMap.insert("Edit", tr("&Edit"));
    menuTextMap.insert("Delete", tr("&Delete"));
    menuTextMap.insert("All Columns", tr("All &Columns"));
    menuTextMap.insert("All Rows", tr("All &Rows"));
    menuTextMap.insert("Copy", tr("&Copy") + "...");
    menuTextMap.insert("Delete Rows In Filter", tr("&Delete Rows In Filter"));
    menuTextMap.insert("Edit Columns", tr("Edi&t Columns"));
    menuTextMap.insert("Edit Enums", tr("Edit &Enums"));
    menuTextMap.insert("Close", tr("&Close"));

    accelMap.insert("Help Contents", Key_F1);
    accelMap.insert("New", CTRL+Key_N);
    accelMap.insert("Open", CTRL+Key_O);
    accelMap.insert("Save", CTRL+Key_S);
    accelMap.insert("Close", CTRL+Key_W);
    accelMap.insert("Quit", CTRL+Key_Q);
    accelMap.insert("Quick Filter", CTRL+Key_F);
    accelMap.insert("Preferences", CTRL+Key_P);
#endif
}

MenuActions::~MenuActions()
{

}

QString MenuActions::text(const QString &base)
{
    return textMap[base];
}

QString MenuActions::menuText(const QString &base)
{
    if (menuTextMap.contains(base)) {
        return menuTextMap[base];
    }
    return textMap[base];
}

int MenuActions::accel(const QString &base)
{
    if (accelMap.contains(base)) {
        return accelMap[base];
    }
    return 0;
}

QAction *MenuActions::action(const QString &base, bool toggle)
{
    return new QAction(text(base), menuText(base), accel(base), parent(), 0,
                       toggle);
}

QAction *MenuActions::action(const QString &base, const QIconSet &icon)
{
    return new QAction(text(base), icon, menuText(base), accel(base),
                       parent());
}
