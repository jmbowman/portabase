/*
 * portabase.cpp
 *
 * (c) 2002-2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#if defined(DESKTOP)
#include <qfiledialog.h>
#include "desktop/config.h"
#include "desktop/filemanager.h"
#include "desktop/fileselector.h"
#include "desktop/helpbrowser.h"
#include "desktop/qpeapplication.h"
#include "desktop/qpemenubar.h"
#include "desktop/resource.h"
#include "desktop/importdialog.h"
#else
#include <qpe/config.h>
#include <qpe/filemanager.h>
#include <qpe/fileselector.h>
#include <qpe/qpemenubar.h>
#include <qpe/resource.h>
#include "importdialog.h"
#include "inputdialog.h"
#endif

#include <qaction.h>
#include <qapplication.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qfont.h>
#include <qmainwindow.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qwidgetstack.h>
#include "condition.h"
#include "conditioneditor.h"
#include "database.h"
#include "dbeditor.h"
#include "enummanager.h"
#include "filter.h"
#include "filtereditor.h"
#include "portabase.h"
#include "preferences.h"
#include "sorteditor.h"
#include "viewdisplay.h"
#include "vieweditor.h"

PortaBase::PortaBase(QWidget *parent, const char *name, WFlags f)
    : QMainWindow(parent, name, f), db(0), doc(0), isEdited(FALSE)
{
    setToolBarsMovable(FALSE);

    QFont currentFont = qApp->font();
    QString family = currentFont.family().lower();
    int size = currentFont.pointSize();
    Config conf("portabase");
    conf.setGroup("General");
    confirmDeletions = conf.readBoolEntry("ConfirmDeletions");
    booleanToggle = conf.readBoolEntry("BooleanToggle");
    conf.setGroup("Font");
    family = conf.readEntry("Name", family);
    size = conf.readNumEntry("Size", size);
    QFont font(family, size);
    qApp->setFont(font);
    setFont(font);

    // menu and toolbar, shared between file selector and data viewer modes
    QToolBar *bar = new QToolBar(this);
    addToolBar(bar, QMainWindow::Top);
    bar->setHorizontalStretchable(TRUE);
    menu = new QPEMenuBar(bar);
    toolbar = new QToolBar(this);
#if defined(DESKTOP)
    addToolBar(toolbar, QMainWindow::Top, TRUE);
#else
    addToolBar(toolbar, QMainWindow::Top, FALSE);
#endif

    // menu and toolbar icon sets
    QIconSet addIcons = Resource::loadIconSet("new");
    QIconSet openIcons = Resource::loadIconSet("fileopen");
    QIconSet deleteIcons = Resource::loadIconSet("trash");
    QIconSet quitIcons = Resource::loadIconSet("quit_icon");
    QIconSet saveIcons = Resource::loadIconSet("portabase/save");
#ifndef DESKTOP
    QPixmap disabledSave = Resource::loadPixmap("portabase/save_disabled");
    saveIcons.setPixmap(disabledSave, QIconSet::Small, QIconSet::Disabled);
#endif
    QIconSet closeIcons = Resource::loadIconSet("close");
    QIconSet editIcons = Resource::loadIconSet("edit");
    QIconSet findIcons = Resource::loadIconSet("find");

    // file selector actions
    fileNewAction = new QAction(tr("New"), addIcons, QString::null, 0, this);
    connect(fileNewAction, SIGNAL(activated()), this, SLOT(newFile()));
    fileOpenAction = new QAction(tr("Open"), openIcons, QString::null, 0,
                                 this);
    connect(fileOpenAction, SIGNAL(activated()), this, SLOT(openFile()));
    fileDeleteAction = new QAction(tr("Delete") + "...", deleteIcons,
                                   QString::null, 0, this);
    connect(fileDeleteAction, SIGNAL(activated()), this, SLOT(deleteFile()));
    importMobileDBAction = new QAction(tr("Import MobileDB File") + "...",
                                       QString::null, 0, this);
    connect(importMobileDBAction, SIGNAL(activated()),
            this, SLOT(importMobileDB()));
    quitAction = new QAction(tr("Quit"), quitIcons, QString::null, 0, this);
    connect(quitAction, SIGNAL(activated()), this, SLOT(close()));

    // File menu actions
    fileSaveAction = new QAction(tr("Save"), saveIcons,
                                 QString::null, 0, this);
    connect(fileSaveAction, SIGNAL(activated()), this, SLOT(save()));
    dataImportAction = new QAction(tr("Import"), QString::null, 0, this);
    connect(dataImportAction, SIGNAL(activated()), this, SLOT(dataImport()));
    dataExportAction = new QAction(tr("Export Rows In Filter"), QString::null,
                                   0, this);
    connect(dataExportAction, SIGNAL(activated()), this, SLOT(dataExport()));
    deleteRowsAction = new QAction(tr("Delete Rows In Filter"), deleteIcons,
                           QString::null, 0, this);
    connect(deleteRowsAction, SIGNAL(activated()),
            this, SLOT(deleteAllRows()));
    editColsAction = new QAction(tr("Edit Columns"), QString::null, 0, this);
    connect(editColsAction, SIGNAL(activated()), this, SLOT(editColumns()));
    manageEnumsAction = new QAction(tr("Edit Enums"), QString::null, 0, this);
    connect(manageEnumsAction, SIGNAL(activated()), this, SLOT(editEnums()));
    prefsAction = new QAction(tr("Preferences"), QString::null, 0, this);
    connect(prefsAction, SIGNAL(activated()), this, SLOT(editPreferences()));
    closeAction = new QAction(tr("Close"), closeIcons, QString::null, 0, this);
    connect(closeAction, SIGNAL(activated()), this, SLOT(close()));

    // Toolbar actions
    rowAddAction = new QAction(tr("Add"), addIcons, QString::null, 0, this);
    connect(rowAddAction, SIGNAL(activated()), this, SLOT(addRow()));
    rowEditAction = new QAction(tr("Edit"), editIcons, QString::null, 0,
                                this);
    connect(rowEditAction, SIGNAL(activated()), this, SLOT(editRow()));
    rowDeleteAction = new QAction(tr("Delete"), deleteIcons, QString::null,
                                  0, this);
    connect(rowDeleteAction, SIGNAL(activated()), this, SLOT(deleteRow()));

    // View menu actions
    viewAddAction = new QAction(tr("Add"), addIcons, QString::null, 0, this);
    connect(viewAddAction, SIGNAL(activated()), this, SLOT(addView()));
    viewEditAction = new QAction(tr("Edit"), editIcons, QString::null, 0,
                                 this);
    connect(viewEditAction, SIGNAL(activated()), this, SLOT(editView()));
    viewDeleteAction = new QAction(tr("Delete"), deleteIcons, QString::null, 0,
                                   this);
    connect(viewDeleteAction, SIGNAL(activated()), this, SLOT(deleteView()));
    viewAllColsAction = new QAction(tr("All Columns"), QString::null, 0, this,
                                    0, TRUE);
    connect(viewAllColsAction, SIGNAL(activated()),
            this, SLOT(viewAllColumns()));

    // Sort menu actions
    sortAddAction = new QAction(tr("Add"), addIcons, QString::null, 0, this);
    connect(sortAddAction, SIGNAL(activated()), this, SLOT(addSorting()));
    sortEditAction = new QAction(tr("Edit"), editIcons, QString::null, 0,
                                 this);
    connect(sortEditAction, SIGNAL(activated()), this, SLOT(editSorting()));
    sortDeleteAction = new QAction(tr("Delete"), deleteIcons, QString::null, 0,
                                   this);
    connect(sortDeleteAction, SIGNAL(activated()),
            this, SLOT(deleteSorting()));

    // Filter menu actions
    findAction = new QAction(tr("Quick"), findIcons, QString::null, 0, this);
    connect(findAction, SIGNAL(activated()), this, SLOT(simpleFilter()));
    filterAddAction = new QAction(tr("Add"), addIcons, QString::null, 0, this);
    connect(filterAddAction, SIGNAL(activated()), this, SLOT(addFilter()));
    filterEditAction = new QAction(tr("Edit"), editIcons, QString::null, 0,
                                 this);
    connect(filterEditAction, SIGNAL(activated()), this, SLOT(editFilter()));
    filterDeleteAction = new QAction(tr("Delete"), deleteIcons, QString::null,
                                     0, this);
    connect(filterDeleteAction, SIGNAL(activated()),
            this, SLOT(deleteFilter()));
    filterAllRowsAction = new QAction(tr("All Rows"), QString::null, 0, this,
                                      0, TRUE);
    connect(filterAllRowsAction, SIGNAL(activated()),
            this, SLOT(viewAllRows()));

    // Help menu actions
    helpAction = new QAction(tr("Help Contents"), QString::null, 0, this);
    connect(helpAction, SIGNAL(activated()), this, SLOT(showHelp()));
    aboutAction = new QAction(tr("About PortaBase"), QString::null, 0, this);
    connect(aboutAction, SIGNAL(activated()), this, SLOT(aboutPortaBase()));
    aboutQtAction = new QAction(tr("About Qt"), QString::null, 0, this);
    connect(aboutQtAction, SIGNAL(activated()), this, SLOT(aboutQt()));
                                
    mainStack = new QWidgetStack(this);
    setCentralWidget(mainStack);

    viewer = new ViewDisplay(this, mainStack);
    viewer->allowBooleanToggle(booleanToggle);

    fileSelector = new FileSelector("application/portabase", mainStack,
                                    "fileselector", FALSE, FALSE);
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
        showDataViewer();
        return TRUE;
    }
    else {
        return FALSE;
    }
}

void PortaBase::editEnums()
{
    EnumManager manager(db, this);
    viewer->closeView();
    if (manager.exec()) {
        manager.applyChanges();
    }
    viewer->setDatabase(db);
    setEdited(TRUE);
}

void PortaBase::editPreferences()
{
    Preferences prefs(this);
    if (prefs.exec()) {
        QFont font = prefs.applyChanges();
        setFont(font);
        viewer->updateButtonSizes();
        file->setFont(font);
        view->setFont(font);
        sort->setFont(font);
        filter->setFont(font);
#ifdef DESKTOP
        help->setFont(font);
#endif
        Config conf("portabase");
        conf.setGroup("General");
        confirmDeletions = conf.readBoolEntry("ConfirmDeletions");
        booleanToggle = conf.readBoolEntry("BooleanToggle");
        viewer->allowBooleanToggle(booleanToggle);
        db->setShowSeconds(conf.readBoolEntry("ShowSeconds"));
        db->updateDateTimePrefs();
        viewer->updateTable();
    }
}

void PortaBase::newFile()
{
    DocLnk f;
    createFile(f, NO_DATA);
}

void PortaBase::importMobileDB()
{
    DocLnk f;
    createFile(f, MOBILEDB_FILE);
}

void PortaBase::createFile(const DocLnk &f, int source)
{
    bool ok = TRUE;
#if defined(DESKTOP)
    QString file = QFileDialog::getSaveFileName(QPEApplication::documentDir(),
                       tr("PortaBase files") + " (*.pob)",
                       this,
                       "new file dialog",
                       tr("Choose a filename to save under"));
    if (!file.isEmpty()) {
        doc = new DocLnk(f);
        doc->setType("application/portabase");
        QFileInfo info(file);
        doc->setName(info.baseName());
        doc->setFile(info.dirPath(TRUE) + "/" + info.baseName() + ".pob");
        QPEApplication::setDocumentDir(info.dirPath(TRUE));
    }
#else
    QString name = InputDialog::getText(tr("PortaBase"),
                                        tr("Enter a name for the new file"),
                                        QString::null, &ok, this);
    if (ok && !name.isEmpty()) {
        doc = new DocLnk(f);
        doc->setType("application/portabase");
        if (name.length() > 40) {
            name = name.left(40);
        }
        doc->setName(name);
        QString defaultFile = doc->file();
        QFileInfo info(defaultFile);
        // calling file() created an empty file, delete it now
	QFile::remove(defaultFile);
        doc->setFile(info.dirPath(true) + "/" + info.baseName() + ".pob");
    }
#endif
    else {
        ok = FALSE;
    }
    if (ok) {
        FileManager fm;
        fm.saveFile(*doc, "");
        db = new Database(doc->file(), &ok);
        if (source == NO_DATA) {
            ok = editColumns();
        }
        else {
            ImportDialog dialog(MOBILEDB_FILE, db, this);
            ok = dialog.exec();
            if (ok) {
                finishNewFile(db);
            }
        }
        if (ok) {
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

void PortaBase::finishNewFile(Database *db)
{
    viewer->setDatabase(db);
    viewer->updateTable();
    viewer->updateButtons();
    showDataViewer();
    rebuildViewMenu();
    rebuildSortMenu();
    rebuildFilterMenu();
    setEdited(TRUE);
}

void PortaBase::openFile()
{
    const DocLnk *selection = fileSelector->selected();
    if (selection == 0) {
        return;
    }
    openFile(*selection);
    delete selection;
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
    showDataViewer();
    updateCaption();
    rebuildViewMenu();
    rebuildSortMenu();
    rebuildFilterMenu();
}

void PortaBase::fileOpen()
{
    showFileSelector();
    updateCaption();
}

void PortaBase::deleteFile()
{
    const DocLnk *selection = fileSelector->selected();
    if (selection == 0) {
        return;
    }
    if (QMessageBox::warning(this, tr("PortaBase"), tr("Delete") + " \""
                             + selection->name() + "\"\n"
                             + tr("Are you sure?"),
                             tr("Yes"), tr("No"), QString::null, 1) > 0) {
        return;
    }
    QFile::remove(selection->file());
    QFile::remove(selection->linkFile());
    delete selection;
    fileSelector->reread();
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
    showFileSelector();
    delete doc;
    doc = 0;
    delete db;
    db = 0;
    updateCaption();
}

void PortaBase::showFileSelector()
{
    menu->clear();
    toolbar->clear();
    QPopupMenu *file = new QPopupMenu(this);
    fileNewAction->addTo(file);
    fileNewAction->addTo(toolbar);
    fileOpenAction->addTo(file);
    fileOpenAction->addTo(toolbar);
#ifndef DESKTOP
    fileDeleteAction->addTo(file);
    fileDeleteAction->addTo(toolbar);
#endif
    file->insertSeparator();
    importMobileDBAction->addTo(file);
    file->insertSeparator();
    quitAction->addTo(file);

    menu->insertItem(tr("File"), file);
#ifdef DESKTOP
    QPopupMenu *help = new QPopupMenu(this);
    helpAction->addTo(help);
    help->insertSeparator();
    aboutAction->addTo(help);
    aboutQtAction->addTo(help);
    menu->insertItem(tr("Help"), help);
#endif
    fileSelector->reread();
    mainStack->raiseWidget(fileSelector);
}

void PortaBase::showDataViewer()
{
    menu->clear();
    toolbar->clear();

    // Toolbar
    fileSaveAction->addTo(toolbar);
    rowAddAction->addTo(toolbar);
    rowEditAction->addTo(toolbar);
    rowDeleteAction->addTo(toolbar);

    // File menu
    file = new QPopupMenu(this);
    fileSaveAction->addTo(file);
    dataImportAction->addTo(file);
    dataExportAction->addTo(file);
    deleteRowsAction->addTo(file);
    editColsAction->addTo(file);
    manageEnumsAction->addTo(file);
    prefsAction->addTo(file);
    file->insertSeparator();
    closeAction->addTo(file);

    // View menu
    view = new QPopupMenu(this);
    view->setCheckable(TRUE);
    viewAddAction->addTo(view);
    viewEditAction->addTo(view);
    viewDeleteAction->addTo(view);
    view->insertSeparator();
    viewAllColsAction->addTo(view);
    connect(view, SIGNAL(activated(int)), this, SLOT(changeView(int)));

    // Sort menu
    sort = new QPopupMenu(this);
    sort->setCheckable(TRUE);
    sortAddAction->addTo(sort);
    sortEditAction->addTo(sort);
    sortDeleteAction->addTo(sort);
    sort->insertSeparator();
    connect(sort, SIGNAL(activated(int)), this, SLOT(changeSorting(int)));

    // Filter menu
    filter = new QPopupMenu(this);
    filter->setCheckable(TRUE);
    findAction->addTo(filter);
    filterAddAction->addTo(filter);
    filterEditAction->addTo(filter);
    filterDeleteAction->addTo(filter);
    filter->insertSeparator();
    filterAllRowsAction->addTo(filter);
    connect(filter, SIGNAL(activated(int)), this, SLOT(changeFilter(int)));

    menu->insertItem(tr("File"), file);
    menu->insertItem(tr("View"), view);
    menu->insertItem(tr("Sort"), sort);
    menu->insertItem(tr("Filter"), filter);
#ifdef DESKTOP
    help = new QPopupMenu(this);
    helpAction->addTo(help);
    help->insertSeparator();
    aboutAction->addTo(help);
    aboutQtAction->addTo(help);
    menu->insertItem(tr("Help"), help);
#endif
    mainStack->raiseWidget(viewer);
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
    ImportDialog dialog(CSV_FILE, db, this);
    if (dialog.exec()) {
        viewer->updateTable();
        viewer->updateButtons();
        setEdited(TRUE);
    }
}

void PortaBase::dataExport()
{
#if defined(DESKTOP)
    QString file = QFileDialog::getSaveFileName(QPEApplication::documentDir(),
                     tr("Text files with comma separated values") + " (*.csv)",
                     this,
                     "csv export dialog",
                     tr("Choose a filename to save under"));
    if (!file.isEmpty()) {
        DocLnk csv;
        csv.setType("text/x-csv");
        QFileInfo info(file);
        csv.setName(info.baseName());
        csv.setFile(info.dirPath(true) + "/" + info.baseName() + ".csv");
        FileManager fm;
        fm.saveFile(csv, "");
        viewer->exportToCSV(csv.file());
    }
#else
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
        viewer->exportToCSV(csv.file());
    }
#endif
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
    int i;
    for (i = 0; i < count; i++) {
        view->removeItem(viewIds[i]);
    }
    viewIds.clear();
    // add new view names
    viewNames = db->listViews();
    viewNames.remove("_all");
    count = viewNames.count();
    for (i = 0; i < count; i++) {
        int id = view->insertItem(viewNames[i]);
        viewIds.append(id);
    }
    updateViewMenu();
}

void PortaBase::rebuildSortMenu()
{
    // remove old sorting names
    int count = sortNames.count();
    int i;
    for (i = 0; i < count; i++) {
        sort->removeItem(sortIds[i]);
    }
    sortIds.clear();
    // add new sorting names
    sortNames = db->listSortings();
    sortNames.remove("_single");
    count = sortNames.count();
    for (i = 0; i < count; i++) {
        int id = sort->insertItem(sortNames[i]);
        sortIds.append(id);
    }
    updateSortMenu();
}

void PortaBase::rebuildFilterMenu()
{
    // remove old filter names
    int count = filterNames.count();
    int i;
    for (i = 0; i < count; i++) {
        filter->removeItem(filterIds[i]);
    }
    filterIds.clear();
    // add new filter names
    filterNames = db->listFilters();
    filterNames.remove("_allrows");
    filterNames.remove("_simple");
    count = filterNames.count();
    for (i = 0; i < count; i++) {
        int id = filter->insertItem(filterNames[i]);
        filterIds.append(id);
    }
    updateFilterMenu();
}

void PortaBase::updateViewMenu()
{
    QString viewName = db->currentView();
    if (viewName == "_all") {
        viewAllColsAction->setOn(TRUE);
        viewEditAction->setEnabled(FALSE);
        viewDeleteAction->setEnabled(FALSE);
    }
    else {
        viewAllColsAction->setOn(FALSE);
        viewEditAction->setEnabled(TRUE);
        viewDeleteAction->setEnabled(TRUE);
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
        sortEditAction->setEnabled(FALSE);
        sortDeleteAction->setEnabled(FALSE);
    }
    else {
        sortEditAction->setEnabled(TRUE);
        sortDeleteAction->setEnabled(TRUE);
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
        filterAllRowsAction->setOn(TRUE);
        filterEditAction->setEnabled(FALSE);
        filterDeleteAction->setEnabled(FALSE);
    }
    else if (filterName == "_simple") {
        filterAllRowsAction->setOn(FALSE);
        filterEditAction->setEnabled(FALSE);
        filterDeleteAction->setEnabled(FALSE);
    }
    else {
        filterAllRowsAction->setOn(FALSE);
        filterEditAction->setEnabled(TRUE);
        filterDeleteAction->setEnabled(TRUE);
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
    ViewEditor editor(this);
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
    SortEditor editor(this);
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
    FilterEditor editor(this);
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
    ViewEditor editor(this);
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
    SortEditor editor(this);
    if (editor.edit(db, "")) {
        editor.applyChanges();
        viewer->setSorting(editor.getName());
        rebuildSortMenu();
        setEdited(TRUE);
    }
}

void PortaBase::addFilter()
{
    FilterEditor editor(this);
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

void PortaBase::showHelp()
{
#ifdef DESKTOP
    HelpBrowser helpBrowser(this);
    helpBrowser.exec();
#endif
}

void PortaBase::aboutPortaBase()
{
    QString message = tr("PortaBase") + " 1.5\n";
    message += tr("Copyright (C)") + " 2002-2003 Jeremy Bowman\n\n";
    message += tr("Web site at http://portabase.sourceforge.net");
    QMessageBox::information(this, tr("About PortaBase"), message);
}

void PortaBase::aboutQt()
{
    QMessageBox::aboutQt(this, tr("PortaBase"));
}
