/*
 * menuactions.h
 *
 * (c) 2003,2009-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file menuactions.h
 * Header file for MenuActions
 */

#ifndef MENUACTIONS_H
#define MENUACTIONS_H

#include <QIcon>
#include <QKeySequence>
#include <QMap>
#include <QObject>
#include <QString>
#include "dbeditor.h"

class QAction;

/**
 * Text, accelerator, and shortcut definitions for menu actions.  Based
 * around the idea of mapping an enumeration key to the corresponding bits
 * of data needed to build a menu action.  For example, the key
 * MenuActions::Print maps to tr("&Print") and CTRL+Key_P.  Grouped
 * together here partially so the relevant text strings are always included
 * in generated translation files, regardless of which platform the code is
 * being built on (some menu actions may not be present on all platforms).
 */
class MenuActions : public QObject
{
    Q_OBJECT
public:
    /** Enumeration of menus and menu items */
    enum Item {
        Row,
        View,
        Sort,
        Filter,
        ChangePassword,
        Import,
        ImportCSV,
        Export,
        Slideshow,
        Properties,
        Preferences,
        QuickFilter,
        AddRow,
        AddView,
        AddSorting,
        AddFilter,
        EditRow,
        EditView,
        EditSorting,
        EditFilter,
        DeleteRow,
        DeleteView,
        DeleteSorting,
        DeleteFilter,
        AllColumns,
        AllRows,
        CopyRow,
        CopyText,
        Show,
        DeleteRowsInFilter,
        EditColumns,
        EditEnums,
        PrintPreview,
        Print,
        Views,
        Sortings,
        Filters,
        Fullscreen
    };
    typedef QMap< Item, QString > PhraseMap;
    typedef QMap< Item, QKeySequence > ShortcutMap;

    MenuActions(QObject *parent=0);

    QString menuText(Item item);
    QAction *action(Item item, bool toggle=false);
    QAction *action(Item item, const QIcon &icon);

private:
    int shortcut(Item item);
    void prepareAction(Item item, QAction *action);

private:
    PhraseMap textMap; /**< Mapping of items to translations (including accelerators) */
    PhraseMap toolTipMap; /**< Mapping of items to translated tooltips */
    ShortcutMap shortcutMap; /**< Mapping of items to shortcut key combinations */
};

#endif
