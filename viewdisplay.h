/*
 * viewdisplay.h
 *
 * (c) 2002-2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef VIEWDISPLAY_H
#define VIEWDISPLAY_H

#include <qdatetime.h>
#include <qvbox.h>

class Database;
class PortaBase;
class QHBox;
class QHButtonGroup;
class QKeyEvent;
class QLabel;
class QListView;
class QListViewItem;
class QPoint;
class QSpinBox;
class QString;
class QToolButton;
class QWidgetStack;
class View;

#define PAGE_BUTTON_COUNT 5

class ViewDisplay: public QVBox
{
    Q_OBJECT
public:
    ViewDisplay(PortaBase *pbase, QWidget *parent = 0, const char *name = 0,
                WFlags f = 0);
    ~ViewDisplay();

    void setDatabase(Database *dbase);
    void updateTable();
    void updateButtons();
    void updateButtonSizes();
    void saveViewSettings();
    void setView(QString name, bool applyDefaults=FALSE);
    void setSorting(QString name);
    void setFilter(QString name);
    void closeView();
    void deleteAllRows();
    void exportToCSV(QString filename);
    void exportToXML(QString filename);
    void allowBooleanToggle(bool flag);
    void addRow();
    void deleteRow();
    bool editRow(int rowId=-1, bool copy=FALSE);
    void usePages(bool flag);

private slots:
    void viewRow();

protected:
    void keyReleaseEvent(QKeyEvent *e);

private:
    void setEdited(bool y);
    int selectedRowIndex();
    int selectedRowId();
    void sort(int column);
    void showStatistics(int column);

private slots:
    void changePage(int id);
    void nextPages();
    void previousPages();
    void updateRowsPerPage(int rpp);
    void rowSelected();
    void cellPressed(QListViewItem *item, const QPoint &point, int column);
    void cellReleased(QListViewItem *item, const QPoint &point, int column);
    void headerPressed(int column);
    void headerReleased(int column);
    void columnResized(int column, int oldWidth, int newWidth);

private:
    PortaBase *portabase;
    QWidgetStack *stack;
    QListView *table;
    QLabel *noResults;
    QHBox *buttonRow;
    QSpinBox *rowsPerPage;
    QToolButton *prevButton;
    QToolButton *nextButton;
    QHButtonGroup *buttonGroup;
    QToolButton *pageButtons[PAGE_BUTTON_COUNT];
    Database *db;
    View *view;
    int currentPage;
    int firstPageButton;
    bool isEdited;
    int pressedIndex;
    QTime timer;
    bool booleanToggle;
    bool paged;
};

#endif
