/*
 * viewdisplay.h
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
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
class QHButtonGroup;
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
    void saveViewSettings();
    void setView(QString name);
    void setSorting(QString name);
    void setFilter(QString name);
    void closeView();
    void deleteAllRows();
    void allowBooleanToggle(bool flag);

public slots:
    void addRow();
    void editRow();
    void deleteRow();

private:
    void setEdited(bool y);
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
};

#endif
