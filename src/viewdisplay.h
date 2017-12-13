/*
 * viewdisplay.h
 *
 * (c) 2002-2004,2009-2013,2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
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

#include <QModelIndex>
#include <QTime>
#include <QWidget>
#include "qqutil/qqspinbox.h"

class CSVUtils;
class Database;
class DataModel;
class PortaBase;
class QButtonGroup;
class QKeyEvent;
class QLabel;
class QSettings;
class QStackedWidget;
class QString;
class QToolButton;
class QTreeView;
class View;
#if defined(Q_OS_ANDROID)
#include <QScroller>
class ActionBar;
#endif

#define PAGE_BUTTON_COUNT 5

/**
 * The main data display widget in %PortaBase.  Shows a table which displays
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
    void resetTable();
    void updateButtonSizes();
    void saveViewSettings();
    void setView(const QString &name, bool applyDefaults=false);
    void setSorting(const QString &name);
    void setFilter(const QString &name);
    void closeView();
    void deleteAllRows();
    void exportToCSV(const QString &filename, CSVUtils *csv);
    void exportToHTML(const QString &filename);
    void exportToXML(const QString &filename);
    QString toPrintHTML();
    void updateColWidths();
    int rowCount();
    int columnCount();
    void updatePreferences(QSettings *settings);
#if defined(Q_OS_ANDROID)
    ActionBar *actionBar();
#endif

public slots:
    void addRow();
    bool editRow(int id=-1, bool copy=false, QWidget *parent=0);
    void deleteRow(int id=-1);
    void slideshow();
    void viewRow();
    void showStatistics(bool allColumns=false, QWidget *parent=0);

protected:
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);

private:
    void setEdited(bool y);
    int selectedRowIndex();
    int selectedRowId();
    void sort(int column);
    void usePages(bool flag);

private slots:
    void changePage(int id);
    void nextPages();
    void previousPages();
    void updateRowsPerPage(int rpp);
    void rowSelected();
    void cellPressed(const QModelIndex &index);
    void cellReleased(const QModelIndex &index);
    void headerPressed(int column);
    void headerReleased(int column);
    void columnResized(int column, int oldWidth, int newWidth);
    void updateButtons(int currentPage, int totalPages);
    void matchNewView(View *view);
    void tableChanged();
    void screenGeometryChanged(const QRect &geometry);
    void incrementalSearch();
#if defined(Q_OS_ANDROID)
    void scrollerStateChanged(QScroller::State newState);
#endif

private:
    PortaBase *portabase; /**< The main application window */
    QStackedWidget *stack; /**< Main widget stack (data display and "no results" label) */
    QTreeView *table; /**< Table of data records */
    DataModel *model; /**< The model of the currently displayed data used by the table widget */
    QLabel *noResults; /**< "No results" placeholder label */
    QWidget *buttonRow; /**< Row of results page navigation controls */
    QQSpinBox *rowsPerPage; /**< Records per results page selection widget */
    QToolButton *prevButton; /**< "Previous few pages" button */
    QToolButton *nextButton; /**< "Next few pages" button */
    QButtonGroup *buttonGroup; /**< The group of page navigation buttons */
    QToolButton *pageButtons[PAGE_BUTTON_COUNT]; /**< Array of page navigation buttons */
    bool propogateColWidths; /**< True if column width resizes are to be passed to the view */
    int pressedIndex; /**< Index of the column in which the mouse has been pressed (while waiting for a release) */
    int pressedHeader; /**< Index of the column header which has been pressed (while waiting for a release) */
    QTime timer; /**< Time elapsed between the last mouse press and release */
    bool booleanToggle; /**< True if boolean field values can be toggled by clicking on them in the display */
    bool paged; /**< False if all records in the view are always shown on one scrolling page */
    bool singleClickShow; /**< True if a single click launches the row viewer */
#if defined(Q_OS_ANDROID)
    ActionBar *androidActionBar;
#endif
};

#endif
