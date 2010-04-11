/*
 * viewdisplay.h
 *
 * (c) 2002-2004,2009-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file viewdisplay.h
 * Header file for ViewDisplay
 */

#ifndef VIEWDISPLAY_H
#define VIEWDISPLAY_H

#include <QTime>
#include <QWidget>

class Database;
class PortaBase;
class QButtonGroup;
class QKeyEvent;
class QLabel;
class QSpinBox;
class QStackedWidget;
class QString;
class QToolButton;
class QTreeWidget;
class QTreeWidgetItem;
class View;

#define PAGE_BUTTON_COUNT 5

/**
 * The main data display widget in PortaBase.  Shows a table which displays
 * records matching the current filter as rows, and fields in the current
 * view as columns.  By default records are split between pages containing
 * a maximum number of rows each (with navigation controls at the bottom of
 * the widget), but an option in the Preferences dialog lets the user force
 * all rows to be shown in a single page and hide the navigation widget row.
 */
class ViewDisplay: public QWidget
{
    Q_OBJECT
public:
    ViewDisplay(PortaBase *pbase, QWidget *parent = 0);

    void setDatabase(Database *dbase);
    void updateTable();
    void updateButtons();
    void updateButtonSizes();
    void saveViewSettings();
    void setView(const QString &name, bool applyDefaults=false);
    void setSorting(const QString &name);
    void setFilter(const QString &name);
    void closeView();
    void deleteAllRows();
    void exportToCSV(const QString &filename);
    void exportToXML(const QString &filename);
    QString toHtml();
    void allowBooleanToggle(bool flag);
    void usePages(bool flag);
    void showWithSingleClick(bool flag);

public slots:
    void addRow();
    bool editRow(int rowId=-1, bool copy=false);
    void deleteRow();
    void slideshow();
    void viewRow();

protected:
    void keyPressEvent(QKeyEvent *e);
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
    void cellPressed(QTreeWidgetItem *item, int column);
    void cellReleased(QTreeWidgetItem *item, int column);
    void headerPressed(int column);
    void headerReleased(int column);
    void columnResized(int column, int oldWidth, int newWidth);

private:
    PortaBase *portabase; /**< The main application window */
    QStackedWidget *stack; /**< Main widget stack (data display and "no results" label) */
    QTreeWidget *table; /**< Table of data records */
    QLabel *noResults; /**< "No results" placeholder label */
    QWidget *buttonRow; /**< Row of results page navigation controls */
    QSpinBox *rowsPerPage; /**< Records per results page selection widget */
    QToolButton *prevButton; /**< "Previous few pages" button */
    QToolButton *nextButton; /**< "Next few pages" button */
    QButtonGroup *buttonGroup; /**< The group of page navigation buttons */
    QToolButton *pageButtons[PAGE_BUTTON_COUNT]; /**< Array of page navigation buttons */
    Database *db; /**< The database in use */
    View *view; /**< The currently selected database view */
    int currentPage; /**< The currently displayed results page number */
    int firstPageButton; /**< Number of the first results page for which a button is currently shown */
    int pressedIndex; /**< Index of the column in which the mouse has been pressed (while waiting for a release) */
    QTime timer; /**< Time elapsed between the last mouse press and release */
    bool booleanToggle; /**< True if boolean field values can be toggled by clicking on them in the display */
    bool paged; /**< False if all records in the view are always shown on one scrolling page */
    bool singleClickShow; /**< True if a single click launches the row viewer */
};

#endif
