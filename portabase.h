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

class Database;
class DocLnk;
class FileSelector;
class QAction;
class QPEMenuBar;
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
    void refreshFileList();
    bool editColumns();
    void editEnums();
    void editPreferences();
    void addRow();
    void editRow();
    void deleteRow();
    void copyRow();
    void deleteAllRows();
    void save();
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
    void createFile(const DocLnk &f, int source);
    void finishNewFile(Database *db);
    void updateCaption(const QString &name=QString::null);
    void rebuildViewMenu();
    void updateViewMenu();
    void rebuildSortMenu();
    void rebuildFilterMenu();
    void closeViewer();
    void showFileSelector();
    void showDataViewer();

private:
    Database *db;
    QWidgetStack *mainStack;
    FileSelector *fileSelector;
    QPEMenuBar *menu;
    QToolBar *toolbar;
    QAction *fileNewAction;
    QAction *fileOpenAction;
    QAction *fileDeleteAction;
    QAction *refreshAction;
    QAction *importAction;
    QAction *quitAction;
    QAction *fileSaveAction;
    QAction *dataImportAction;
    QAction *exportAction;
    QAction *deleteRowsAction;
    QAction *editColsAction;
    QAction *manageEnumsAction;
    QAction *prefsAction;
    QAction *closeAction;
    QAction *rowAddAction;
    QAction *rowEditAction;
    QAction *rowDeleteAction;
    QAction *rowCopyAction;
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
    QPopupMenu *row;
    QPopupMenu *view;
    QPopupMenu *sort;
    QPopupMenu *filter;
    QPopupMenu *help;
    DocLnk *doc;
    ViewDisplay *viewer;
    bool newdb;
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
