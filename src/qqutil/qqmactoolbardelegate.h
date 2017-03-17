/*
 * qqmactoolbardelegate.h
 *
 * (c) 2016-2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file qqutil/qqmactoolbardelegate.h
 * Header file for QQMacToolbarItemValidator
 */

#ifndef QQMACTOOLBARDELEGATE_H
#define QQMACTOOLBARDELEGATE_H

#import <AppKit/AppKit.h>

#include <QtCore/qglobal.h>

class QQToolBar;

/**
 * Delegate for Mac native toolbars.  Replaces QMacToolbarDelegate in order to
 * support disabling and re-enabling items in sync with the corresponding
 * QActions.
 */
@interface QQMacToolbarDelegate : NSObject <NSToolbarDelegate>
{
@public
    QQToolBar *toolBar;
}

- (NSToolbarItem *) toolbar:(NSToolbar *)toolbar itemForItemIdentifier:(NSString *) itemIdent willBeInsertedIntoToolbar:(BOOL) willBeInserted;
- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar*)toolbar;
- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar*)toolbar;
- (NSArray *)toolbarSelectableItemIdentifiers:(NSToolbar *)toolbar;
- (IBAction)itemClicked:(id)sender;
- (BOOL)validateToolbarItem:(NSToolbarItem *)toolbarItem;
@end

#endif // QQMACTOOLBARDELEGATE_H
