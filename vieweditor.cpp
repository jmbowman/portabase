/*
 * dbeditor.cpp
 *
 * (c) 2002-2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#if defined(DESKTOP)
#include "desktop/resource.h"
#endif

#include <qcombobox.h>
#include <qhbox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qvbox.h>
#include "database.h"
#include "shadedlistitem.h"
#include "vieweditor.h"

ViewEditor::ViewEditor(QWidget *parent, const char *name, WFlags f)
    : QDialog(parent, name, TRUE, f), db(0), resized(FALSE)
{
    setCaption(tr("View Editor") + " - " + tr("PortaBase"));
    vbox = new QVBox(this);
#if defined(Q_WS_WIN)
    setSizeGripEnabled(TRUE);
    new QLabel("<center><b>" + tr("View Editor") + "</b></center>", vbox);
#endif

    QHBox *hbox = new QHBox(vbox);
    new QLabel(tr("View Name"), hbox);
    nameBox = new QLineEdit(hbox);

    hbox = new QHBox(vbox);
    new QLabel(tr("Default Sorting"), hbox);
    sortingBox = new QComboBox(FALSE, hbox);
    hbox->setStretchFactor(sortingBox, 1);

    hbox = new QHBox(vbox);
    new QLabel(tr("Default Filter"), hbox);
    filterBox = new QComboBox(FALSE, hbox);
    hbox->setStretchFactor(filterBox, 1);

    table = new QListView(vbox);
    table->setAllColumnsShowFocus(TRUE);
    table->setSorting(-1);
    table->header()->setMovingEnabled(FALSE);
    table->addColumn(tr("Include"));
    table->setColumnWidthMode(0, QListView::Manual);
    table->setColumnAlignment(0, Qt::AlignHCenter);
    int colWidth = vbox->width() - table->columnWidth(0) - 5;
    table->addColumn(tr("Column Name"), colWidth);
    connect(table, SIGNAL(clicked(QListViewItem*, const QPoint&, int)),
            this, SLOT(tableClicked(QListViewItem*, const QPoint&, int)));

    hbox = new QHBox(vbox);
    QPushButton *upButton = new QPushButton(tr("Up"), hbox);
    connect(upButton, SIGNAL(clicked()), this, SLOT(moveUp()));
    QPushButton *downButton = new QPushButton(tr("Down"), hbox);
    connect(downButton, SIGNAL(clicked()), this, SLOT(moveDown()));

#if defined(DESKTOP)
    new QLabel(" ", vbox);
    hbox = new QHBox(vbox);
    new QWidget(hbox);
    QPushButton *okButton = new QPushButton(tr("OK"), hbox);
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    new QWidget(hbox);
    QPushButton *cancelButton = new QPushButton(tr("Cancel"), hbox);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    new QWidget(hbox);
    setMinimumWidth(parent->width() / 2);
    setMinimumHeight(parent->height());
    setIcon(Resource::loadPixmap("portabase"));
#else
    showMaximized();
#endif
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
        item->setPixmap(0, db->getCheckBoxPixmap(isIncluded(name)));
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
        item->setPixmap(0, db->getCheckBoxPixmap(!included));
    }
}

bool ViewEditor::hasValidName()
{
    QString name = nameBox->text();
    if (name.isEmpty()) {
        QMessageBox::warning(this, tr("PortaBase"),
                             tr("No name entered"));
        return FALSE;
    }
    if (name == originalName) {
        // hasn't changed and isn't empty, must be valid
        return TRUE;
    }
    if (name[0] == '_') {
        QMessageBox::warning(this, tr("PortaBase"),
                             tr("Name must not start with '_'"));
        return FALSE;
    }
    // check for other views with same name
    bool result = TRUE;
    QStringList viewNames = db->listViews();
    int count = viewNames.count();
    for (int i = 0; i < count; i++) {
        if (name == viewNames[i]) {
            result = FALSE;
            break;
        }
    }
    if (!result) {
        QMessageBox::warning(this, tr("PortaBase"), tr("Duplicate name"));
    }
    return result;
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
    if (viewName != originalName) {
        db->renameView(originalName, viewName);
    }
    db->setViewDefaults(defaultSort, defaultFilter);
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
    vbox->resize(size());
    if (!resized) {
        int colWidth = vbox->width() - table->columnWidth(0) - 20;
        table->setColumnWidth(1, colWidth);
        resized = TRUE;
    }
}
