/*
 * viewdisplay.cpp
 *
 * (c) 2002-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#if defined(Q_WS_QWS)
#include <qpe/resource.h>
#else
#include "desktop/resource.h"
#endif

#include <math.h>

#include <qhbox.h>
#include <qhbuttongroup.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qmessagebox.h>
#include <qregexp.h>
#include <qspinbox.h>
#include <qstringlist.h>
#include <qtoolbutton.h>
#include <qvbox.h>
#include <qwidgetstack.h>
#include "database.h"
#include "datatypes.h"
#include "noteeditor.h"
#include "portabase.h"
#include "roweditor.h"
#include "rowviewer.h"
#include "shadedlistitem.h"
#include "view.h"
#include "viewdisplay.h"
#include "image/imageviewer.h"
#include "image/slideshowdialog.h"

ViewDisplay::ViewDisplay(PortaBase *pbase, QWidget *parent, const char *name,
    WFlags f) : QVBox(parent, name, f), portabase(pbase), db(0), view(0),
    booleanToggle(FALSE), paged(TRUE)
{
    timer.start();
    stack = new QWidgetStack(this);
    setStretchFactor(stack, 1);
    noResults = new QLabel("<center>" + tr("No results") + "</center>", stack);
    table = new QListView(stack);
    table->setAllColumnsShowFocus(TRUE);
    table->setSorting(-1);
    connect(table, SIGNAL(selectionChanged()), this, SLOT(rowSelected()));
    connect(table, SIGNAL(pressed(QListViewItem*, const QPoint&, int)),
            this, SLOT(cellPressed(QListViewItem*, const QPoint&, int)));
    connect(table, SIGNAL(clicked(QListViewItem*, const QPoint&, int)),
            this, SLOT(cellReleased(QListViewItem*, const QPoint&, int)));
    connect(table, SIGNAL(doubleClicked(QListViewItem*)),
            this, SLOT(viewRow()));
    connect(table, SIGNAL(returnPressed(QListViewItem*)),
            this, SLOT(viewRow()));

    QHeader *header = table->header();
    header->setClickEnabled(TRUE);
    header->setMovingEnabled(FALSE);
    connect(header, SIGNAL(pressed(int)), this, SLOT(headerPressed(int)));
    connect(header, SIGNAL(released(int)), this, SLOT(headerReleased(int)));
    connect(header, SIGNAL(sizeChange(int, int, int)),
            this, SLOT(columnResized(int, int, int)));
    stack->raiseWidget(noResults);

    buttonRow = new QHBox(this);
    rowsPerPage = new QSpinBox(buttonRow);
    rowsPerPage->setRange(1, 9999);
    rowsPerPage->setValue(13);
    connect(rowsPerPage, SIGNAL(valueChanged(int)), this,
            SLOT(updateRowsPerPage(int)));

    prevButton = new QToolButton(LeftArrow, buttonRow);
    connect(prevButton, SIGNAL(clicked()), this, SLOT(previousPages()));
    buttonGroup = new QHButtonGroup(buttonRow);
    buttonGroup->hide();
    buttonGroup->setExclusive(TRUE);
    for (int i=0; i < PAGE_BUTTON_COUNT; i++) {
        pageButtons[i] = new QToolButton(buttonRow);
        pageButtons[i]->setToggleButton(TRUE);
        pageButtons[i]->setUsesTextLabel(TRUE);
        buttonGroup->insert(pageButtons[i], i);
    }
    connect(buttonGroup, SIGNAL(clicked(int)), this, SLOT(changePage(int)));
    nextButton = new QToolButton(RightArrow, buttonRow);
    connect(nextButton, SIGNAL(clicked()), this, SLOT(nextPages()));

    updateButtonSizes();
    currentPage = 1;
    firstPageButton = 1;
}

ViewDisplay::~ViewDisplay()
{

}

void ViewDisplay::usePages(bool flag)
{
    if (paged == flag) {
        return;
    }
    paged = flag;
    if (paged) {
        buttonRow->show();
    }
    else {
        buttonRow->hide();
        currentPage = 1;
        firstPageButton = 1;
    }
    if (view != 0) {
        updateTable();
        updateButtons();
    }
}

void ViewDisplay::updateButtonSizes()
{
    int buttonHeight = rowsPerPage->sizeHint().height();
    prevButton->setFixedHeight(buttonHeight);
    for (int i=0; i < PAGE_BUTTON_COUNT; i++) {
        pageButtons[i]->setFixedHeight(buttonHeight);
    }
    nextButton->setFixedHeight(buttonHeight);
}

void ViewDisplay::setEdited(bool y)
{
    portabase->setEdited(y);
}

void ViewDisplay::allowBooleanToggle(bool flag)
{
    booleanToggle = flag;
}

void ViewDisplay::updateRowsPerPage(int rpp)
{
    if (rpp < 1) {
        rowsPerPage->setValue(1);
    }
    currentPage = 1;
    firstPageButton = 1;
    updateTable();
    updateButtons();
    setEdited(TRUE);
}

void ViewDisplay::changePage(int id)
{
    int newPage = firstPageButton + id;
    if (newPage == currentPage) {
        return;
    }
    currentPage = newPage;
    updateTable();
}

void ViewDisplay::nextPages()
{
    firstPageButton += PAGE_BUTTON_COUNT;
    currentPage = firstPageButton;
    updateTable();
    updateButtons();
}

void ViewDisplay::previousPages()
{
    firstPageButton -= PAGE_BUTTON_COUNT;
    firstPageButton = QMAX(firstPageButton, 1);
    if (currentPage >= firstPageButton + PAGE_BUTTON_COUNT) {
        currentPage = firstPageButton;
    }
    updateTable();
    updateButtons();
}

void ViewDisplay::updateTable()
{
    int rowCount = view->getRowCount();
    int rows = rowCount;
    int index = 0;
    int rpp = rowsPerPage->value();
    if (paged) {
        index = (currentPage - 1) * rpp;
        rows = QMIN(rpp, rowCount - index);
    }
    if (rows <= 0 && rowCount > 0) {
        // past end of rows due to deletion or filtering, move to last page
        int pageCount = rowCount / rpp;
        if (rowCount > pageCount * rpp) {
            pageCount++;
        }
        currentPage = pageCount;
        index = (currentPage - 1) * rpp;
        rows = QMIN(rpp, rowCount - index);
        if (firstPageButton > currentPage) {
            firstPageButton -= PAGE_BUTTON_COUNT;
            firstPageButton = QMAX(firstPageButton, 1);
        }
    }
    table->clear();
    int *types = view->getColTypes();
    QListViewItem *item = 0;
    QRegExp linefeed("\n");
    for (int i = 0; i < rows; i++) {
        if (i == 0) {
            item = new ShadedListItem(0, table);
        }
        else {
            item = new ShadedListItem(i, table, item);
        }
        QStringList data = view->getRow(index);
        int count = data.count();
        for (int j = 0; j < count; j++) {
            int type = types[j];
            if (type == BOOLEAN) {
                int checked = data[j].toInt();
                item->setPixmap(j, PortaBase::getCheckBoxPixmap(checked));
            }
            else if (type == IMAGE) {
                if (data[j] == "") {
                    item->setText(j, "");
                }
                else {
                    item->setPixmap(j, Resource::loadPixmap("portabase/image"));
                }
            }
            else if (type == NOTE || type == STRING) {
                item->setText(j, data[j].replace(linefeed, " "));
            }
            else {
                item->setText(j, data[j]);
            }
        }
        index++;
    }
    if (rows > 0) {
        stack->raiseWidget(table);
    }
    else {
        stack->raiseWidget(noResults);
    }
    portabase->setRowSelected(FALSE);
}

void ViewDisplay::updateButtons()
{
    if (!paged) {
        return;
    }
    int rpp = rowsPerPage->value();
    double totalPages = ceil((double)(view->getRowCount()) / (double)rpp);
    prevButton->setEnabled(firstPageButton > 1);
    nextButton->setEnabled(totalPages >= firstPageButton + PAGE_BUTTON_COUNT);
    int page = firstPageButton;
    for (int i=0; i < PAGE_BUTTON_COUNT; i++) {
        pageButtons[i]->setOn(page == currentPage);
        pageButtons[i]->setEnabled(totalPages >= page);
        pageButtons[i]->setTextLabel(QString::number(page), FALSE);
        page++;
    }
}

void ViewDisplay::setDatabase(Database *dbase)
{
    closeView();
    db = dbase;
    currentPage = 1;
    firstPageButton = 1;
    setView(db->currentView());
    setEdited(FALSE);
}

void ViewDisplay::setView(QString name, bool applyDefaults)
{
    closeView();
    table->clear();
    int numCols = table->columns();
    int i;
    for (i = 0; i < numCols; i++) {
        table->removeColumn(0);
    }
    view = db->getView(name, applyDefaults);
    QStringList colNames = view->getColNames();
    int *types = view->getColTypes();
    int count = colNames.count();
    for (i = 0; i < count; i++) {
        if (types[i] == NOTE) {
            table->addColumn(PortaBase::getNotePixmap(), colNames[i],
                             view->getColWidth(i));
        }
        else {
            table->addColumn(colNames[i], view->getColWidth(i));
        }
        table->setColumnWidthMode(i, QListView::Manual);
    }
    rowsPerPage->setValue(view->getRowsPerPage());
    view->prepareData();
    updateTable();
    updateButtons();
}

void ViewDisplay::setSorting(QString name)
{
    view->sort(name);
    view->prepareData();
    updateTable();
}

void ViewDisplay::setFilter(QString name)
{
    db->getFilter(name);
    view->prepareData();
    updateTable();
    updateButtons();
}

void ViewDisplay::closeView()
{
    if (view) {
        saveViewSettings();
        view = 0;
    }
}

void ViewDisplay::saveViewSettings()
{
    view->saveColWidths();
    view->setRowsPerPage(rowsPerPage->value());
}

void ViewDisplay::addRow()
{
    RowEditor rowEditor(this);
    if (rowEditor.edit(db, -1)) {
        view->prepareData();
        updateTable();
        updateButtons();
        setEdited(TRUE);
    }
}

bool ViewDisplay::editRow(int id, bool copy)
{
    int rowId = id;
    if (rowId == -1) {
        rowId = selectedRowId();
    }
    if (rowId != -1) {
        RowEditor rowEditor(this);
        if (rowEditor.edit(db, rowId, copy)) {
            view->prepareData();
            updateTable();
            updateButtons();
            setEdited(TRUE);
            return TRUE;
        }
    }
    return FALSE;
}

void ViewDisplay::viewRow()
{
    int rowIndex = selectedRowIndex();
    if (rowIndex != -1) {
        RowViewer rowViewer(db, this);
        rowViewer.viewRow(view, rowIndex);
    }
}

void ViewDisplay::deleteRow()
{
    int rowId = selectedRowId();
    if (rowId != -1) {
        db->deleteRow(rowId);
        view->prepareData();
        updateTable();
        updateButtons();
        setEdited(TRUE);
    }
}

void ViewDisplay::deleteAllRows()
{
    view->deleteAllRows();
    view->prepareData();
    updateTable();
    updateButtons();
    setEdited(TRUE);
}

void ViewDisplay::exportToCSV(QString filename)
{
    view->exportToCSV(filename);
}

void ViewDisplay::exportToXML(QString filename)
{
    view->exportToXML(filename);
}

int ViewDisplay::selectedRowIndex()
{
    QListViewItem *selected = table->selectedItem();
    if (!selected) {
        return -1;
    }
    int index = 0;
    QListViewItem *item = table->firstChild();
    if (item != selected) {
        QListViewItem *next = item->nextSibling();
        while (next) {
            index++;
            if (next == selected) {
                break;
            }
            next = next->nextSibling();
        }
    }
    if (!paged) {
        return index;
    }
    int rpp = rowsPerPage->value();
    int startIndex = (currentPage - 1) * rpp;
    return startIndex + index;
}

int ViewDisplay::selectedRowId()
{
    int index = selectedRowIndex();
    if (index == -1) {
        return -1;
    }
    return view->getId(index);
}

void ViewDisplay::rowSelected()
{
    portabase->setRowSelected(TRUE);
}

void ViewDisplay::cellPressed(QListViewItem *item, const QPoint&, int column)
{
    if (item == 0) {
        // no row selected
        return;
    }
    pressedIndex = column;
    timer.restart();
}

void ViewDisplay::cellReleased(QListViewItem *item, const QPoint&, int column)
{
    if (item == 0) {
        // no row selected
        return;
    }
    if (column != pressedIndex) {
        return;
    }
    int *types = view->getColTypes();
    int type = types[column];
    int rowId = selectedRowId();
    QString colName = table->header()->label(column);
    if (type == BOOLEAN && booleanToggle) {
        db->toggleBoolean(rowId, colName);
        view->prepareData();
        updateTable();
        setEdited(TRUE);
    }
    else if (timer.elapsed() > 500) {
        if (type == NOTE) {
            NoteEditor viewer(colName, TRUE, this);
            viewer.setContent(view->getNote(rowId, column));
            viewer.exec();
        }
        else if (type == IMAGE) {
            QString format = view->getImageFormat(rowId, column);
            if (format != "") {
                QImage image = view->getImage(rowId, column);
                ImageViewer viewer(TRUE, this);
                viewer.setView(view, selectedRowIndex(), column);
                viewer.setImage(image);
                viewer.exec();
            }
        }
        else {
            editRow();
        }
    }
}

void ViewDisplay::headerPressed(int column)
{
    pressedIndex = column;
    timer.restart();
}

void ViewDisplay::headerReleased(int column)
{
    if (column != pressedIndex) {
        return;
    }
    if (timer.elapsed() > 500) {
        showStatistics(column);
    }
    else {
        sort(column);
    }
}

void ViewDisplay::columnResized(int column, int, int newWidth)
{
    view->setColWidth(column, newWidth);
    setEdited(TRUE);
}

void ViewDisplay::sort(int column)
{
    view->sort(column);
    view->prepareData();
    updateTable();
    portabase->updateSortMenu();
    setEdited(TRUE);
}

void ViewDisplay::showStatistics(int column)
{
    QStringList stats = view->getStatistics(column);
    QString content("<qt><center><b>");
    content += table->header()->label(column) + "</b></center>";
    int count = stats.count();
    for (int i = 0; i < count; i++) {
        content += stats[i] + "<br/>";
    }
    content += "</qt>";
    QMessageBox mb(tr("PortaBase"), content, QMessageBox::NoIcon,
                   QMessageBox::Ok, QMessageBox::NoButton,
                   QMessageBox::NoButton, this);
    mb.exec();
}

void ViewDisplay::slideshow()
{
    QStringList colNames = view->getColNames();
    int *types = view->getColTypes();
    int count = colNames.count();
    QStringList imageCols;
    for (int i = 0; i < count; i++) {
        if (types[i] == IMAGE) {
            imageCols.append(colNames[i]);
        }
    }
    if (imageCols.count() == 0) {
        QMessageBox::warning(this, tr("PortaBase"), tr("No image columns in this view"));
        return;
    }
    if (view->getRowCount() == 0) {
        QMessageBox::warning(this, tr("PortaBase"), tr("No rows in this view"));
        return;
    }
    SlideshowDialog dialog(imageCols, view, this);
    dialog.exec();
}

void ViewDisplay::keyReleaseEvent(QKeyEvent *e)
{
    int key = e->key();
    if (key == Qt::Key_Space) {
        viewRow();
    }
    else {
        e->ignore();
    }
}
