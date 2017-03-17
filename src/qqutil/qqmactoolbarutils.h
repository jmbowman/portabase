/*
 * qqmactoolbarutils.h
 *
 * (c) 2016-2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file qqutil/qqmactoolbarutils.h
 * Header file for QQMacToolBarUtils
 */

#ifndef QQMACTOOLBAR_H
#define QQMACTOOLBAR_H

#include <QList>

class QMacToolBar;
class QMacToolBarItem;
class QQToolBar;

/**
 * Utilities for QMacToolBar (from macextras) to perform some useful
 * functions like set tooltips, save and restore toolbar customizations,
 * etc.
 */
class QQMacToolBarUtils
{
public:
    QQMacToolBarUtils();

    static void copyState(QMacToolBar *src, QMacToolBar *dest);
    static QString displayMode(QMacToolBar *toolBar);
    static QString identifier(QMacToolBarItem *item);
    static void setDelegate(QQToolBar *qqToolBar);
    static void setDisplayMode(QMacToolBar *toolBar, const QString &displayMode);
    static void setSizeMode(QMacToolBar *toolBar, const QString &sizeMode);
    static void setToolTip(QMacToolBarItem *item, const QString &text);
    static QString sizeMode(QMacToolBar *toolBar);
    static QList<QMacToolBarItem *> includedItems(QMacToolBar *toolBar);
};

#endif
