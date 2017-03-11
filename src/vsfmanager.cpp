/*
 * vsfmanager.cpp
 *
 * (c) 2010,2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file vsfmanager.cpp
 * Source file for VSFManager
 */

#include <QAction>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include "database.h"
#include "factory.h"
#include "menuactions.h"
#include "portabase.h"
#include "vsfmanager.h"

/**
 * Constructor.
 *
 * @param parent The application's main window
 */
VSFManager::VSFManager(PortaBase *parent)
    : PBDialog("", parent), portabase(parent), editButton(0), deleteButton(0),
      addButton(0), nameFilter("^[^_].*")
{
    QScrollArea *sa = new QScrollArea(this);
    Factory::configureScrollArea(sa);
    vbox->addWidget(sa);
    QWidget *content = new QWidget();
    sa->setWidgetResizable(true);
    layout = Factory::vBoxLayout(content, true);

    QHBoxLayout *hbox = Factory::hBoxLayout(layout);
    currentLabel = new QLabel("", content);
    hbox->addWidget(currentLabel, 1);

#if defined(Q_OS_ANDROID)
    currentLabel->setMargin(Factory::dpToPixels(6));

    QAction *addButtonAction = new QAction(Factory::icon("add"), "", this);
    connect(addButtonAction, SIGNAL(triggered()), this, SLOT(addItem()));
    PBDialog::addButton(addButtonAction, 0);

    editButtonAction = new QAction(Factory::icon("edit"), "", this);
    connect(editButtonAction, SIGNAL(triggered()), this, SLOT(editItem()));
    PBDialog::addButton(editButtonAction, 0);

    deleteButtonAction = new QAction(Factory::icon("delete"), "", this);
    connect(deleteButtonAction, SIGNAL(triggered()), this, SLOT(deleteItem()));
    PBDialog::addButton(deleteButtonAction, 0);
#else
    editButton = Factory::button(content);
    editButton->setIcon(Factory::icon("edit"));
    connect(editButton, SIGNAL(clicked()), this, SLOT(editItem()));
    hbox->addWidget(editButton);

    deleteButton = Factory::button(content);
    deleteButton->setIcon(Factory::icon("delete"));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteItem()));
    hbox->addWidget(deleteButton);

    addButton = new QPushButton(content);
    addButton->setIcon(Factory::icon("add"));
    connect(addButton, SIGNAL(clicked()), this, SLOT(addItem()));
    layout->addWidget(addButton);
#endif

    QWidget *padding = new QWidget(content);
    layout->addWidget(padding);
#if defined(Q_OS_ANDROID)
    layout->addStretch(1);
#else
    layout->setStretchFactor(padding, 1);
#endif
    sa->setWidget(content);

    finishLayout(false, true, 400, 400);
}

/**
 * Specify the data source and type of item to be managed by this dialog.
 *
 * @param database The database being edited
 * @param s View, Sorting, or Filter
 */
void VSFManager::setSubject(Database *database, Subject s)
{
    db = database;
    subject = s;
    QString label("%1: %2");
    QString current;
    if (subject == View) {
        setWindowTitle(tr("Views"));
        current = db->currentView();
        label = label.arg(tr("Current View"));
        if (addButton) {
            addButton->setText(tr("New View"));
        }
        updateButtonList(db->listViews(), current);
    }
    else if (subject == Sorting) {
        setWindowTitle(tr("Sortings"));
        current = db->currentSorting();
        label = label.arg(tr("Current Sorting"));
        if (addButton) {
            addButton->setText(tr("New Sorting"));
        }
        updateButtonList(db->listSortings(), current);
    }
    else if (subject == Filter) {
        setWindowTitle(tr("Filters"));
        current = db->currentFilter();
        label = label.arg(tr("Current Filter"));
        if (addButton) {
            addButton->setText(tr("New Filter"));
        }
        updateButtonList(db->listFilters(), current);
    }
    bool specialCase;
    QString name = displayedName(current, &specialCase);
    currentLabel->setText(label.arg(name));
#if defined(Q_OS_ANDROID)
    editButtonAction->setEnabled(!specialCase);
    deleteButtonAction->setEnabled(!specialCase);
#else
    editButton->setEnabled(!specialCase);
    deleteButton->setEnabled(!specialCase);
#endif
}

/**
 * Get the name shown to the user for the given item name as stored in the
 * database.  Usually the same, except for the special built-in items.
 *
 * @param name The name as returned by the database
 * @param specialCase A bool to set to true if the item is one of the special
 *                    internal cases, false otherwise
 * @return The name as it is to be shown to the user
 */
QString VSFManager::displayedName(const QString &name, bool *specialCase)
{
    *specialCase = true;
    if (name == "_all") {
        return MenuActions::tr("All &Columns").replace("&", "");
    }
    else if (name == "_single") {
        return tr("Selected Column");
    }
    else if (name == "") {
        return tr("None");
    }
    else if (name == "_allrows") {
        return MenuActions::tr("All &Rows").replace("&", "");
    }
    else if (name == "_simple") {
        return MenuActions::tr("&Quick Filter").replace("&", "");
    }
    else {
        *specialCase = false;
        return name;
    }
}

/**
 * Specify the add, edit, and delete actions for the type of item currently
 * being managed by this dialog.
 *
 * @param add The add action for views, sortings, or filters
 * @param edit The edit action for views, sortings, or filters
 * @param del The delete action for views, sortings, or filters
 */
void VSFManager::setActions(QAction *add, QAction *edit, QAction *del)
{
    addAction = add;
    editAction = edit;
    deleteAction = del;
#if !defined(Q_OS_ANDROID)
    addButton->setToolTip(add->toolTip());
    editButton->setToolTip(edit->toolTip());
    deleteButton->setToolTip(del->toolTip());
#endif
}

/**
 * Update the list of buttons for selecting existing views, sortings, or
 * filters.
 *
 * @param items Names of the new items to be represented by the buttons
 * @param current The name of the currently selected item
 */
void VSFManager::updateButtonList(const QStringList &items, const QString &current)
{
    QStringList filteredItems = items.filter(nameFilter);
    filteredItems.removeAll(current);
    filteredItems.removeAll("");
    if (subject == View && current != "_all") {
        filteredItems.prepend("_all");
    }
    else if (subject == Filter && current != "_allrows") {
        filteredItems.prepend("_allrows");
    }
    int oldCount = buttonList.count();
    int newCount = filteredItems.count();
    int missingCount = newCount - oldCount;
    int extraCount = oldCount - newCount;
    int i;
    QPushButton *button;
    for (i = 0; i < missingCount; i++) {
        button = new QPushButton(0);
        connect(button, SIGNAL(clicked()), this, SLOT(selectItem()));
        layout->insertWidget(oldCount + i + 2, button);
        buttonList << button;
    }
    for (i = 0; i < extraCount; i++) {
        button = buttonList.last();
        buttonList.removeLast();
        delete button;
    }
    bool specialCase;
    for (i = 0; i < newCount; i++) {
        QString internalName = filteredItems[i];
        QString visibleName = displayedName(internalName, &specialCase);
        buttonList[i]->setText(visibleName);
        // abusing an unused property to store this
        buttonList[i]->setWindowTitle(internalName);
    }
}

/**
 * Handler for the "New..." button.
 */
void VSFManager::addItem()
{
    accept();
    addAction->trigger();
}

/**
 * Handler for the edit button.
 */
void VSFManager::editItem()
{
    accept();
    editAction->trigger();
}

/**
 * Handler for the delete button.
 */
void VSFManager::deleteItem()
{
    accept();
    deleteAction->trigger();
}

/**
 * Handler for existing item button clicks.
 */
void VSFManager::selectItem()
{
    QPushButton *source = qobject_cast<QPushButton *>(sender());
    QString name = source->windowTitle();
    accept();
    if (subject == View) {
        portabase->changeView(name);
    }
    else if (subject == Sorting) {
        portabase->changeSorting(name);
    }
    else if (subject == Filter) {
        portabase->changeFilter(name);
    }
}
