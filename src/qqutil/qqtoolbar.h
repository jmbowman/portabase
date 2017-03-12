/*
 * qqtoolbar.h
 *
 * (c) 2016-2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file qqutil/qqtoolbar.h
 * Header file for QQToolBar
 */

#ifndef QQTOOLBAR_H
#define QQTOOLBAR_H

#include <QObject>

class QAction;
class QMainWindow;
class QSettings;

#if defined(Q_OS_MAC)
#include <QMap>
class QMacToolBar;
class QMacToolBarItem;
#else
class QToolBar;
#endif

/**
 * A toolbar that behaves correctly across different platforms, including
 * Mac OS X and Maemo.  Is not itself a QWidget, as it wraps various underlying
 * toolbar implementations.
 */
class QQToolBar: public QObject
{
    Q_OBJECT
public:
    QQToolBar(QMainWindow *parent, const QString &name);

    void add(QAction *action);
    void loadSettings(QSettings *settings);
    void saveSettings(QSettings *settings);
    void hide();
    void show();
#if defined(Q_OS_MAC)
    bool isEnabled(QMacToolBarItem *item);
    QMacToolBarItem *itemFor(const QString &identifier);
    QMacToolBar *macToolBar();
private slots:
    void activated();
#endif

private:
    QMainWindow *mainWindow; /**< The application's main window */
    QString identifier; /**< The name used to identify this toolbar in saved settings */
    static QQToolBar *currentToolBar; /**< The currently displayed toolbar, if any */
#if defined(Q_OS_MAC)
    QMacToolBar *toolBar; /**< Toolbar on Mac OS X */
    void *validator; /**< Mac OS X toolbar item disabler */
    QMap<QMacToolBarItem*,QAction*> itemActionMap; /**< Mapping of toolbar items to source actions */
    QMap<QString,QMacToolBarItem*> nativeItemMap; /**< Mapping of native toolbar item identifiers to Qt toolbar items */
#else
    QToolBar *toolBar; /**< Toolbar on non-Mac platforms */
#endif
};

#endif
