/*
 * qqmactoolbar.mm
 *
 * (c) 2016 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file qqutil/qqmactoolbar.cpp
 * Source file for QQMacToolBarUtils
 */

#import <AppKit/AppKit.h>
#include <QMacToolBar>
#include <QMacToolBarItem>
#include "qqmactoolbardelegate.h"
#include "qqmactoolbarutils.h"
#include "qqtoolbar.h"

/**
 * Constructor.  Never called in practice, all methods are static.
 */
QQMacToolBarUtils::QQMacToolBarUtils()
{

}

/**
 * Copy the appearance settings from one Mac toolbar to another.  Affects
 * icon/text/both choice and icon size.
 *
 * @param src The toolbar whose settings are to be copied
 * @param dest The toolbar whose settings are to be set
 */
void QQMacToolBarUtils::copyState(QMacToolBar *src, QMacToolBar *dest)
{
    if (!src || !dest) {
        return;
    }
    [dest->nativeToolbar() setDisplayMode:[src->nativeToolbar() displayMode]];
    [dest->nativeToolbar() setSizeMode:[src->nativeToolbar() sizeMode]];
}

/**
 * Create and set the delegate for the given toolbar, in order to validate the
 * enabled/disabled state of its items.
 *
 * @param toolBar The toolbar for which to set the delegate
 */
void QQMacToolBarUtils::setDelegate(QQToolBar *toolBar)
{
    QQMacToolbarDelegate *delegate = [[QQMacToolbarDelegate alloc] init];
    delegate->toolBar = toolBar;
    [toolBar->macToolBar()->nativeToolbar() setDelegate:delegate];
}

/**
 * Get the current display mode of a toolbar for saving in the application
 * preferences.
 *
 * @param toolBar The toolbar for which to fetch the display mode
 * @return "iconAndLabel", "iconOnly", "labelOnly", or "default"
 */
QString QQMacToolBarUtils::displayMode(QMacToolBar *toolBar)
{
    NSToolbarDisplayMode mode = [toolBar->nativeToolbar() displayMode];
    if (mode == NSToolbarDisplayModeIconAndLabel) {
        return "iconAndLabel";
    }
    else if (mode == NSToolbarDisplayModeIconOnly) {
        return "iconOnly";
    }
    else if (mode == NSToolbarDisplayModeLabelOnly) {
        return "labelOnly";
    }
    else {
        return "default";
    }
}

/**
 * Get the internal identifier string of a toolbar item.
 *
 * @param item The toolbar item for which to fetch the identifier
 * @return An identifier string
 */
QString QQMacToolBarUtils::identifier(QMacToolBarItem *item)
{
    return QString::fromNSString([item->nativeToolBarItem() itemIdentifier]);
}

/**
 * Set the display mode of a toolbar.
 *
 * @param toolBar The toolbar to be updated
 * @param sizeMode "iconAndLabel", "iconOnly", "labelOnly" or "default"
 */
void QQMacToolBarUtils::setDisplayMode(QMacToolBar *toolBar, const QString &displayMode)
{
    NSToolbarDisplayMode mode;
    if (displayMode == "iconAndLabel") {
        mode = NSToolbarDisplayModeIconAndLabel;
    }
    else if (displayMode == "iconOnly") {
        mode = NSToolbarDisplayModeIconOnly;
    }
    else if (displayMode == "labelOnly") {
        mode = NSToolbarDisplayModeLabelOnly;
    }
    else {
        mode = NSToolbarDisplayModeDefault;
    }
    [toolBar->nativeToolbar() setDisplayMode:mode];
}

/**
 * Set the size mode of a toolbar (24 or 32 pixels).
 *
 * @param toolBar The toolbar to be updated
 * @param sizeMode "regular", "small", or "default"
 */
void QQMacToolBarUtils::setSizeMode(QMacToolBar *toolBar, const QString &sizeMode)
{
    NSToolbarSizeMode mode;
    if (sizeMode == "regular") {
        mode = NSToolbarSizeModeRegular;
    }
    else if (sizeMode == "small") {
        mode = NSToolbarSizeModeSmall;
    }
    else {
        mode = NSToolbarSizeModeDefault;
    }
    [toolBar->nativeToolbar() setSizeMode:mode];
}

/**
 * Set the tooltip of a toolbar item.
 *
 * @param item The toolbar item to be updated
 * @param text The text to use for the tooltip
 */
void QQMacToolBarUtils::setToolTip(QMacToolBarItem *item, const QString &text)
{
    [item->nativeToolBarItem() setToolTip:text.toNSString()];
}

/**
 * Get the current size mode of a toolbar for saving in the application
 * preferences.
 *
 * @param toolBar The toolbar for which to fetch the size mode
 * @return "regular", "small", or "default"
 */
QString QQMacToolBarUtils::sizeMode(QMacToolBar *toolBar)
{
    NSToolbarSizeMode mode = [toolBar->nativeToolbar() sizeMode];
    if (mode == NSToolbarSizeModeRegular) {
        return "regular";
    }
    else if (mode == NSToolbarSizeModeSmall) {
        return "small";
    }
    else {
        return "default";
    }
}

/**
 * Get the currently included toolbar items.  Need to ask the native toolbar
 * directly, because QMacToolBar isn't informed of changes made in the
 * customization menu.
 *
 * @param toolBar The toolbar for which to get an item listing
 * @return The items included in the current toolbar configuration
 */
QList<QMacToolBarItem *> QQMacToolBarUtils::includedItems(QMacToolBar *toolBar)
{
    QList<QMacToolBarItem *> items;
    for (NSToolbarItem *item in [toolBar->nativeToolbar() items]) {
        QString identifier = QString::fromNSString([item itemIdentifier]);
        QMacToolBarItem *toolButton = reinterpret_cast<QMacToolBarItem *>(identifier.toULongLong());
        items.append(toolButton);
    }
    return items;
}
