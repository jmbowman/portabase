/*
 * sorteditor.cpp
 *
 * (c) 2002-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qhbox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qmessagebox.h>
#include "database.h"
#include "portabase.h"
#include "shadedlistitem.h"
#include "sorteditor.h"

SortEditor::SortEditor(QWidget *parent, const char *name)
    : PBDialog(tr("Sorting Editor"), parent, name), db(0), resized(FALSE)
{
    QHBox *hbox = new QHBox(this);
    vbox->addWidget(hbox);
    new QLabel(tr("Sorting Name") + " ", hbox);
    nameBox = new QLineEdit(hbox);

    table = new QListView(this);
    vbox->addWidget(table);
    table->setAllColumnsShowFocus(TRUE);
    table->setSorting(-1);
    table->header()->setMovingEnabled(FALSE);
    table->addColumn(tr("Sort"));
    table->setColumnWidthMode(0, QListView::Manual);
    table->setColumnAlignment(0, Qt::AlignHCenter);
    int colWidth = -1;
#if !defined(Q_WS_WIN)
    colWidth = (width() - table->columnWidth(0) - 5) / 2;
#endif
    table->addColumn(tr("Column Name"), colWidth);
    table->addColumn(tr("Direction"), colWidth);
    connect(table, SIGNAL(clicked(QListViewItem*, const QPoint&, int)),
            this, SLOT(tableClicked(QListViewItem*, const QPoint&, int)));

    addEditButtons(TRUE);
    connect(upButton, SIGNAL(clicked()), this, SLOT(moveUp()));
    connect(downButton, SIGNAL(clicked()), this, SLOT(moveDown()));

    finishLayout();
    nameBox->setFocus();
}

SortEditor::~SortEditor()
{

}

int SortEditor::edit(Database *subject, QString sortingName)
{
    db = subject;
    originalName = sortingName;
    nameBox->setText(sortingName);
    colNames = db->listColumns();
    db->getSortingInfo(sortingName, &sortCols, &descCols);
    // move currently sorted columns to the top of the list, in correct order
    int count = sortCols.count();
    for (int i = count - 1; i > -1; i--) {
        QString name = sortCols[i];
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

QString SortEditor::getName()
{
    return nameBox->text();
}

void SortEditor::moveUp()
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

void SortEditor::moveDown()
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

void SortEditor::selectRow(QString name)
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

void SortEditor::updateTable()
{
    table->clear();
    int count = colNames.count();
    QListViewItem *item = 0;
    for (int i = 0; i < count; i++) {
        QString name = colNames[i];
        bool sorted = isSorted(name);
        QString direction = "";
        if (sorted) {
            if (descCols.findIndex(name) == -1) {
                direction = tr("Ascending");
            }
            else {
                direction = tr("Descending");
            }
        }
        if (i == 0) {
            item = new ShadedListItem(0, table, "", name, direction);
        }
        else {
            item = new ShadedListItem(i, table, item, "", name, direction);
        }
        item->setPixmap(0, PortaBase::getCheckBoxPixmap(sorted));
    }
}

void SortEditor::tableClicked(QListViewItem *item, const QPoint&, int column)
{
    if (item == 0) {
        // no row selected
        return;
    }
    if (column == 0) {
        QString name = item->text(1);
        int sorted = isSorted(name);
        if (sorted) {
            sortCols.remove(name);
            descCols.remove(name);
            item->setText(2, "");
        }
        else {
            sortCols.append(name);
            item->setText(2, tr("Ascending"));
        }
        item->setPixmap(0, PortaBase::getCheckBoxPixmap(!sorted));
    }
    else if (column == 2) {
        QString name = item->text(1);
        int sorted = isSorted(name);
        if (sorted) {
            if (descCols.findIndex(name) == -1) {
                descCols.append(name);
                item->setText(2, tr("Descending"));
            }
            else {
                descCols.remove(name);
                item->setText(2, tr("Ascending"));
            }
        }
    }
}

bool SortEditor::hasValidName()
{
    return validateName(nameBox->text(), originalName, db->listSortings());
}

int SortEditor::isSorted(QString name)
{
    return (sortCols.findIndex(name) != -1);
}

void SortEditor::applyChanges()
{
    QString sortingName = nameBox->text();
    QStringList orderedSort;
    QStringList orderedDesc;
    int count = colNames.count();
    for (int i = 0; i < count; i++) {
        QString name = colNames[i];
        if (isSorted(name)) {
            orderedSort.append(name);
            if (descCols.findIndex(name) != -1) {
                orderedDesc.append(name);
            }
        }
    }
    db->deleteSorting(originalName);
    db->addSorting(sortingName, orderedSort, orderedDesc);
}

void SortEditor::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    if (!resized) {
        int colWidth = width() - table->columnWidth(0) - 20;
        table->setColumnWidth(1, colWidth / 2);
        table->setColumnWidth(2, colWidth / 2);
        resized = TRUE;
    }
}
