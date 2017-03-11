/*
 * qqtoolbar.cpp
 *
 * (c) 2016-2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file qqutil/qqtoolbar.cpp
 * Source file for QQToolBar
 */

#include <QAction>
#include <QMainWindow>
#include <QSettings>
#include "qqtoolbar.h"
#include "qqtoolbarstretch.h"

#if defined(Q_OS_MAC)
#include <QMacToolBar>
#include <QMacToolBarItem>
#include <QWindow>
#include "qqmactoolbarutils.h"
#else
#include <QToolBar>
#endif

QQToolBar *QQToolBar::currentToolBar = 0;

/**
 * Constructor.
 *
 * @param parent The main window to which the toolbar will be attached
 * @param name The name of the toolbar (must be different for each one, and
 *             should be valid as a QSettings group name)
 */
QQToolBar::QQToolBar(QMainWindow *parent, const QString &name)
  : QObject(parent), mainWindow(parent), identifier(name)
{
#if defined(Q_OS_MAC)
    toolBar = new QMacToolBar(name, parent);
    // Make sure each toolbar is attached so it can be cleaned up before exit
    mainWindow->window()->winId();
    QQMacToolBarUtils utils;
    utils.setDelegate(this);
#else
    toolBar = mainWindow->addToolBar(name);
    toolBar->setObjectName(name);
    toolBar->toggleViewAction()->setEnabled(false);
    toolBar->toggleViewAction()->setVisible(false);
#endif
}

#if defined(Q_OS_MAC)
/**
 * Convert a Mac toolbar item activation signal into a triggered signal for
 * the original QAction.
 */
void QQToolBar::activated()
{
    QMacToolBarItem *item = static_cast<QMacToolBarItem *>(sender());
    QAction *action = itemActionMap[item];
    if (action->isCheckable()) {
        action->toggle();
        emit action->triggered(action->isChecked());
    }
    else {
        emit action->triggered();
    }
}

/**
 * Determine if the specified toolbar item should currently be enabled.
 * Based on the state of the corresponding QAction.
 *
 * @param item A toolbar item
 * @return true if the item should be enabled, false otherwise
 */
bool QQToolBar::isEnabled(QMacToolBarItem *item)
{
    if (!item || !itemActionMap.contains(item)) {
        return true;
    }
    QAction *action = itemActionMap[item];
    return action->isEnabled();
}

/**
 * Get the QMacToolBarItem which wraps the NSToolbarItem with the given identifier.
 *
 * @param identifier The identifier of the NSToolbarItem whose Qt wrapper is desired
 * @return The corresponding Qt wrapper item, or zero if none exists
 */
QMacToolBarItem *QQToolBar::itemFor(const QString &identifier)
{
    if (!(nativeItemMap.contains(identifier))) {
        return 0;
    }
    return nativeItemMap[identifier];
}

/**
 * Get the QMacToolBar associated with this toolbar.
 *
 * @return The underlying QMacToolBar
 */
QMacToolBar *QQToolBar::macToolBar()
{
    return toolBar;
}
#endif

/**
 * Add an action to the toolbar.  Correctly accounts for quirks on Maemo and
 * Mac OS X.
 *
 * @param action The action to be added
 */
void QQToolBar::add(QAction *action)
{
#ifdef MOBILE
    new QQToolBarStretch(toolBar, action);
#endif
#if defined (Q_OS_MAC)
    // Remove any trailing ellipsis
    QString text = action->text().remove(QChar(8230));
    QMacToolBarItem *item = toolBar->addItem(action->icon(), text);
    item->setObjectName(action->objectName());
    item->setSelectable(action->isCheckable());
    QQMacToolBarUtils::setToolTip(item, text);
    itemActionMap[item] = action;
    QString identifier = QQMacToolBarUtils::identifier(item);
    nativeItemMap[identifier] = item;
    connect(item, &QMacToolBarItem::activated, this, &QQToolBar::activated);
#else
    toolBar->addAction(action);
#endif
#ifdef MOBILE
    new QQToolBarStretch(toolBar, action);
#endif
}

/**
 * Load saved toolbar configuration options.  Primarily called by
 * QQMenuHelper, and currently only does anything on Mac OS X (where it
 * handles icon size, icon and/or text preference, and the set of buttons
 * currently shown).
 *
 * @param settings The QSettings object from which to load the settings.
 */
void QQToolBar::loadSettings(QSettings *settings)
{
#if defined(Q_OS_MAC)
    settings->beginGroup(identifier);
    QString displayMode = settings->value("displayMode").toString();
    QQMacToolBarUtils::setDisplayMode(toolBar, displayMode);
    QString sizeMode = settings->value("sizeMode").toString();
    QQMacToolBarUtils::setSizeMode(toolBar, sizeMode);
    QVariant itemNames = settings->value("items");
    if (!(itemNames.isNull())) {
        QHash<QString, QMacToolBarItem *> nameToItem;
        foreach(QMacToolBarItem *item, toolBar->allowedItems()) {
            nameToItem[item->objectName()] = item;
        }
        QList<QMacToolBarItem *> items;
        foreach(QString itemName, itemNames.toStringList()) {
            if (nameToItem.contains(itemName)) {
                items.append(nameToItem.value(itemName));
            }
        }
        toolBar->setItems(items);
    }
    settings->endGroup();
#else
    Q_UNUSED(settings);
#endif
}

/**
 * Save the toolbar's current configuration so it can be restored the next
 * time the application is run.
 *
 * @param settings the QSettings object to be updated
 */
void QQToolBar::saveSettings(QSettings *settings)
{
#if defined(Q_OS_MAC)
    settings->beginGroup(identifier);
    settings->setValue("displayMode", QQMacToolBarUtils::displayMode(toolBar));
    settings->setValue("sizeMode", QQMacToolBarUtils::sizeMode(toolBar));
    QStringList itemNames;
    foreach (QMacToolBarItem *item, QQMacToolBarUtils::includedItems(toolBar)) {
        itemNames.append(item->objectName());
    }
    settings->setValue("items", itemNames);
    settings->endGroup();
#else
    Q_UNUSED(settings);
#endif
}

/**
 * Hide this tolbar, without making another one visible.
 */
void QQToolBar::hide()
{
#ifndef Q_OS_MAC
    toolBar->hide();
#endif
}

/**
 * Show this toolbar.  Hides any other toolbar which may currently be visible.
 */
void QQToolBar::show()
{
    if (QQToolBar::currentToolBar == this) {
        return;
    }
#if defined(Q_OS_MAC)
    if (QQToolBar::currentToolBar) {
        QQMacToolBarUtils::copyState(QQToolBar::currentToolBar->toolBar, toolBar);
    }
    toolBar->attachToWindow(mainWindow->window()->windowHandle());
#else
    if (QQToolBar::currentToolBar) {
        QQToolBar::currentToolBar->toolBar->hide();
    }
    toolBar->show();
#endif
    QQToolBar::currentToolBar = this;
}
