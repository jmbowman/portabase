/*
 * portabase.h
 *
 * (c) 2002-2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef PORTABASE_H
#define PORTABASE_H

#include <qmainwindow.h>
#include <qstringlist.h>
#include <qpixmap.h>
#include "datatypes.h"

class Config;
class Database;
class DocLnk;
class MenuActions;
class PBFileSelector;
class QAction;
class QMenuBar;
class QPopupMenu;
class QToolButton;
class QWidgetStack;
class ViewDisplay;

class PortaBase: public QMainWindow
{
    Q_OBJECT
public:
    PortaBase(QWidget *parent = 0, const char *name = 0, WFlags f = 0);
    ~PortaBase();

    void openFile(const QString &);
    void setEdited(bool y);
    void setRowSelected(bool y);
    static QPixmap getCheckBoxPixmap(int checked);
    static QPixmap getNotePixmap();
    void updateSortMenu();
    void updateFilterMenu();

public slots:
    void setDocument(const QString&);

private slots:
    void newFile();
    void openFile(const DocLnk &);
    void openFile();
    void deleteFile();
    void copyFile();
    void renameFile();
    void refreshFileList();
    void viewList();
    void viewIcons();
    void openRecent(int id);
    bool editColumns();
    void editEnums();
    void viewProperties();
    void editPreferences();
    void addRow();
    void editRow();
    void deleteRow();
    void copyRow();
    void viewRow();
    void deleteAllRows();
    void save();
    void changePassword();
    void import();
    void dataImport();
    void dataExport();
    void viewAllColumns();
    void changeView(int index);
    void addView();
    void editView();
    void deleteView();
    void changeSorting(int index);
    void addSorting();
    void editSorting();
    void deleteSorting();
    void viewAllRows();
    void changeFilter(int index);
    void addFilter();
    void editFilter();
    void deleteFilter();
    void simpleFilter();
    void showHelp();
    void aboutPortaBase();
    void aboutQt();

protected:
    void closeEvent(QCloseEvent *e);

private:
    void fileOpen();
    void createFile(int source);
    void finishNewFile(Database *db);
    void updateCaption(const QString &name=QString::null);
    void rebuildViewMenu();
    void updateViewMenu();
    void rebuildSortMenu();
    void rebuildFilterMenu();
    void closeViewer();
    void showFileSelector();
    void showDataViewer();
    QPopupMenu *getMenuPointer(const QString &menuName);
    QAction *getButtonAction(const QString &buttonName);
    QStringList getRecentFiles(Config &conf);
    void updateRecentFiles(Config &conf);

public:
    static const QColor *evenRowColor;
    static const QColor *oddRowColor;

private:
    Database *db;
    QWidgetStack *mainStack;
    PBFileSelector *fileSelector;
    MenuActions *ma;
    QMenuBar *menu;
    QToolBar *toolbar;
    QAction *fileNewAction;
    QAction *fileOpenAction;
    QAction *fileDeleteAction;
    QAction *fileCopyAction;
    QAction *fileRenameAction;
    QAction *refreshAction;
    QAction *importAction;
    QAction *quitAction;
    QAction *viewListAction;
    QAction *viewIconsAction;
    QAction *fileSaveAction;
    QAction *changePassAction;
    QAction *dataImportAction;
    QAction *exportAction;
    QAction *deleteRowsAction;
    QAction *editColsAction;
    QAction *manageEnumsAction;
    QAction *propsAction;
    QAction *prefsAction;
    QAction *closeAction;
    QAction *rowAddAction;
    QAction *rowEditAction;
    QAction *rowDeleteAction;
    QAction *rowCopyAction;
    QAction *rowViewAction;
    QAction *viewAddAction;
    QAction *viewEditAction;
    QAction *viewDeleteAction;
    QAction *viewAllColsAction;
    QAction *sortAddAction;
    QAction *sortEditAction;
    QAction *sortDeleteAction;
    QAction *findAction;
    QAction *filterAddAction;
    QAction *filterEditAction;
    QAction *filterDeleteAction;
    QAction *filterAllRowsAction;
    QAction *helpAction;
    QAction *aboutAction;
    QAction *aboutQtAction;
    QPopupMenu *file;
    QPopupMenu *recent;
    QPopupMenu *row;
    QPopupMenu *view;
    QPopupMenu *sort;
    QPopupMenu *filter;
    QPopupMenu *help;
    DocLnk *doc;
    ViewDisplay *viewer;
    QStringList viewNames;
    IntList viewIds;
    QStringList sortNames;
    IntList sortIds;
    QStringList filterNames;
    IntList filterIds;
    bool isEdited;
    bool confirmDeletions;
    bool booleanToggle;
    bool needsRefresh;
};

#endif
