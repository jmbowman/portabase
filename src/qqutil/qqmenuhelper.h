/*
 * qqmenuhelper.h
 *
 * (c) 2005-2010,2016 by Jeremy Bowman <jmbowman@alum.mit.edu>
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

#if defined(Q_OS_MAC)
#include <QStatusBar>
#endif

class QAction;
class QMainWindow;
class QMenu;
class QSettings;
class QQToolBar;

#define MAX_RECENT_FILES 10

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
        Recent6 = 8,
        Recent7 = 9,
        Recent8 = 10,
        Recent9 = 11,
        Recent10 = 12,
        Separator = 13,
        Close = 14,
        Preferences = 15,
        Quit = 16,
        Help = 17,
        About = 18,
        AboutQt = 19
    };
    QQMenuHelper(QMainWindow *window, const QString &fileDescription,
                 const QString &fileExtension,
                 bool newFileLaunchesDialog=false);

    void loadSettings(QSettings *settings);
    void saveSettings(QSettings *settings);
    void updateRecentMenu();
    void updateForFileSelector();
    void updateForDocument();
    void addToFileMenu(QAction *action);
    void addToDocumentToolBar(QAction *action);
    void addToFileSelectorToolBar(QAction *action);
    int saveChangesPrompt();
    void setHelpLocation(const QString &urlTemplate, const QStringList &locales);
    QMenu *createMenu(QMainWindow *mainWindow);
    QMenu *fileMenu();
    QMenu *helpMenu();
    QAction *action(Action actionId);
    QString createNewFile(const QString &fileDescription=QString::null,
                          const QString &fileExtension=QString::null);
    void opened(const QString &file);
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
    void clearRecentMenu();
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
    QQToolBar *documentToolBar; /**< The toolbar shown when a document is open */
    QQToolBar *fileSelectorToolBar; /**< The toolbar for the file selector screen */
    QIcon docIcon; /**< The application document icon (used in Mac titlebar) */
    QIcon modifiedDocIcon; /**< The application document icon when there are unsaved changes */
    QString description; /**< The description of the document file type */
    QString extension; /**< The extension of the document file type */
    QHash<Action, QAction*> actions; /**< Lookup hash for the menu actions */
    QAction *fileNewAction; /**< Action for creating a new file */
    QAction *fileOpenAction; /**< Action for opening an existing file */
    QAction *fileSaveAction; /**< Action for saving the current file */
    QAction* recentActions[MAX_RECENT_FILES]; /**< Actions for opening recently used files */
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
    QString helpUrlTemplate; /**< Template for the main help URL, just add locale */
    QStringList helpLocales; /**< The locales for which the help files have been translated */
    static QRegExp menuRegExp; /**< Regular expression for menu text processing */
};

#endif
