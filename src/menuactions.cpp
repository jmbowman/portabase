/*
 * menuactions.cpp
 *
 * (c) 2003-2004,2009-2010,2016-2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file menuactions.cpp
 * Source file for MenuActions
 */

#include <QAction>
#include "menuactions.h"
#include "qqutil/qqmenuhelper.h"

/**
 * Constructor.
 *
 * @param parent The parent object, if any
 */
MenuActions::MenuActions(QObject *parent)
    : QObject(parent)
{
#if defined(Q_OS_ANDROID)
    QString ellipsis("");
#else
    QChar ellipsis(8230);
#endif
    textMap.insert(View, tr("&View"));
    textMap.insert(Row, tr("&Row"));
    textMap.insert(Sort, tr("&Sort"));
    textMap.insert(Filter, tr("Fi&lter"));

    textMap.insert(ChangePassword, tr("C&hange Password") + ellipsis);
    toolTipMap.insert(ChangePassword, tr("Change the current file's password"));

    objectNameMap.insert(Import, "Import");
    textMap.insert(Import, tr("&Import") + ellipsis);
    toolTipMap.insert(Import, tr("Import a file"));

    textMap.insert(ImportCSV, tr("&Import") + ellipsis);
    toolTipMap.insert(ImportCSV, tr("Import rows from a CSV file"));

    textMap.insert(Export, tr("E&xport") + ellipsis);
    toolTipMap.insert(Export, tr("Export data to another file format"));

    textMap.insert(Slideshow, tr("S&lideshow") + ellipsis);
    toolTipMap.insert(Slideshow, tr("Start an image slideshow"));

    textMap.insert(Properties, tr("Proper&ties"));
    toolTipMap.insert(Properties, tr("Show information about the current file"));

    textMap.insert(PrintPreview, tr("Print Pre&view") + ellipsis);
    toolTipMap.insert(PrintPreview, tr("Configure and preview printing options"));

    textMap.insert(Print, tr("&Print") + ellipsis);
    toolTipMap.insert(Print, tr("Print the current file"));
    shortcutMap.insert(Print, QKeySequence(QKeySequence::Print));

    objectNameMap.insert(QuickFilter, "Quick Filter");
    textMap.insert(QuickFilter, tr("&Quick Filter"));
    toolTipMap.insert(QuickFilter, tr("Apply a one-condition filter"));
    shortcutMap.insert(QuickFilter, QKeySequence(QKeySequence::Find));

    objectNameMap.insert(AddRow, "AddRow");
    textMap.insert(AddRow, tr("&Add") + ellipsis);
    toolTipMap.insert(AddRow, tr("Create a new row"));

    textMap.insert(AddView, tr("&Add") + ellipsis);
    toolTipMap.insert(AddView, tr("Create a new view"));

    textMap.insert(AddSorting, tr("&Add") + ellipsis);
    toolTipMap.insert(AddSorting, tr("Create a new sorting"));

    textMap.insert(AddFilter, tr("&Add") + ellipsis);
    toolTipMap.insert(AddFilter, tr("Create a new filter"));

    objectNameMap.insert(EditRow, "Edit Row");
    textMap.insert(EditRow, tr("&Edit") + ellipsis);
    toolTipMap.insert(EditRow, tr("Edit the selected row"));

    textMap.insert(EditView, tr("&Edit") + ellipsis);
    toolTipMap.insert(EditView, tr("Edit the selected view"));

    textMap.insert(EditSorting, tr("&Edit") + ellipsis);
    toolTipMap.insert(EditSorting, tr("Edit the selected sorting"));

    textMap.insert(EditFilter, tr("&Edit") + ellipsis);
    toolTipMap.insert(EditFilter, tr("Edit the selected filter"));

    objectNameMap.insert(DeleteRow, "Delete Row");
    textMap.insert(DeleteRow, tr("&Delete"));
    toolTipMap.insert(DeleteRow, tr("Delete the selected row"));
    shortcutMap.insert(DeleteRow, QKeySequence::Delete);

    textMap.insert(DeleteView, tr("&Delete"));
    toolTipMap.insert(DeleteView, tr("Delete the selected view"));

    textMap.insert(DeleteSorting, tr("&Delete"));
    toolTipMap.insert(DeleteSorting, tr("Delete the selected sorting"));

    textMap.insert(DeleteFilter, tr("&Delete"));
    toolTipMap.insert(DeleteFilter, tr("Delete the selected filter"));

    textMap.insert(AllColumns, tr("All &Columns"));
    toolTipMap.insert(AllColumns, tr("Show all of the database columns"));

    textMap.insert(AllRows, tr("All &Rows"));
    toolTipMap.insert(AllRows, tr("Show all rows of data"));

    objectNameMap.insert(CopyRow, "Copy Row");
    textMap.insert(CopyRow, tr("&Copy") + ellipsis);
    toolTipMap.insert(CopyRow, tr("Create a copy of the selected row"));

    textMap.insert(CopyText, tr("&Copy"));
    toolTipMap.insert(CopyText, tr("Copy the selected text"));

    textMap.insert(Show, tr("&Show"));
    toolTipMap.insert(Show, tr("Show the selected row in more detail"));
    shortcutMap.insert(Show, QKeySequence(Qt::CTRL + Qt::Key_R));

    textMap.insert(DeleteRowsInFilter, tr("&Delete Rows in Filter"));
    toolTipMap.insert(DeleteRowsInFilter, tr("Delete all rows matching the current filter"));

    textMap.insert(EditColumns, tr("Edit Col&umns") + ellipsis);
    toolTipMap.insert(EditColumns, tr("Edit the database format"));

    textMap.insert(EditEnums, tr("Edit E&nums") + ellipsis);
    toolTipMap.insert(EditEnums, tr("Edit the enumerated data types"));

    objectNameMap.insert(Views, "Views");
    textMap.insert(Views, tr("Views") + ellipsis);
    toolTipMap.insert(Views, tr("Change the active view"));

    objectNameMap.insert(Sortings, "Sortings");
    textMap.insert(Sortings, tr("Sortings") + ellipsis);
    toolTipMap.insert(Sortings, tr("Change the active sorting"));

    objectNameMap.insert(Filters, "Filters");
    textMap.insert(Filters, tr("Filters") + ellipsis);
    toolTipMap.insert(Filters, tr("Change the active filter"));

    objectNameMap.insert(Fullscreen, "Fullscreen");
    textMap.insert(Fullscreen, tr("Fullscreen"));
    textMap.insert(ExitFullscreen, tr("Exit Fullscreen"));
    toolTipMap.insert(Fullscreen, tr("View PortaBase in fullscreen mode"));
    toolTipMap.insert(ExitFullscreen, tr("Exit fullscreen mode"));
}

/**
 * Get the translation for the specified word or phrase as it should
 * appear in a menu (possibly including an accelerator).
 *
 * @param item The identifier of the desired menu or action
 */
QString MenuActions::menuText(Item item)
{
    return QQMenuHelper::menuText(textMap[item]);
}

/**
 * Get the translation for the tooltip of the specified menu or action.
 *
 * @param item The identifier of the desired menu or action
 */
QString MenuActions::toolTipText(Item item)
{
    return toolTipMap[item];
}

/**
 * Get the action corresponding to the provided item identifier.
 *
 * @param item The identifier of the desired action
 * @param toggle True if the action can be toggled (on/off)
 */
QAction *MenuActions::action(Item item, bool toggle)
{
    QAction *action = new QAction(menuText(item), parent());
    action->setCheckable(toggle);
    prepareAction(item, action);
    return action;
}

/**
 * Get the action corresponding to the provided item identifier.
 *
 * @param item The identifier of the desired action
 * @param icon The icon to use for the action
 */
QAction *MenuActions::action(Item item, const QIcon &icon)
{
    QAction *action = new QAction(icon, menuText(item), parent());
#if defined(Q_OS_MAC) || defined(Q_WS_HILDON) || defined(Q_OS_ANDROID)
    action->setIconVisibleInMenu(false);
#endif
    prepareAction(item, action);
    return action;
}

/**
 * Peform initialization steps common to all new menu and toolbar actions.
 *
 * @param item The identifier of the desired action
 * @param action The action being initialized
 */
void MenuActions::prepareAction(Item item, QAction *action)
{
    if (objectNameMap.contains(item)) {
        action->setObjectName(objectNameMap[item]);
    }
    if (toolTipMap.contains(item)) {
        QString text = toolTipMap[item];
        action->setToolTip(text);
        action->setStatusTip(text);
    }
#if !defined(Q_OS_ANDROID)
    if (shortcutMap.contains(item)) {
        action->setShortcut(shortcutMap[item]);
    }
#endif
}
