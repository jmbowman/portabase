/*
 * portabase.h
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef PORTABASE_H
#define PORTABASE_H

#include <qpe/filemanager.h>
#include <qmainwindow.h>
#include <qpixmap.h>
#include "datatypes.h"

class Database;
class FileSelector;
class QAction;
class QPopupMenu;
class QToolBar;
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

public slots:
    void setDocument(const QString&);

private slots:
    void newFile(const DocLnk &);
    void openFile(const DocLnk &);
    bool editColumns();
    void addRow();
    void editRow();
    void deleteRow();
    void save();
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

protected:
    void closeEvent(QCloseEvent *e);

private:
    void fileOpen();
    void updateCaption(const QString &name=QString::null);
    void rebuildViewMenu();
    void updateViewMenu();
    void rebuildSortMenu();
    void closeViewer();

private:
    Database *db;
    QWidgetStack *mainStack;
    FileSelector *fileSelector;
    QToolBar *menu;
    QToolBar *toolbar;
    QAction *fileSaveAction;
    QAction *rowEditAction;
    QAction *rowDeleteAction;
    QPopupMenu *view;
    QPopupMenu *sort;
    DocLnk *doc;
    ViewDisplay *viewer;
    bool newdb;
    int allColumnsViewId;
    int editViewId;
    int deleteViewId;
    QStringList viewNames;
    IntList viewIds;
    int editSortId;
    int deleteSortId;
    QStringList sortNames;
    IntList sortIds;
    bool isEdited;
};

#endif
