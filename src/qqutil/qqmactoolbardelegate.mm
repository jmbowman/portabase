/*
 * qqmactoolbaritemvalidator.mm
 *
 * (c) 2016 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file qqutil/qqmactoolbaritemvalidator.mm
 * Source file for QQMacToolbarItemValidator
 */

#include <QtCore/QString>
#include <QMacToolBar>
#include <QMacToolBarItem>
#include <qmactoolbaritem.h>

#include "qmacfunctions.h"
#include "qqmactoolbardelegate.h"
#include "qqmactoolbarutils.h"
#include "qqtoolbar.h"

/**
 * Get the native toolbar identifier strings for all items in the given list.
 * @param items The items whose identifiers are to be fetched
 * @param cullUnselectable True if unselectable items should be skipped
 * @return An array o native toolbar item identifiers
 */
NSMutableArray *getItemIdentifiers(const QList<QMacToolBarItem *> &items, bool cullUnselectable)
{
    NSMutableArray *array = [[NSMutableArray alloc] init];
    foreach (const QMacToolBarItem *item, items) {
        if (cullUnselectable && item->selectable() == false)
            continue;
        [array addObject : [item->nativeToolBarItem() itemIdentifier]];
    }
    return array;
}

QT_USE_NAMESPACE

@implementation QQMacToolbarDelegate

- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar*)toolbar
{
    Q_UNUSED(toolbar);
    return getItemIdentifiers(toolBar->macToolBar()->items(), false);
}

- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar*)toolbar
{
    Q_UNUSED(toolbar);
    return getItemIdentifiers(toolBar->macToolBar()->allowedItems(), false);
}

- (NSArray *)toolbarSelectableItemIdentifiers: (NSToolbar *)toolbar
{
    Q_UNUSED(toolbar);
    NSMutableArray *array = getItemIdentifiers(toolBar->macToolBar()->items(), true);
    [array addObjectsFromArray:getItemIdentifiers(toolBar->macToolBar()->allowedItems(), true)];
    return array;
}

- (IBAction)itemClicked:(id)sender
{
    NSToolbarItem *nativeItem = reinterpret_cast<NSToolbarItem *>(sender);
    QString identifier = QString::fromNSString([nativeItem itemIdentifier]);
    QMacToolBarItem *item = toolBar->itemFor(identifier);
    emit item->activated();
}

- (BOOL)validateToolbarItem:(NSToolbarItem *)toolbarItem
{
    QString identifier = QString::fromNSString([toolbarItem itemIdentifier]);
    QMacToolBarItem *item = toolBar->itemFor(identifier);
    return toolBar->isEnabled(item);
}

- (NSToolbarItem *) toolbar: (NSToolbar *)toolbar itemForItemIdentifier: (NSString *) itemIdentifier willBeInsertedIntoToolbar:(BOOL) willBeInserted
{
    Q_UNUSED(toolbar);
    Q_UNUSED(willBeInserted);
    const QString identifier = QString::fromNSString(itemIdentifier);
    QMacToolBarItem *toolButton = reinterpret_cast<QMacToolBarItem *>(identifier.toULongLong()); // string -> unisgned long long -> pointer
    NSToolbarItem *toolbarItem = toolButton->nativeToolBarItem();

    [toolbarItem setTarget:self];
    [toolbarItem setAction:@selector(itemClicked:)];

    return toolbarItem;
}

@end
