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
#include <qinputdialog.h>
typedef QInputDialog InputDialog;
#include "desktop/config.h"
#include "desktop/fileselector.h"
#include "desktop/helpbrowser.h"
#include "desktop/newfiledialog.h"
#include "desktop/qpeapplication.h"
#include "desktop/qpemenubar.h"
#include "desktop/resource.h"
#include "desktop/importdialog.h"
#else
#include <qpe/config.h>
#include <qpe/qpemenubar.h>
#include <qpe/resource.h>
#include "inputdialog.h"
#if defined(SHARP)
#include "sharp/fileselector.h"
#include "sharp/importdialog.h"
#include "sharp/newfiledialog.h"
#else
#include "fileselector.h"
#include "importdialog.h"
#include "newfiledialog.h"
#endif
#endif

#include <qaction.h>
#include <qapplication.h>
#include <qdir.h>
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
#include "passdialog.h"
#include "pbdialog.h"
#include "portabase.h"
#include "preferences.h"
#include "sorteditor.h"
#include "viewdisplay.h"
#include "vieweditor.h"

PortaBase::PortaBase(QWidget *parent, const char *name, WFlags f)
  : QMainWindow(parent, name, f), db(0), doc(0), isEdited(FALSE), needsRefresh(FALSE)
{
#ifndef DESKTOP
    setToolBarsMovable(FALSE);
#endif
    QFont currentFont = qApp->font();
    QString family = currentFont.family().lower();
    int size = currentFont.pointSize();
    Config conf("portabase");
    conf.setGroup("General");
    confirmDeletions = conf.readBoolEntry("ConfirmDeletions", TRUE);
    booleanToggle = conf.readBoolEntry("BooleanToggle");
    bool pagedDisplay = conf.readBoolEntry("PagedDisplay", TRUE);
    conf.setGroup("Font");
    family = conf.readEntry("Name", family);
    size = conf.readNumEntry("Size", size);
    QFont font(family, size);
    qApp->setFont(font);
    setFont(font);

    conf.setGroup("Colors");
    QString color = conf.readEntry("EvenRows", "#FFFFFF");
    PortaBase::evenRowColor = new QColor(color);
    color = conf.readEntry("OddRows", "#E0E0E0");
    PortaBase::oddRowColor = new QColor(color);

    // menu and toolbar, shared between file selector and data viewer modes
#if defined(DESKTOP)
    menu = menuBar();
    toolbar = new QToolBar(this);
    addToolBar(toolbar, QMainWindow::Top, TRUE);
    statusBar();
#else
    QToolBar *bar = new QToolBar(this);
    addToolBar(bar, QMainWindow::Top);
    bar->setHorizontalStretchable(TRUE);
    menu = new QPEMenuBar(bar);
    toolbar = new QToolBar(this);
    addToolBar(toolbar, QMainWindow::Top, FALSE);
#endif

    // menu and toolbar icon sets
    QIconSet addIcons = Resource::loadIconSet("new");
    QIconSet openIcons = Resource::loadIconSet("fileopen");
    QIconSet deleteIcons = Resource::loadIconSet("trash");
    QIconSet copyIcons = Resource::loadIconSet("copy");
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
    fileCopyAction = new QAction(tr("Copy") + "...", copyIcons,QString::null,
                                 0, this);
    connect(fileCopyAction, SIGNAL(activated()), this, SLOT(copyFile()));
    fileRenameAction = new QAction(tr("Rename") + "...", QString::null, 0,
                                   this);
    connect(fileRenameAction, SIGNAL(activated()), this, SLOT(renameFile()));
    refreshAction = new QAction(tr("Refresh"), QString::null, 0, this);
    connect(refreshAction, SIGNAL(activated()), this, SLOT(refreshFileList()));
    importAction = new QAction(tr("Import") + "...", QString::null, 0, this);
    connect(importAction, SIGNAL(activated()), this, SLOT(import()));
    quitAction = new QAction(tr("Quit"), quitIcons, QString::null, 0, this);
    connect(quitAction, SIGNAL(activated()), this, SLOT(close()));

    viewListAction = new QAction(tr("List"), QString::null, 0, this, 0, TRUE);
    connect(viewListAction, SIGNAL(activated()), this, SLOT(viewList()));
    viewIconsAction = new QAction(tr("Icons"), QString::null, 0, this, 0,
                                  TRUE);
    connect(viewIconsAction, SIGNAL(activated()), this, SLOT(viewIcons()));

    // File menu actions
    fileSaveAction = new QAction(tr("Save"), saveIcons,
                                 QString::null, 0, this);
    connect(fileSaveAction, SIGNAL(activated()), this, SLOT(save()));
    changePassAction = new QAction(tr("Change Password"), QString::null, 0,
                                   this);
    connect(changePassAction, SIGNAL(activated()),
            this, SLOT(changePassword()));
    dataImportAction = new QAction(tr("Import"), QString::null, 0, this);
    connect(dataImportAction, SIGNAL(activated()), this, SLOT(dataImport()));
    exportAction = new QAction(tr("Export"), QString::null, 0, this);
    connect(exportAction, SIGNAL(activated()), this, SLOT(dataExport()));
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

    // Row menu/toolbar actions
    rowAddAction = new QAction(tr("Add"), addIcons, QString::null, 0, this);
    connect(rowAddAction, SIGNAL(activated()), this, SLOT(addRow()));
    rowEditAction = new QAction(tr("Edit"), editIcons, QString::null, 0,
                                this);
    connect(rowEditAction, SIGNAL(activated()), this, SLOT(editRow()));
    rowDeleteAction = new QAction(tr("Delete"), deleteIcons, QString::null,
                                  0, this);
    connect(rowDeleteAction, SIGNAL(activated()), this, SLOT(deleteRow()));
    rowCopyAction = new QAction(tr("Copy"), copyIcons, QString::null, 0, this);
    connect(rowCopyAction, SIGNAL(activated()), this, SLOT(copyRow()));
    // this submenu doesn't get deleted when the menubar is cleared...
    row = new QPopupMenu(this);
    rowAddAction->addTo(row);
    rowEditAction->addTo(row);
    rowDeleteAction->addTo(row);
    rowCopyAction->addTo(row);

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
    findAction = new QAction(tr("Quick Filter"), findIcons, QString::null, 0,
                             this);
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
    viewer->usePages(pagedDisplay);

    conf.setGroup("Files");
    QDir defaultDir = QDir::home();
    defaultDir.cd("Documents");
    QString lastDir = conf.readEntry("LastDir", defaultDir.absPath());
    if (!QDir(lastDir).exists()) {
        lastDir = defaultDir.absPath();
    }
    QStringList recentFiles = getRecentFiles(conf);
    fileSelector = new PBFileSelector(lastDir, recentFiles,
                                      "application/portabase", mainStack);
    connect(fileSelector, SIGNAL(fileSelected(const DocLnk &)), this,
            SLOT(openFile(const DocLnk &)));
    QString viewMode = conf.readEntry("View", "Icon");
    if (viewMode == "Icon") {
        viewIcons();
    }
    else {
        viewList();
    }
    fileOpen();
#if defined(DESKTOP)
    resize(600, 400);
#else
    resize(200, 300);
#endif
}

const QColor *PortaBase::evenRowColor = &Qt::white;
const QColor *PortaBase::oddRowColor = &Qt::lightGray;

PortaBase::~PortaBase()
{
    Config conf("portabase");
    conf.setGroup("Files");
    QDir lastDir(fileSelector->currentDir());
    conf.writeEntry("LastDir", lastDir.absPath());
    conf.writeEntry("View", viewIconsAction->isOn() ? "Icon" : "List");
    updateRecentFiles(conf);
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
            db->addView("_all", db->listColumns(), "_none", "_none");
            viewer->setDatabase(db);
            rebuildViewMenu();
            rebuildSortMenu();
            rebuildFilterMenu();
            setEdited(TRUE);
            showDataViewer();
        }
        else {
            db->setViewColumnSequence("_all", db->listColumns());
            viewAllColumns();
            setEdited(TRUE);
            showDataViewer();
            rebuildViewMenu();
            rebuildSortMenu();
            rebuildFilterMenu();
        }
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
    if (manager.changesMade()) {
        viewer->setDatabase(db);
        setEdited(TRUE);
    }
    else {
        viewer->setView(db->currentView());
    }
}

void PortaBase::editPreferences()
{
    Preferences prefs(this);
    if (prefs.exec()) {
        QFont font = prefs.applyChanges();
        setFont(font);
        viewer->updateButtonSizes();
        file->setFont(font);
        if (doc) {
            showDataViewer();
            rebuildViewMenu();
            rebuildSortMenu();
            rebuildFilterMenu();
        }
#ifdef DESKTOP
        help->setFont(font);
        fileSelector->setFont(font);
#endif
        Config conf("portabase");
        conf.setGroup("General");
        confirmDeletions = conf.readBoolEntry("ConfirmDeletions");
        booleanToggle = conf.readBoolEntry("BooleanToggle");
        bool pagedDisplay = conf.readBoolEntry("PagedDisplay");
        viewer->allowBooleanToggle(booleanToggle);
        viewer->usePages(pagedDisplay);
        if (doc) {
            db->setShowSeconds(conf.readBoolEntry("ShowSeconds"));
            db->updateDateTimePrefs();
            viewer->updateTable();
        }
    }
}

void PortaBase::newFile()
{
    createFile(NO_DATA);
}

void PortaBase::import()
{
    QStringList types;
    types.append(tr("XML"));
    types.append(tr("MobileDB"));
    bool ok = FALSE;
    QString type = InputDialog::getItem(tr("Import"), tr("Import from:"),
                                        types, 0, FALSE, &ok, this);
    if (!ok) {
        return;
    }
    DocLnk f;
    if (type == types[0]) {
        createFile(XML_FILE);
    }
    else {
        createFile(MOBILEDB_FILE);
    }
}

void PortaBase::createFile(int source)
{
    bool ok = FALSE;
    NewFileDialog filedlg(".pob", this);
    if (filedlg.exec()) {
        DocLnk *file = filedlg.doc();
        if (file != 0) {
            doc = file;
            ok = TRUE;
        }
    }
    if (!ok) {
        QMessageBox::warning(this, tr("PortaBase"),
                             tr("Unable to create new file"));
        return;
    }
    fileSelector->initFile(*doc);
    int openResult;
    bool encrypt = filedlg.encryption();
    db = new Database(doc->file(), &openResult, encrypt ? 1 : 0);
    if (encrypt) {
        PasswordDialog passdlg(db, NEW_PASSWORD, this);
        bool finished = FALSE;
        while (!finished) {
            if (!passdlg.exec()) {
                finished = TRUE;
                ok = FALSE;
            }
            else {
                finished = passdlg.validate();
            }
        }
    }
    if (ok) {
        db->load();
        if (source == NO_DATA) {
            ok = editColumns();
        }
        else {
            ImportDialog dialog(source, db, this);
            ok = dialog.exec();
            if (ok) {
                finishNewFile(db);
            }
        }
    }
    if (ok) {
        updateCaption();
        // if not saved now, file is empty without later save...bad
        save();
        needsRefresh = TRUE;
    }
    else {
        delete db;
        db = 0;
        doc->removeFiles();
        delete doc;
        doc = 0;
    }
}

void PortaBase::finishNewFile(Database *db)
{
    Config conf("portabase");
    updateRecentFiles(conf);
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
    openFile(nf);
}

void PortaBase::openFile(const DocLnk &f)
{
    int openResult;
    Database *temp = new Database(f.file(), &openResult);
    if (openResult == OPEN_NEWER_VERSION) {
        QMessageBox::warning(this, tr("PortaBase"), tr("This file uses a newer version of the\nPortaBase format than this version\nof PortaBase supports; please\nupgrade"));
        delete temp;
        return;
    }
    else if (openResult == OPEN_ENCRYPTED) {
        PasswordDialog passdlg(temp, OPEN_PASSWORD, this);
        if (!passdlg.exec()) {
            delete temp;
            return;
        }
        if (!passdlg.validate()) {
            delete temp;
            return;
        }
    }
    else {
        temp->load();
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
    Config conf("portabase");
    updateRecentFiles(conf);
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
        delete selection;
        return;
    }
    QFile::remove(selection->file());
    QFile::remove(selection->linkFile());
    delete selection;
    fileSelector->reread();
}

void PortaBase::copyFile()
{
    fileSelector->duplicate();
}

void PortaBase::renameFile()
{
    fileSelector->rename();
}

void PortaBase::refreshFileList()
{
    fileSelector->reread();
    needsRefresh = FALSE;
}

void PortaBase::viewList()
{
#ifdef SHARP
    fileSelector->setListView();
    viewListAction->setOn(TRUE);
    viewIconsAction->setOn(FALSE);
    Config conf("portabase");
    conf.setGroup("Files");
    conf.writeEntry("View", "List");
#endif
}

void PortaBase::viewIcons()
{
#ifdef SHARP
    fileSelector->setIconView();
    viewListAction->setOn(FALSE);
    viewIconsAction->setOn(TRUE);
    Config conf("portabase");
    conf.setGroup("Files");
    conf.writeEntry("View", "Icon");
#endif
}

void PortaBase::openRecent(int id)
{
    QString path = recent->text(id);
    if (!QFile::exists(path)) {
        QMessageBox::warning(this, tr("PortaBase"), tr("File does not exist"));
        return;
    }
    openFile(path);
}

void PortaBase::updateCaption(const QString &name)
{
    if (!doc) {
        setCaption(tr("PortaBase") + PBDialog::titleSuffix);
    }
    else {
        QString s = name;
        if (s.isNull()) {
            s = doc->name();
        }
        setCaption(s + " - " + tr("PortaBase") + PBDialog::titleSuffix);
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
    viewIds.clear();
    sortIds.clear();
    filterIds.clear();

    file = new QPopupMenu(this);
    fileNewAction->addTo(file);
    fileNewAction->addTo(toolbar);
    fileOpenAction->addTo(file);
    fileOpenAction->addTo(toolbar);
#ifdef DESKTOP
    recent = new QPopupMenu(this);
    Config conf("portabase");
    QStringList recentFiles = getRecentFiles(conf);
    int count = recentFiles.count();
    for (int i = 0; i < count; i++) {
        recent->insertItem(recentFiles[i]);
    }
    connect(recent, SIGNAL(activated(int)), this, SLOT(openRecent(int)));
    file->insertItem(tr("Open Recent"), recent);
#else
    fileDeleteAction->addTo(file);
    fileDeleteAction->addTo(toolbar);
    fileRenameAction->addTo(file);
    fileCopyAction->addTo(file);
    refreshAction->addTo(file);
#endif
    file->insertSeparator();
    importAction->addTo(file);
    prefsAction->addTo(file);
    file->insertSeparator();
    quitAction->addTo(file);

    menu->insertItem(tr("File"), file);

#ifdef SHARP
    view = new QPopupMenu(this);
    view->setCheckable(TRUE);
    viewListAction->addTo(view);
    viewIconsAction->addTo(view);
    menu->insertItem(tr("View"), view);
#endif

#ifdef DESKTOP
    help = new QPopupMenu(this);
    helpAction->addTo(help);
    help->insertSeparator();
    aboutAction->addTo(help);
    aboutQtAction->addTo(help);
    menu->insertItem(tr("Help"), help);
#endif
    if (needsRefresh) {
        fileSelector->reread();
        needsRefresh = FALSE;
    }
    mainStack->raiseWidget(fileSelector);
}

void PortaBase::showDataViewer()
{
    menu->clear();
    toolbar->clear();

    // Toolbar
    QStringList shown;
    QStringList hidden;
    Preferences::buttonConfiguration(shown, hidden);
    int count = shown.count();
    int i;
    for (i = 0; i < count; i++) {
        QAction *action = getButtonAction(shown[i]);
        action->addTo(toolbar);
    }

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

    // Load menu preferences
    QStringList topLevel;
    QStringList underFile;
    Preferences::menuConfiguration(topLevel, underFile);

    // File menu
    file = new QPopupMenu(this);
    count = underFile.count();
    for (i = 0; i < count; i++) {
        QString menuName = underFile[i];
        file->insertItem(getMenuLabel(menuName), getMenuPointer(menuName));
    }
    fileSaveAction->addTo(file);
    if (db->encrypted()) {
        changePassAction->addTo(file);
    }
    dataImportAction->addTo(file);
    exportAction->addTo(file);
    deleteRowsAction->addTo(file);
    editColsAction->addTo(file);
    manageEnumsAction->addTo(file);
    prefsAction->addTo(file);
    file->insertSeparator();
    closeAction->addTo(file);

    menu->insertItem(tr("File"), file);
    count = topLevel.count();
    for (i = 0; i < count; i++) {
        QString menuName = topLevel[i];
        menu->insertItem(getMenuLabel(menuName), getMenuPointer(menuName));
    }
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

QString PortaBase::getMenuLabel(const QString &menuName)
{
    if (menuName == "Row") {
        return tr("Row");
    }
    else if (menuName == "View") {
        return tr("View");
    }
    else if (menuName == "Sort") {
        return tr("Sort");
    }
    else {
        return tr("Filter");
    }
}

QPopupMenu *PortaBase::getMenuPointer(const QString &menuName)
{
    if (menuName == "Row") {
        return row;
    }
    else if (menuName == "View") {
        return view;
    }
    else if (menuName == "Sort") {
        return sort;
    }
    else {
        return filter;
    }
}

QAction *PortaBase::getButtonAction(const QString &buttonName)
{
    if (buttonName == "Save") {
        return fileSaveAction;
    }
    else if (buttonName == "Add") {
        return rowAddAction;
    }
    else if (buttonName == "Edit") {
        return rowEditAction;
    }
    else if (buttonName == "Copy") {
        return rowCopyAction;
    }
    else if (buttonName == "Delete") {
        return rowDeleteAction;
    }
    else {
        return findAction;
    }
}

QStringList PortaBase::getRecentFiles(Config &conf)
{
    conf.setGroup("Files");
    QStringList recentFiles;
    for (int i = 0; i < 5; i++) {
        QString path = conf.readEntry(QString().sprintf("Recent%i", i), "");
        if (!path.isEmpty()) {
            recentFiles.append(path);
        }
    }
    return recentFiles;
}

void PortaBase::updateRecentFiles(Config &conf)
{
    conf.setGroup("Files");
    QStringList files = fileSelector->recent();
    int count = files.count();
    int i;
#ifdef DESKTOP
    recent->clear();
    for (i = 0; i < count; i++) {
        recent->insertItem(files[i]);
    }
#endif
    while (files.count() < 5) {
        files.append("");
    }
    for (i = 0; i < 5; i++) {
        conf.writeEntry(QString().sprintf("Recent%i", i), files[i]);
    }
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

void PortaBase::copyRow()
{
    viewer->editRow(-1, TRUE);
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

void PortaBase::changePassword()
{
    PasswordDialog passdlg(db, CHANGE_PASSWORD, this);
    if (passdlg.exec()) {
        if (passdlg.validate()) {
            setEdited(TRUE);
        }
    }
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
    QStringList types;
    types.append(tr("CSV") + "(" + tr("rows in current filter") + ")");
    types.append(tr("XML"));
    bool ok = FALSE;
    QString type = InputDialog::getItem(tr("Export"), tr("Export to:"),
                                        types, 0, FALSE, &ok, this);
    if (!ok) {
        return;
    }
    QString extension = ".csv";
    if (type == types[1]) {
        extension = ".xml";
    }
    NewFileDialog filedlg(extension, this);
    if (!filedlg.exec()) {
        return;
    }
    DocLnk *output = filedlg.doc();
    if (output == 0) {
        return;
    }
    fileSelector->initFile(*output);
    if (extension == ".csv") {
        viewer->exportToCSV(output->file());
    }
    else {
        viewer->exportToXML(output->file());
    }
    delete output;
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
        viewer->setView(viewNames[index], TRUE);
        updateViewMenu();
        // there might be a default sorting and/or filter...
        updateSortMenu();
        updateFilterMenu();
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
    int count = viewIds.count();
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
    int count = sortIds.count();
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
    int count = filterIds.count();
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
    if (editor.edit(db, viewName, db->listViewColumns(viewName),
                    db->getDefaultSort(viewName),
                    db->getDefaultFilter(viewName))) {
        viewer->closeView();
        editor.applyChanges();
        QString newName = editor.getName();
        viewer->setView(newName, TRUE);
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
    if (editor.edit(db, "", empty, "_none", "_none")) {
        editor.applyChanges();
        viewer->setView(editor.getName(), TRUE);
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
    rowCopyAction->setEnabled(y);
}

QPixmap PortaBase::getCheckBoxPixmap(int checked)
{
    if (checked) {
        return Resource::loadPixmap("portabase/checked");
    }
    else {
        return Resource::loadPixmap("portabase/unchecked");
    }
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
    QString message = tr("PortaBase") + " 1.8\n";
    message += tr("Copyright (C)") + " 2002-2003 Jeremy Bowman\n\n";
    message += tr("Web site at http://portabase.sourceforge.net");
    QMessageBox::information(this, tr("About PortaBase"), message);
}

void PortaBase::aboutQt()
{
    QMessageBox::aboutQt(this, tr("PortaBase"));
}
