/*
 * portabase.cpp
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qpe/config.h>
#include <qpe/filemanager.h>
#include <qpe/fileselector.h>
#include <qpe/qpemenubar.h>
#include <qpe/qpetoolbar.h>
#include <qpe/resource.h>

#include <qaction.h>
#include <qapplication.h>
#include <qfont.h>
#include <qinputdialog.h>
#include <qmainwindow.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qwidgetstack.h>
#include "condition.h"
#include "conditioneditor.h"
#include "database.h"
#include "dbeditor.h"
#include "filter.h"
#include "filtereditor.h"
#include "importdialog.h"
#include "inputdialog.h"
#include "portabase.h"
#include "preferences.h"
#include "sorteditor.h"
#include "viewdisplay.h"
#include "vieweditor.h"

PortaBase::PortaBase(QWidget *parent, const char *name, WFlags f)
    : QMainWindow(parent, name, f), db(0), doc(0), isEdited(FALSE)
{
    doc = 0;
    setToolBarsMovable(FALSE);

    QFont currentFont = qApp->font();
    QString family = currentFont.family().lower();
    int size = currentFont.pointSize();
    Config conf("portabase");
    conf.setGroup("General");
    if (conf.readNumEntry("ConfirmDeletions", 0)) {
        confirmDeletions = TRUE;
    }
    else {
        confirmDeletions = FALSE;
    }
    conf.setGroup("Font");
    family = conf.readEntry("Name", family);
    size = conf.readNumEntry("Size", size);
    QFont font(family, size);
    qApp->setFont(font);
    setFont(font);

    QPEToolBar *bar = new QPEToolBar(this);
    bar->setHorizontalStretchable(TRUE);
    menu = bar;

    QPEMenuBar *mb = new QPEMenuBar(bar);
    file = new QPopupMenu(this);
    view = new QPopupMenu(this);
    view->setCheckable(TRUE);
    sort = new QPopupMenu(this);
    sort->setCheckable(TRUE);
    filter = new QPopupMenu(this);
    filter->setCheckable(TRUE);

    toolbar = new QPEToolBar(this);

    QIconSet saveIcons = Resource::loadIconSet("portabase/save");
    QPixmap disabledSave = Resource::loadPixmap("portabase/save_disabled");
    saveIcons.setPixmap(disabledSave, QIconSet::Small, QIconSet::Disabled);
    fileSaveAction = new QAction(tr("Save"), saveIcons,
                                 QString::null, 0, this, 0);
    connect(fileSaveAction, SIGNAL(activated()), this, SLOT(save()));
    fileSaveAction->addTo(toolbar);
    fileSaveAction->addTo(file);

    QAction *act = new QAction(tr("Import"), QString::null, 0, this, 0);
    connect(act, SIGNAL(activated()), this, SLOT(dataImport()));
    act->addTo(file);

    act = new QAction(tr("Export"), QString::null, 0, this, 0);
    connect(act, SIGNAL(activated()), this, SLOT(dataExport()));
    act->addTo(file);

    QIconSet deleteIcons = Resource::loadIconSet("trash");
    act = new QAction(tr("Delete Rows In Filter"), deleteIcons,
                      QString::null, 0, this, 0);
    connect(act, SIGNAL(activated()), this, SLOT(deleteAllRows()));
    act->addTo(file);

    act = new QAction(tr("Edit Columns"), QString::null, 0, this, 0);
    connect(act, SIGNAL(activated()), this, SLOT(editColumns()));
    act->addTo(file);

    act = new QAction(tr("Preferences"), QString::null, 0, this, 0);
    connect(act, SIGNAL(activated()), this, SLOT(editPreferences()));
    act->addTo(file);

    QIconSet addIcons = Resource::loadIconSet("new");
    act = new QAction(tr("Add"), addIcons, QString::null, 0, this, 0);
    connect(act, SIGNAL(activated()), this, SLOT(addRow()));
    act->addTo(toolbar);

    QIconSet editIcons = Resource::loadIconSet("edit");
    rowEditAction = new QAction(tr("Edit"), editIcons, QString::null, 0,
                                this, 0);
    connect(rowEditAction, SIGNAL(activated()), this, SLOT(editRow()));
    rowEditAction->addTo(toolbar);

    rowDeleteAction = new QAction(tr("Delete"), deleteIcons, QString::null,
                                  0, this, 0);
    connect(rowDeleteAction, SIGNAL(activated()), this, SLOT(deleteRow()));
    rowDeleteAction->addTo(toolbar);

    view->insertItem(addIcons, tr("Add"), this, SLOT(addView()));
    editViewId = view->insertItem(editIcons, tr("Edit"),
                                  this, SLOT(editView()));
    deleteViewId = view->insertItem(deleteIcons, tr("Delete"),
                                    this, SLOT(deleteView()));
    view->insertSeparator();
    allColumnsViewId = view->insertItem(tr("All Columns"), this,
                                        SLOT(viewAllColumns()));
    connect(view, SIGNAL(activated(int)), this, SLOT(changeView(int)));

    sort->insertItem(addIcons, tr("Add"), this, SLOT(addSorting()));
    editSortId = sort->insertItem(editIcons, tr("Edit"),
                                  this, SLOT(editSorting()));
    deleteSortId = sort->insertItem(deleteIcons, tr("Delete"),
                                    this, SLOT(deleteSorting()));
    sort->insertSeparator();
    connect(sort, SIGNAL(activated(int)), this, SLOT(changeSorting(int)));

    QIconSet findIcons = Resource::loadIconSet("find");
    act = new QAction(tr("Quick"), findIcons, QString::null, 0, this, 0);
    connect(act, SIGNAL(activated()), this, SLOT(simpleFilter()));
    act->addTo(filter);

    filter->insertItem(addIcons, tr("Add"), this, SLOT(addFilter()));
    editFilterId = filter->insertItem(editIcons, tr("Edit"),
                                      this, SLOT(editFilter()));
    deleteFilterId = filter->insertItem(deleteIcons, tr("Delete"),
                                        this, SLOT(deleteFilter()));
    filter->insertSeparator();
    allRowsFilterId = filter->insertItem(tr("All Rows"), this,
                                         SLOT(viewAllRows()));
    connect(filter, SIGNAL(activated(int)), this, SLOT(changeFilter(int)));

    mb->insertItem(tr("File"), file);
    mb->insertItem(tr("View"), view);
    mb->insertItem(tr("Sort"), sort);
    mb->insertItem(tr("Filter"), filter);
                                
    mainStack = new QWidgetStack(this);
    setCentralWidget(mainStack);

    viewer = new ViewDisplay(this, mainStack);

    fileSelector = new FileSelector("application/portabase", mainStack,
                                    "fileselector", TRUE, FALSE);
    connect(fileSelector, SIGNAL(newSelected(const DocLnk &)), this,
            SLOT(newFile(const DocLnk &)));
    connect(fileSelector, SIGNAL(fileSelected(const DocLnk &)), this,
            SLOT(openFile(const DocLnk &)));
    fileOpen();

    resize(200, 300);
}

PortaBase::~PortaBase()
{

}

bool PortaBase::editColumns()
{
    DBEditor editor(this);
    int accepted = editor.edit(db);
    if (accepted) {
        viewer->closeView();
        editor.applyChanges();
        QStringList views = db->listViews();
        if (views.count() == 0) {
            db->addView("_all", db->listColumns());
            viewer->setDatabase(db);
            rebuildViewMenu();
            rebuildSortMenu();
            rebuildFilterMenu();
        }
        else {
            db->setViewColumnSequence("_all", db->listColumns());
            viewAllColumns();
        }
        setEdited(TRUE);
        mainStack->raiseWidget(viewer);
        menu->show();
        toolbar->show();
        return TRUE;
    }
    else {
        return FALSE;
    }
}

void PortaBase::editPreferences()
{
    Preferences prefs;
    if (prefs.exec()) {
        QFont font = prefs.applyChanges();
        setFont(font);
        file->setFont(font);
        view->setFont(font);
        sort->setFont(font);
        filter->setFont(font);
        Config conf("portabase");
        conf.setGroup("General");
        if (conf.readNumEntry("ConfirmDeletions", 0)) {
            confirmDeletions = TRUE;
        }
        else {
            confirmDeletions = FALSE;
        }
    }
}

void PortaBase::newFile(const DocLnk &f)
{
    bool ok = FALSE;
    QString name = InputDialog::getText(tr("New"),
                                        tr("Please select a file name"),
                                        QString::null, &ok, this);
    if (ok && !name.isEmpty()) {
        doc = new DocLnk(f);
        doc->setType("application/portabase");
        if (name.length() > 40) {
            name = name.left(40);
        }
        doc->setName(name);
    }
    else {
        ok = FALSE;
    }
    if (ok) {
        FileManager fm;
        fm.saveFile(*doc, "");
        db = new Database(doc->file(), &ok);
        if (editColumns()) {
            updateCaption();
            // if not saved now, file is empty without later save...bad
            save();
        }
        else {
            doc->removeFiles();
        }
    }
    else {
        QMessageBox::warning(this, tr("PortaBase"),
                             tr("Unable to create new file"));
    }
}

void PortaBase::openFile(const QString &f)
{
    DocLnk nf(f);
    nf.setType("application/portabase");
    openFile(nf);
}

void PortaBase::openFile(const DocLnk &f)
{
    bool ok = FALSE;
    Database *temp = new Database(f.file(), &ok);
    if (!ok) {
        QMessageBox::warning(this, tr("PortaBase"), tr("This file uses a newer version of the\nPortaBase format than this version\nof PortaBase supports; please\nupgrade"));
        return;
    }
    if (doc) {
        delete doc;
    }
    doc = new DocLnk(f);
    if (db) {
        delete db;
    }
    db = temp;
    viewer->setDatabase(db);
    mainStack->raiseWidget(viewer);
    menu->show();
    toolbar->show();
    updateCaption();
    rebuildViewMenu();
    rebuildSortMenu();
    rebuildFilterMenu();
}

void PortaBase::fileOpen()
{
    menu->hide();
    toolbar->hide();
    mainStack->raiseWidget(fileSelector);
    fileSelector->reread();
    updateCaption();
}

void PortaBase::updateCaption(const QString &name)
{
    if (!doc) {
        setCaption(tr("PortaBase"));
    }
    else {
        QString s = name;
        if (s.isNull()) {
            s = doc->name();
        }
        setCaption(s + " - " + tr("PortaBase"));
    }
}

void PortaBase::closeViewer()
{
    viewer->closeView();
    menu->hide();
    toolbar->hide();
    fileSelector->reread();
    mainStack->raiseWidget(fileSelector);
    delete doc;
    doc = 0;
    delete db;
    db = 0;
    updateCaption();
}

void PortaBase::setDocument(const QString &fileref)
{
    openFile(fileref);
}

void PortaBase::closeEvent(QCloseEvent *e)
{
    if (mainStack->visibleWidget() == viewer) {
        if (isEdited) {
            int choice = QMessageBox::warning(this, tr("PortaBase"),
                                              tr("Save changes?"),
                                              tr("Yes"), tr("No"));
            if (choice == 0) {
                save();
            }
        }
        e->ignore();
        closeViewer();
    }
    else {
        e->accept();
    }
}

void PortaBase::addRow()
{
    viewer->addRow();
}

void PortaBase::editRow()
{
    viewer->editRow();
}

void PortaBase::deleteRow()
{
    if (confirmDeletions) {
        if (QMessageBox::warning(this, tr("PortaBase"), tr("Delete this row?"),
                                 tr("Yes"), tr("No"), QString::null, 1) > 0) {
            return;
        }
    }
    viewer->deleteRow();
}

void PortaBase::deleteAllRows()
{
    if (confirmDeletions) {
        if (QMessageBox::warning(this, tr("PortaBase"),
                                 tr("Delete all rows in the\ncurrent filter?"),
                                 tr("Yes"), tr("No"), QString::null, 1) > 0) {
            return;
        }
    }
    viewer->deleteAllRows();
}

void PortaBase::save()
{
    viewer->saveViewSettings();
    db->commit();
    setEdited(FALSE);
}

void PortaBase::dataImport()
{
    ImportDialog dialog(db, this);
    if (dialog.exec()) {
        viewer->updateTable();
        viewer->updateButtons();
        setEdited(TRUE);
    }
}

void PortaBase::dataExport()
{
    bool ok = FALSE;
    QString name = InputDialog::getText(tr("Export"),
                                        tr("Please select a file name"),
                                        QString::null, &ok, this);
    if (ok && !name.isEmpty()) {
        DocLnk csv;
        csv.setType("text/x-csv");
        if (name.length() > 40) {
            name = name.left(40);
        }
        csv.setName(name);
        FileManager fm;
        fm.saveFile(csv, "");
        db->exportToCSV(csv.file());
    }
}

void PortaBase::viewAllColumns()
{
    viewer->setView("_all");
    updateViewMenu();
    setEdited(TRUE);
}

void PortaBase::viewAllRows()
{
    viewer->setFilter("_allrows");
    updateFilterMenu();
    setEdited(TRUE);
}

void PortaBase::changeView(int id)
{
    int index = viewIds.findIndex(id);
    if (index != -1) {
        viewer->setView(viewNames[index]);
        updateViewMenu();
        setEdited(TRUE);
    }
}

void PortaBase::changeSorting(int id)
{
    int index = sortIds.findIndex(id);
    if (index != -1) {
        viewer->setSorting(sortNames[index]);
        updateSortMenu();
        setEdited(TRUE);
    }
}

void PortaBase::changeFilter(int id)
{
    int index = filterIds.findIndex(id);
    if (index != -1) {
        viewer->setFilter(filterNames[index]);
        updateFilterMenu();
        setEdited(TRUE);
    }
}

void PortaBase::rebuildViewMenu()
{
    // remove old view names
    int count = viewNames.count();
    for (int i = 0; i < count; i++) {
        view->removeItem(viewIds[i]);
    }
    viewIds.clear();
    // add new view names
    viewNames = db->listViews();
    viewNames.remove("_all");
    count = viewNames.count();
    for (int i = 0; i < count; i++) {
        int id = view->insertItem(viewNames[i]);
        viewIds.append(id);
    }
    updateViewMenu();
}

void PortaBase::rebuildSortMenu()
{
    // remove old sorting names
    int count = sortNames.count();
    for (int i = 0; i < count; i++) {
        sort->removeItem(sortIds[i]);
    }
    sortIds.clear();
    // add new sorting names
    sortNames = db->listSortings();
    sortNames.remove("_single");
    count = sortNames.count();
    for (int i = 0; i < count; i++) {
        int id = sort->insertItem(sortNames[i]);
        sortIds.append(id);
    }
    updateSortMenu();
}

void PortaBase::rebuildFilterMenu()
{
    // remove old filter names
    int count = filterNames.count();
    for (int i = 0; i < count; i++) {
        filter->removeItem(filterIds[i]);
    }
    filterIds.clear();
    // add new filter names
    filterNames = db->listFilters();
    filterNames.remove("_allrows");
    filterNames.remove("_simple");
    count = filterNames.count();
    for (int i = 0; i < count; i++) {
        int id = filter->insertItem(filterNames[i]);
        filterIds.append(id);
    }
    updateFilterMenu();
}

void PortaBase::updateViewMenu()
{
    QString viewName = db->currentView();
    if (viewName == "_all") {
        view->setItemChecked(allColumnsViewId, TRUE);
        view->setItemEnabled(editViewId, FALSE);
        view->setItemEnabled(deleteViewId, FALSE);
    }
    else {
        view->setItemChecked(allColumnsViewId, FALSE);
        view->setItemEnabled(editViewId, TRUE);
        view->setItemEnabled(deleteViewId, TRUE);
    }
    int count = viewNames.count();
    for (int i = 0; i < count; i++) {
        if (viewName == viewNames[i]) {
            view->setItemChecked(viewIds[i], TRUE);
        }
        else {
            view->setItemChecked(viewIds[i], FALSE);
        }
    }
}

void PortaBase::updateSortMenu()
{
    QString sortName = db->currentSorting();
    if (sortName == "" || sortName == "_single") {
        sort->setItemEnabled(editSortId, FALSE);
        sort->setItemEnabled(deleteSortId, FALSE);
    }
    else {
        sort->setItemEnabled(editSortId, TRUE);
        sort->setItemEnabled(deleteSortId, TRUE);
    }
    int count = sortNames.count();
    for (int i = 0; i < count; i++) {
        if (sortName == sortNames[i]) {
            sort->setItemChecked(sortIds[i], TRUE);
        }
        else {
            sort->setItemChecked(sortIds[i], FALSE);
        }
    }
}

void PortaBase::updateFilterMenu()
{
    QString filterName = db->currentFilter();
    if (filterName == "_allrows") {
        filter->setItemChecked(allRowsFilterId, TRUE);
        filter->setItemEnabled(editFilterId, FALSE);
        filter->setItemEnabled(deleteFilterId, FALSE);
    }
    else if (filterName == "_simple") {
        filter->setItemChecked(allRowsFilterId, FALSE);
        filter->setItemEnabled(editFilterId, FALSE);
        filter->setItemEnabled(deleteFilterId, FALSE);
    }
    else {
        filter->setItemChecked(allRowsFilterId, FALSE);
        filter->setItemEnabled(editFilterId, TRUE);
        filter->setItemEnabled(deleteFilterId, TRUE);
    }
    int count = filterNames.count();
    for (int i = 0; i < count; i++) {
        if (filterName == filterNames[i]) {
            filter->setItemChecked(filterIds[i], TRUE);
        }
        else {
            filter->setItemChecked(filterIds[i], FALSE);
        }
    }
}

void PortaBase::editView()
{
    ViewEditor editor;
    QString viewName = db->currentView();
    if (editor.edit(db, viewName, db->listViewColumns(viewName))) {
        viewer->closeView();
        editor.applyChanges();
        QString newName = editor.getName();
        viewer->setView(newName);
        // view menu is unchanged unless the view's name changed
        if (viewName != newName) {
            rebuildViewMenu();
        }
        setEdited(TRUE);
    }
}

void PortaBase::editSorting()
{
    SortEditor editor;
    QString sortingName = db->currentSorting();
    if (editor.edit(db, sortingName)) {
        editor.applyChanges();
        QString newName = editor.getName();
        viewer->setSorting(newName);
        // sort menu is unchanged unless the sorting's name changed
        if (sortingName != newName) {
            rebuildSortMenu();
        }
        setEdited(TRUE);
    }
}

void PortaBase::editFilter()
{
    FilterEditor editor;
    QString filterName = db->currentFilter();
    if (editor.edit(db, filterName)) {
        editor.applyChanges();
        QString newName = editor.getName();
        viewer->setFilter(newName);
        // filter menu is unchanged unless the filter's name changed
        if (filterName != newName) {
            rebuildFilterMenu();
        }
        setEdited(TRUE);
    }
}

void PortaBase::addView()
{
    ViewEditor editor;
    QStringList empty;
    if (editor.edit(db, "", empty)) {
        editor.applyChanges();
        viewer->setView(editor.getName());
        rebuildViewMenu();
        setEdited(TRUE);
    }
}

void PortaBase::addSorting()
{
    SortEditor editor;
    if (editor.edit(db, "")) {
        editor.applyChanges();
        viewer->setSorting(editor.getName());
        rebuildSortMenu();
        setEdited(TRUE);
    }
}

void PortaBase::addFilter()
{
    FilterEditor editor;
    if (editor.edit(db, "")) {
        editor.applyChanges();
        viewer->setFilter(editor.getName());
        rebuildFilterMenu();
        setEdited(TRUE);
    }
}

void PortaBase::deleteView()
{
    if (confirmDeletions) {
        if (QMessageBox::warning(this, tr("PortaBase"),
                                 tr("Delete this view?"),
                                 tr("Yes"), tr("No"), QString::null, 1) > 0) {
            return;
        }
    }
    viewer->closeView();
    db->deleteView(db->currentView());
    viewer->setView("_all");
    rebuildViewMenu();
    setEdited(TRUE);
}

void PortaBase::deleteSorting()
{
    if (confirmDeletions) {
        if (QMessageBox::warning(this, tr("PortaBase"),
                                 tr("Delete this sorting?"),
                                 tr("Yes"), tr("No"), QString::null, 1) > 0) {
            return;
        }
    }
    db->deleteSorting(db->currentSorting());
    viewer->setSorting("");
    rebuildSortMenu();
    setEdited(TRUE);
}

void PortaBase::deleteFilter()
{
    if (confirmDeletions) {
        if (QMessageBox::warning(this, tr("PortaBase"),
                                 tr("Delete this filter?"),
                                 tr("Yes"), tr("No"), QString::null, 1) > 0) {
            return;
        }
    }
    db->deleteFilter(db->currentFilter());
    viewer->setFilter("_allrows");
    rebuildFilterMenu();
    setEdited(TRUE);
}

void PortaBase::simpleFilter()
{
    ConditionEditor editor(db, this);
    Condition *condition = db->getCondition("_simple", 0);
    if (editor.edit(condition)) {
        editor.applyChanges(condition);
        db->deleteFilter("_simple");
        Filter *filter = new Filter(db, "_simple");
        filter->addCondition(condition);
        db->addFilter(filter);
        delete filter;
        viewer->setFilter("_simple");
        updateFilterMenu();
        setEdited(TRUE);
    }
    else {
        delete condition;
    }
}

void PortaBase::setEdited(bool y)
{
    isEdited = y;
    fileSaveAction->setEnabled(y);
}

void PortaBase::setRowSelected(bool y)
{
    rowEditAction->setEnabled(y);
    rowDeleteAction->setEnabled(y);
}

QPixmap PortaBase::getNotePixmap()
{
    return Resource::loadPixmap("txt");
}
