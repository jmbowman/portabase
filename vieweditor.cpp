/*
 * dbeditor.cpp
 *
 * (c) 2002-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qcombobox.h>
#include <qhbox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qmessagebox.h>
#include "database.h"
#include "portabase.h"
#include "shadedlistitem.h"
#include "vieweditor.h"

ViewEditor::ViewEditor(QWidget *parent, const char *name)
    : PBDialog(tr("View Editor"), parent, name), db(0), resized(FALSE)
{
    QHBox *hbox = new QHBox(this);
    vbox->addWidget(hbox);
    new QLabel(tr("View Name") + " ", hbox);
    nameBox = new QLineEdit(hbox);

    hbox = new QHBox(this);
    vbox->addWidget(hbox);
    new QLabel(tr("Default Sorting"), hbox);
    sortingBox = new QComboBox(FALSE, hbox);
    hbox->setStretchFactor(sortingBox, 1);

    hbox = new QHBox(this);
    vbox->addWidget(hbox);
    new QLabel(tr("Default Filter"), hbox);
    filterBox = new QComboBox(FALSE, hbox);
    hbox->setStretchFactor(filterBox, 1);

    table = new QListView(this);
    vbox->addWidget(table);
    table->setAllColumnsShowFocus(TRUE);
    table->setSorting(-1);
    table->header()->setMovingEnabled(FALSE);
    table->addColumn(tr("Include"));
    table->setColumnWidthMode(0, QListView::Manual);
    table->setColumnAlignment(0, Qt::AlignHCenter);
    int colWidth = -1;
#if !defined(Q_WS_WIN)
    colWidth = width() - table->columnWidth(0) - 5;
#endif
    table->addColumn(tr("Column Name"), colWidth);
    connect(table, SIGNAL(clicked(QListViewItem*, const QPoint&, int)),
            this, SLOT(tableClicked(QListViewItem*, const QPoint&, int)));

    addEditButtons(TRUE);
    connect(upButton, SIGNAL(clicked()), this, SLOT(moveUp()));
    connect(downButton, SIGNAL(clicked()), this, SLOT(moveDown()));

    finishLayout();
    nameBox->setFocus();
}

ViewEditor::~ViewEditor()
{

}

int ViewEditor::edit(Database *subject, const QString &viewName,
                     QStringList currentCols, const QString &defaultSort,
                     const QString &defaultFilter)
{
    db = subject;
    originalName = viewName;
    nameBox->setText(viewName);

    sortingBox->clear();
    sortingBox->insertItem(tr("None"));
    QStringList sortings = db->listSortings();
    sortings.remove("_single");
    sortingBox->insertStringList(sortings);
    int count = sortingBox->count();
    int i;
    if (defaultSort != "_none") {
        for (i = 1; i < count; i++) {
            if (sortingBox->text(i) == defaultSort) {
                sortingBox->setCurrentItem(i);
                break;
            }
        }
    }

    filterBox->clear();
    filterBox->insertItem(tr("None"));
    filterBox->insertItem(tr("All Rows"));
    QStringList filters = db->listFilters();
    filters.remove("_simple");
    filters.remove("_allrows");
    filterBox->insertStringList(filters);
    if (defaultFilter == "_allrows") {
        filterBox->setCurrentItem(1);
    }
    else if (defaultFilter != "_none") {
        count = filterBox->count();
        for (i = 2; i < count; i++) {
            if (filterBox->text(i) == defaultFilter) {
                filterBox->setCurrentItem(i);
                break;
            }
        }
    }

    colNames = db->listColumns();
    oldNames = currentCols;
    includedNames = currentCols;
    // move the current view columns to the top of the list, in correct order
    count = currentCols.count();
    for (i = count - 1; i > -1; i--) {
        QString name = currentCols[i];
        colNames.remove(name);
        colNames.prepend(name);
    }
    updateTable();
    int result = exec();
    while (result) {
        if (hasValidName()) {
            break;
        }
        else {
            result = exec();
        }
    }
    return result;
}

QString ViewEditor::getName()
{
    return nameBox->text();
}

void ViewEditor::moveUp()
{
    QListViewItem *item = table->selectedItem();
    if (item == 0) {
        return;
    }
    QListViewItem *above = item->itemAbove();
    if (above) {
        QString name = item->text(1);
        QString aboveName = above->text(1);
        colNames.remove(name);
        colNames.insert(colNames.find(aboveName), name);
        updateTable();
        selectRow(name);
    }
}

void ViewEditor::moveDown()
{
    QListViewItem *item = table->selectedItem();
    if (item == 0) {
        return;
    }
    QListViewItem *below = item->itemBelow();
    if (below) {
        QString name = item->text(1);
        colNames.remove(name);
        below = below->itemBelow();
        if (below) {
            QString belowName = below->text(1);
            colNames.insert(colNames.find(belowName), name);
        }
        else {
            colNames.append(name);
        }
        updateTable();
        selectRow(name);
    }
}

void ViewEditor::selectRow(QString name)
{
    QListViewItem *item = table->firstChild();
    if (item) {
        if (item->text(1) == name) {
            table->setSelected(item, TRUE);
        }
        else {
            QListViewItem *next = item->nextSibling();
	    while (next) {
                if (next->text(1) == name) {
                    table->setSelected(next, TRUE);
                    break;
                }
                next = next->nextSibling();
            }
        }
    }
}

void ViewEditor::updateTable()
{
    table->clear();
    int count = colNames.count();
    QListViewItem *item = 0;
    for (int i = 0; i < count; i++) {
        QString name = colNames[i];
        if (i == 0) {
            item = new ShadedListItem(0, table, "", name);
        }
        else {
            item = new ShadedListItem(i, table, item, "", name);
        }
        item->setPixmap(0, PortaBase::getCheckBoxPixmap(isIncluded(name)));
    }
}

void ViewEditor::tableClicked(QListViewItem *item, const QPoint&, int column)
{
    if (item == 0) {
        // no row selected
        return;
    }
    if (column == 0) {
        QString name = item->text(1);
        int included = isIncluded(name);
        if (included) {
            includedNames.remove(name);
        }
        else {
            includedNames.append(name);
        }
        item->setPixmap(0, PortaBase::getCheckBoxPixmap(!included));
    }
}

bool ViewEditor::hasValidName()
{
    return validateName(nameBox->text(), originalName, db->listViews());
}

int ViewEditor::isIncluded(QString name)
{
    return (includedNames.findIndex(name) != -1);
}

void ViewEditor::applyChanges()
{
    QString viewName = nameBox->text();
    QString defaultSort = sortingBox->currentText();
    if (sortingBox->currentItem() == 0) {
        defaultSort = "_none";
    }
    QString defaultFilter = filterBox->currentText();
    if (filterBox->currentItem() == 0) {
        defaultFilter = "_none";
    }
    else if (filterBox->currentItem() == 1) {
        defaultFilter = "_allrows";
    }
    int count = colNames.count();
    if (originalName == "") {
        // new view, just add it and return
        QStringList sequence;
        for (int i = 0; i < count; i++) {
            QString name = colNames[i];
            if (isIncluded(name)) {
                sequence.append(name);
            }
        }
        db->addView(viewName, sequence, defaultSort, defaultFilter);
        return;
    }
    db->setViewDefaults(defaultSort, defaultFilter);
    if (viewName != originalName) {
        db->renameView(originalName, viewName);
    }
    QStringList sequence;
    for (int i = 0; i < count; i++) {
        QString name = colNames[i];
        if (isIncluded(name)) {
            sequence.append(name);
            if (oldNames.findIndex(name) == -1) {
                // newly added column
                db->addViewColumn(viewName, name);
            }
        }
        else {
            if (oldNames.findIndex(name) != -1) {
                // removed column
                db->deleteViewColumn(viewName, name);
            }
        }
    }
    db->setViewColumnSequence(viewName, sequence);
}

void ViewEditor::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    if (!resized) {
        int colWidth = width() - table->columnWidth(0) - 20;
        table->setColumnWidth(1, colWidth);
        resized = TRUE;
    }
}
