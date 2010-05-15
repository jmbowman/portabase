/*
 * portabase.h
 *
 * (c) 2002-2004,2008-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file portabase.h
 * Header file for PortaBase (main window class)
 */

#ifndef PORTABASE_H
#define PORTABASE_H

#include <QMainWindow>
#include <QSettings>
#include <QStringList>
#include "datatypes.h"
#include "importdialog.h"
#include "qqutil/qqmenuhelper.h"

class Database;
class MenuActions;
class QAction;
class QGroupBox;
class QMenu;
class QMenuBar;
class QPrinter;
class QPushButton;
class QScrollArea;
class QStackedWidget;
class ViewDisplay;
class VSFManager;

typedef QList<QAction*> ActionList;

/**
 * The main window of the PortaBase application.  Most interaction with the
 * menus and toolbars is managed here.
 */
class PortaBase: public QMainWindow
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "net.sourceforge.portabase")
public:
    PortaBase(QWidget *parent = 0);
    ~PortaBase();

    void setEdited(bool y);
    void setRowSelected(bool y);
    void updateSortMenu();
    void updateFilterMenu();
    QString createNewFile(const QString &fileDescription=QString::null,
                          const QString &fileExtension=QString::null);
    void changeView(const QString &name);
    void changeSorting(const QString &name);
    void changeFilter(const QString &name);

public Q_SLOTS:
    void openFile(const QString &file);
    Q_SCRIPTABLE void mime_open(const QString &url);
    Q_SCRIPTABLE int top_application();

private slots:
    void newFile(const QString &file);
    void updateRecentFileButtons();
    bool editColumns();
    void editEnums();
    void viewProperties();
    void editPreferences();
    void deleteRow();
    void copyRow();
    void deleteAllRows();
    void save();
    void changePassword();
    void import();
    void dataImport();
    void dataExport();
    void viewAllColumns();
    void changeView(QAction *action);
    void addView();
    void editView();
    void deleteView();
    void changeSorting(QAction *action);
    void addSorting();
    void editSorting();
    void deleteSorting();
    void viewAllRows();
    void changeFilter(QAction *action);
    void addFilter();
    void editFilter();
    void deleteFilter();
    void simpleFilter();
    void aboutPortaBase();
    void quit();
    void openAt(QAction *where);
    void printPreview();
    void print();
    void print(QPrinter *p);
    void changeView();
    void changeSorting();
    void changeFilter();
    void toggleFullscreen();

protected:
    void closeEvent(QCloseEvent *e);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    bool event (QEvent *event);

private:
    void restoreWindowSettings(QSettings *settings);
    void saveWindowSettings(QSettings *settings);
    void createFile(ImportDialog::DataSource source, const QString &file=QString::null);
    void finishNewFile(Database *db);
    void updateCaption();
    void rebuildViewMenu();
    void updateViewMenu();
    void rebuildSortMenu();
    void rebuildFilterMenu();
    void closeViewer();
    void showFileSelector();
    void showDataViewer();
    QSettings *getSettings();
    void createFillerActions();
    void showAllFillerActions();

private:
    Database *db; /**< The currently open database, if any */
    bool readOnly; /**< True if the open file is read-only, false otherwise */
    QStackedWidget *mainStack; /**< Main widget stack (data display and "No file selected" label) */
    QScrollArea *noFileWidget; /**< The main widget shown when no file is open */
    QPushButton *newButton; /**< The button for creating a new file */
    QPushButton *openButton; /**< The button for picking an existing file to open */
    QPushButton *importButton; /**< The button for importing from another format */
    QGroupBox *recentBox; /**< The grouping box containing recentButtons */
    QPushButton* recentButtons[MAX_RECENT_FILES]; /**< Buttons on noFileWidget representing the most recently opened files */
    QToolBar *toolbar; /**< The application toolbar */
    MenuActions *ma; /**< Menu and toolbar action definition manager */
    QQMenuHelper *mh; /**< Support code for any document-based application */
    QAction *importAction; /**< "Import" menu action (for creating a new file) */
    QAction *changePassAction; /**< "Change Password" menu action */
    QAction *dataImportAction; /**< "Import" menu action (in an existing file) */
    QAction *exportAction; /**< "Export" menu action */
    QAction *deleteRowsAction; /**< "Delete Rows In Filter" menu action */
    QAction *editColsAction; /**< "Edit Columns" menu action */
    QAction *manageEnumsAction; /**< "EditEnums" menu action */
    QAction *slideshowAction; /**< "Slideshow" menu action */
    QAction *propsAction; /**< File "Properties" menu action */
    QAction *fileSeparatorAction; /**< Separator used in the "File" menu */
    QAction *printPreviewAction; /**< "Print Preview" menu action */
    QAction *printAction; /**< "Print" menu action */
    QAction *rowAddAction; /**< Row "Add" menu/toolbar action */
    QAction *rowEditAction; /**< Row "Edit" menu/toolbar action */
    QAction *rowDeleteAction; /**< Row "Delete" menu/toolbar action */
    QAction *rowCopyAction; /**< Row "Copy" menu/toolbar action */
    QAction *rowViewAction; /**< Row "View" menu action */
    QAction *viewAddAction; /**< View "Add" menu action */
    QAction *viewEditAction; /**< View "Edit" menu action */
    QAction *viewDeleteAction; /**< View "Delete" menu action */
    QAction *viewAllColsAction; /**< View "All Columns" menu action */
    QAction *sortAddAction; /**< Sorting "Add" menu action */
    QAction *sortEditAction; /**< Sorting "Edit" menu action */
    QAction *sortDeleteAction; /**< Sorting "Delete" menu action */
    QAction *findAction; /**< "Quick Filter" menu/toolbar action */
    QAction *filterAddAction; /**< Filter "Add" menu action */
    QAction *filterEditAction; /**< Filter "Edit" menu action */
    QAction *filterDeleteAction; /**< Filter "Delete" menu action */
    QAction *filterAllRowsAction; /**< Filter "All Rows" menu action */
    QAction *viewsAction; /**< Toolbar "Views" action */
    QAction *sortingsAction; /**< Toolbar "Sortings" action */
    QAction *filtersAction; /**< Toolbar "Filters" action */
    QAction *fullscreenAction; /**< Toolbar "Fullscreen" action */
    QAction* fillerActions[6]; /**< Toolbar filler actions; Mac toolbar quirk */
    QMenu *row; /**< "Row" Menu */
    QMenu *view; /**< "View" Menu */
    QMenu *sort; /**< "Sort" Menu */
    QMenu *filter; /**< "Filter" Menu */
    QString doc; /**< Path of the current database file */
    ViewDisplay *viewer; /**< Data display grid widget */
    QStringList viewNames; /**< List of all user-named views in the database */
    ActionList viewActions; /**< List of user-created view selection actions */
    QStringList sortNames; /**< List of all user-named sortings in the database */
    ActionList sortActions; /**< List of user-created sorting selection actions */
    QStringList filterNames; /**< List of all user-named filters in the database */
    ActionList filterActions; /**< List of user-created filter selection actions */
    bool confirmDeletions; /**< True if deletion actions should trigger confirmation dialogs */
    bool booleanToggle; /**< True if clicking on a boolean field display should toggle its value */
    QPrinter *printer; /**< The last-used printer settings (for this application instance) */
    VSFManager *vsfManager; /**< Dialog for managing the view, sorting, and filter selections */
};

#endif
