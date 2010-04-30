/*
 * qqmenuhelper.h
 *
 * (c) 2005-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file qqutil/qqmenuhelper.h
 * Header file for QQMenuHelper
 */

#ifndef QQMENUHELPER_H
#define QQMENUHELPER_H

#include <QHash>
#include <QIcon>
#include <QObject>
#include <QRegExp>
#include <QStringList>

class QAction;
class QMainWindow;
class QMenu;
class QSettings;
class QToolBar;

#define MAX_RECENT_FILES 5

/**
 * Helps create and manage the menus for a typical document-based
 * application.
 * <p>The following icons must be present as resources:</p>
 * <ul>
 * <li>icons/new.png</li>
 * <li>icons/open.png</li>
 * <li>icons/quit.png</li>
 * <li>icons/close.png</li>
 * <li>icons/save.png</li>
 * <li>icons/document.png</li>
 * </ul>
 * <p>The following "File" menu is provided when in the initial
 * "file selector" mode:</p>
 * <ul>
 * <li>New</li>
 * <li>Open</li>
 * <li>Open Recent <i>(submenu)</i></li>
 * <li>Preferences</li>
 * <li>Quit</li>
 * </ul>
 * <p>Also, the following "Help" menu is provided:</p>
 * <ul>
 * <li>Help Contents <i>(on Mac OS X, called "[appname] Help" instead, and
 *    moved to the expected location)</i></li>
 * <li>About [appname]</li>
 * <li>About Qt</li>
 * </ul>
 * <p>The main help file is expected to be a resource located at
 * help/html/[appname].html.  Translated versions can be specified in the
 * .qrc file.</p>
 */
class QQMenuHelper : public QObject
{
    Q_OBJECT
public:
    /** Enumeration of actions created by this class */
    enum Action {
        New = 0,
        Open = 1,
        Save = 2,
        Recent1 = 3,
        Recent2 = 4,
        Recent3 = 5,
        Recent4 = 6,
        Recent5 = 7,
        Separator = 8,
        Close = 9,
        Preferences = 10,
        Quit = 11,
        Help = 12,
        About = 13,
        AboutQt = 14
    };
    QQMenuHelper(QMainWindow *window, QToolBar *toolbar,
                 const QString &fileDescription, const QString &fileExtension,
                 bool newFileLaunchesDialog=false);

    void loadSettings(QSettings *settings);
    void saveSettings(QSettings *settings);
    void updateRecentMenu();
    void updateFileSelectorMenu();
    void updateDocumentFileMenu();
    void addToFileMenu(QAction *action);
    void addToToolBar(QAction *action);
    void saveChangesPrompt();
    QMenu *createMenu(QMainWindow *mainWindow);
    QMenu *fileMenu();
    QMenu *helpMenu();
    QAction *action(Action actionId);
    QString createNewFile(const QString &fileDescription=QString::null,
                          const QString &fileExtension=QString::null);
    static QString getLastDir(QSettings *settings);
    static QString menuText(QString text);

signals:
    void newFile(const QString &);
    void openFile(const QString &);
    void recentFilesChanged();
    void saveFile();
    void closeFile();
    void editPreferences();
    void quit();
    void aboutApplication();

public slots:
    void setEdited(bool y);

private slots:
    void showHelp();
    void aboutQt();
    void emitNewFile();
    void emitOpenFile();
    void openRecent();

private:
    QPixmap darkenPixmap(const QPixmap &pixmap);

private:
    QMainWindow *mainWindow; /**< The main application window */
    QToolBar *mainToolBar; /**< The main toolbar */
    QIcon docIcon; /**< The application document icon (used in Mac titlebar) */
    QIcon modifiedDocIcon; /**< The application document icon when there are unsaved changes */
    QString description; /**< The description of the document file type */
    QString extension; /**< The extension of the document file type */
    QHash<Action, QAction*> actions; /**< Lookup hash for the menu actions */
    QAction *fileNewAction; /**< Action for creating a new file */
    QAction *fileOpenAction; /**< Action for opening an existing file */
    QAction *fileSaveAction; /**< Action for saving the current file */
    QAction* recentActions[5]; /**< Actions for opening recently used files */
    QList<QAction *> extraFileActions; /**< Additional actions added to the File menu by the application */
    QAction *fileSeparatorAction; /**< Separator that appears before "Close" in the "File" menu */
    QAction *closeAction; /**< Action for closing an open document */
    QAction *prefsAction; /**< Action for launching the application preferences dialog */
    QAction *quitAction; /**< Action for exiting the application */
    QAction *insertionPoint; /**< Current first item in the "File" menu footer */
    QAction *helpAction; /**< Action for displaying the main help file */
    QAction *aboutAction; /**< Action for displaying info about the app */
    QAction *aboutQtAction; /**< Action for displaying info about Qt */
    QMenu *file; /**< The "File" menu */
    QMenu *recent; /**< The "Open Recent" menu */
    QMenu *help; /**< The "Help" menu */
    QStringList recentFiles; /**< The most recently opened files */
    static QRegExp menuRegExp; /**< Regular expression for menu text processing */
};

#endif
