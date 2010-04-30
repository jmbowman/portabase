/*
 * portabase.cpp
 *
 * (c) 2002-2004,2008-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file portabase.cpp
 * Source file for PortaBase (main window class)
 */

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDirModel>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QFont>
#include <QGroupBox>
#include <QIconDragEvent>
#include <QInputDialog>
#include <QLayout>
#include <QLocale>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QProcess>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QStatusBar>
#include <QStyle>
#include <QTextDocument>
#include <QToolBar>
#include <QUrl>
#include "condition.h"
#include "conditioneditor.h"
#include "database.h"
#include "dbeditor.h"
#include "enummanager.h"
#include "factory.h"
#include "filter.h"
#include "filtereditor.h"
#include "menuactions.h"
#include "oldconfig.h"
#include "passdialog.h"
#include "portabase.h"
#include "preferences.h"
#include "sorteditor.h"
#include "viewdisplay.h"
#include "vieweditor.h"
#include "vsfmanager.h"

/**
 * Constructor.
 *
 * @param parent This window's parent widget, if any (usually none)
 */
PortaBase::PortaBase(QWidget *parent)
  : QMainWindow(parent), db(0), doc(""), printer(0), vsfManager(0)
{
#if defined(Q_WS_MAEMO_5)
    setAttribute(Qt::WA_Maemo5StackedWindow);
#else
    statusBar();
#endif
    QSettings *settings = getSettings();
    confirmDeletions = settings->value("General/ConfirmDeletions", true).toBool();
    booleanToggle = settings->value("General/BooleanToggle", false).toBool();
    bool pagedDisplay = settings->value("General/PagedDisplay", false).toBool();
    bool singleClickShow = settings->value("General/SingleClickShow", true).toBool();

    QString color = settings->value("Colors/EvenRows", "#FFFFFF").toString();
    Factory::evenRowColor = QColor(color);
    color = settings->value("Colors/OddRows", "#E0E0E0").toString();
    Factory::oddRowColor = QColor(color);

    mainStack = new QStackedWidget(this);
    setCentralWidget(mainStack);

    viewer = new ViewDisplay(this, mainStack);
    viewer->allowBooleanToggle(booleanToggle);
    viewer->usePages(pagedDisplay);
    viewer->showWithSingleClick(singleClickShow);
    mainStack->addWidget(viewer);

    // menu and toolbar, shared between file selector and data viewer modes
    toolbar = addToolBar("toolbar");
    toolbar->setObjectName("toolbar");
    ma = new MenuActions(this);
    mh = new QQMenuHelper(this, toolbar, tr("PortaBase files"), "pob", true);
    mh->loadSettings(settings);
    connect(mh, SIGNAL(newFile(const QString &)),
            this, SLOT(newFile(const QString &)));
    connect(mh, SIGNAL(openFile(const QString &)),
            this, SLOT(openFile(const QString &)));
    connect(mh, SIGNAL(closeFile()), this, SLOT(close()));
    connect(mh, SIGNAL(recentFilesChanged()),
            this, SLOT(updateRecentFileButtons()));
    connect(mh, SIGNAL(editPreferences()), this, SLOT(editPreferences()));
    connect(mh, SIGNAL(quit()), this, SLOT(quit()));
    connect(mh, SIGNAL(saveFile()), this, SLOT(save()));
    connect(mh, SIGNAL(aboutApplication()), this, SLOT(aboutPortaBase()));

    // frequently used menu and toolbar icons
    QIcon addIcon = QIcon(":/icons/add.png");
    QIcon editIcon = QIcon(":/icons/edit.png");
    QIcon deleteIcon = QIcon(":/icons/delete.png");

    // file selector actions
    importAction = ma->action(MenuActions::Import, QIcon(":/icons/import.png"));
    connect(importAction, SIGNAL(triggered()), this, SLOT(import()));

    // File menu actions
    changePassAction = ma->action(MenuActions::ChangePassword);
    connect(changePassAction, SIGNAL(triggered()),
            this, SLOT(changePassword()));
    dataImportAction = ma->action(MenuActions::ImportCSV);
    connect(dataImportAction, SIGNAL(triggered()), this, SLOT(dataImport()));
    exportAction = ma->action(MenuActions::Export);
    connect(exportAction, SIGNAL(triggered()), this, SLOT(dataExport()));
    deleteRowsAction = ma->action(MenuActions::DeleteRowsInFilter, deleteIcon);
    connect(deleteRowsAction, SIGNAL(triggered()),
            this, SLOT(deleteAllRows()));
    editColsAction = ma->action(MenuActions::EditColumns);
    connect(editColsAction, SIGNAL(triggered()), this, SLOT(editColumns()));
    manageEnumsAction = ma->action(MenuActions::EditEnums);
    connect(manageEnumsAction, SIGNAL(triggered()), this, SLOT(editEnums()));
    slideshowAction = ma->action(MenuActions::Slideshow);
    connect(slideshowAction, SIGNAL(triggered()), viewer, SLOT(slideshow()));
    propsAction = ma->action(MenuActions::Properties);
    connect(propsAction, SIGNAL(triggered()), this, SLOT(viewProperties()));
    printPreviewAction = ma->action(MenuActions::PrintPreview);
    connect(printPreviewAction, SIGNAL(triggered()), this, SLOT(printPreview()));
    printAction = ma->action(MenuActions::Print);
    connect(printAction, SIGNAL(triggered()), this, SLOT(print()));
    fileSeparatorAction = new QAction(this);
    fileSeparatorAction->setSeparator(true);

    // File menu
    mh->addToFileMenu(fileSeparatorAction);
    mh->addToFileMenu(importAction);
    mh->addToFileMenu(changePassAction);
    mh->addToFileMenu(dataImportAction);
    mh->addToFileMenu(exportAction);
    mh->addToFileMenu(deleteRowsAction);
    mh->addToFileMenu(editColsAction);
    mh->addToFileMenu(manageEnumsAction);
    mh->addToFileMenu(slideshowAction);
    mh->addToFileMenu(propsAction);
    mh->addToFileMenu(printPreviewAction);
    mh->addToFileMenu(printAction);

    // Row menu/toolbar actions
    rowAddAction = ma->action(MenuActions::AddRow, addIcon);
    connect(rowAddAction, SIGNAL(triggered()), viewer, SLOT(addRow()));
    rowEditAction = ma->action(MenuActions::EditRow, editIcon);
    connect(rowEditAction, SIGNAL(triggered()), viewer, SLOT(editRow()));
    rowDeleteAction = ma->action(MenuActions::DeleteRow, deleteIcon);
    connect(rowDeleteAction, SIGNAL(triggered()), this, SLOT(deleteRow()));
    rowCopyAction = ma->action(MenuActions::CopyRow, QIcon(":/icons/copy_row.png"));
    connect(rowCopyAction, SIGNAL(triggered()), this, SLOT(copyRow()));
    rowViewAction = ma->action(MenuActions::Show);
    connect(rowViewAction, SIGNAL(triggered()), viewer, SLOT(viewRow()));
    row = new QMenu(ma->menuText(MenuActions::Row), this);
    row->addAction(rowAddAction);
    row->addAction(rowEditAction);
    row->addAction(rowDeleteAction);
    row->addAction(rowCopyAction);
    row->addAction(rowViewAction);

    // View menu actions
    viewAddAction = ma->action(MenuActions::AddView, addIcon);
    connect(viewAddAction, SIGNAL(triggered()), this, SLOT(addView()));
    viewEditAction = ma->action(MenuActions::EditView, editIcon);
    connect(viewEditAction, SIGNAL(triggered()), this, SLOT(editView()));
    viewDeleteAction = ma->action(MenuActions::DeleteView, deleteIcon);
    connect(viewDeleteAction, SIGNAL(triggered()), this, SLOT(deleteView()));
    viewAllColsAction = ma->action(MenuActions::AllColumns, true);
    connect(viewAllColsAction, SIGNAL(triggered()),
            this, SLOT(viewAllColumns()));

    // View menu
    view = new QMenu(ma->menuText(MenuActions::View), this);
    view->addAction(viewAddAction);
    view->addAction(viewEditAction);
    view->addAction(viewDeleteAction);
    view->addSeparator();
    view->addAction(viewAllColsAction);
    connect(view, SIGNAL(triggered(QAction*)), this, SLOT(changeView(QAction*)));

    // Sort menu actions
    sortAddAction = ma->action(MenuActions::AddSorting, addIcon);
    connect(sortAddAction, SIGNAL(triggered()), this, SLOT(addSorting()));
    sortEditAction = ma->action(MenuActions::EditSorting, editIcon);
    connect(sortEditAction, SIGNAL(triggered()), this, SLOT(editSorting()));
    sortDeleteAction = ma->action(MenuActions::DeleteSorting, deleteIcon);
    connect(sortDeleteAction, SIGNAL(triggered()),
            this, SLOT(deleteSorting()));

    // Sort menu
    sort = new QMenu(ma->menuText(MenuActions::Sort), this);
    sort->addAction(sortAddAction);
    sort->addAction(sortEditAction);
    sort->addAction(sortDeleteAction);
    sort->addSeparator();
    connect(sort, SIGNAL(triggered(QAction*)), this, SLOT(changeSorting(QAction*)));

    // Filter menu actions
    findAction = ma->action(MenuActions::QuickFilter, QIcon(":/icons/find.png"));
    connect(findAction, SIGNAL(triggered()), this, SLOT(simpleFilter()));
    filterAddAction = ma->action(MenuActions::AddFilter, addIcon);
    connect(filterAddAction, SIGNAL(triggered()), this, SLOT(addFilter()));
    filterEditAction = ma->action(MenuActions::EditFilter, editIcon);
    connect(filterEditAction, SIGNAL(triggered()), this, SLOT(editFilter()));
    filterDeleteAction = ma->action(MenuActions::DeleteFilter, deleteIcon);
    connect(filterDeleteAction, SIGNAL(triggered()),
            this, SLOT(deleteFilter()));
    filterAllRowsAction = ma->action(MenuActions::AllRows, true);
    connect(filterAllRowsAction, SIGNAL(triggered()),
            this, SLOT(viewAllRows()));

    // Filter menu
    filter = new QMenu(ma->menuText(MenuActions::Filter), this);
    filter->addAction(findAction);
    filter->addAction(filterAddAction);
    filter->addAction(filterEditAction);
    filter->addAction(filterDeleteAction);
    filter->addSeparator();
    filter->addAction(filterAllRowsAction);
    connect(filter, SIGNAL(triggered(QAction*)), this, SLOT(changeFilter(QAction*)));

    // Toolbar-only actions
    viewsAction = ma->action(MenuActions::Views, QIcon(":/icons/view.png"));
    connect(viewsAction, SIGNAL(triggered()), this, SLOT(changeView()));
    sortingsAction = ma->action(MenuActions::Sortings, QIcon(":/icons/sort.png"));
    connect(sortingsAction, SIGNAL(triggered()), this, SLOT(changeSorting()));
    filtersAction = ma->action(MenuActions::Filters, QIcon(":/icons/filter.png"));
    connect(filtersAction, SIGNAL(triggered()), this, SLOT(changeFilter()));
    fullscreenAction = ma->action(MenuActions::Fullscreen, QIcon(":/icons/fullscreen.png"));
    fullscreenAction->setCheckable(true);
    connect(fullscreenAction, SIGNAL(triggered()), this, SLOT(toggleFullscreen()));

    // Add menus to menubar
#if !defined(Q_WS_HILDON) && !defined(Q_WS_MAEMO_5)
    QAction *helpMenuAction = mh->helpMenu()->menuAction();
    menuBar()->insertMenu(helpMenuAction, row);
    menuBar()->insertMenu(helpMenuAction, view);
    menuBar()->insertMenu(helpMenuAction, sort);
    menuBar()->insertMenu(helpMenuAction, filter);
#endif

    // Toolbar
    mh->addToToolBar(importAction);
    mh->addToToolBar(rowAddAction);
    mh->addToToolBar(rowEditAction);
    mh->addToToolBar(rowDeleteAction);
    mh->addToToolBar(rowCopyAction);
    mh->addToToolBar(viewsAction);
    mh->addToToolBar(sortingsAction);
    mh->addToToolBar(filtersAction);
    mh->addToToolBar(findAction);
    mh->addToToolBar(fullscreenAction);
    createFillerActions();

    // Main widget when no file is open
    noFileWidget = new QScrollArea(mainStack);
    QWidget *buttonPanel = new QWidget();
    noFileWidget->setWidgetResizable(true);
    QHBoxLayout *hlayout = Factory::hBoxLayout(buttonPanel, true);
    hlayout->addStretch(1);
    QVBoxLayout *vlayout = Factory::vBoxLayout(hlayout);
    vlayout->addStretch(1);
    QAction *action = mh->action(QQMenuHelper::New);
    newButton = new QPushButton(action->icon(), action->statusTip(),
                                 buttonPanel);
    connect(newButton, SIGNAL(clicked()), action, SIGNAL(triggered()));
    vlayout->addWidget(newButton);
    action = mh->action(QQMenuHelper::Open);
    openButton = new QPushButton(action->icon(), action->statusTip(),
                                  buttonPanel);
    connect(openButton, SIGNAL(clicked()), action, SIGNAL(triggered()));
    vlayout->addWidget(openButton);
    importButton = new QPushButton(importAction->icon(),
                                    importAction->statusTip(),
                                    buttonPanel);
    connect(importButton, SIGNAL(clicked()), importAction, SIGNAL(triggered()));
    vlayout->addWidget(importButton);
    recentBox = new QGroupBox(tr("Recently opened files"), buttonPanel);
    recentBox->setAlignment(Qt::AlignHCenter);
    QVBoxLayout *boxLayout = Factory::vBoxLayout(recentBox, true);
    for (int i = 0; i < MAX_RECENT_FILES; i++) {
        recentButtons[i] = new QPushButton("", recentBox);
        boxLayout->addWidget(recentButtons[i]);
    }
    connect(recentButtons[0], SIGNAL(clicked()),
            mh->action(QQMenuHelper::Recent1), SIGNAL(triggered()));
    connect(recentButtons[1], SIGNAL(clicked()),
            mh->action(QQMenuHelper::Recent2), SIGNAL(triggered()));
    connect(recentButtons[2], SIGNAL(clicked()),
            mh->action(QQMenuHelper::Recent3), SIGNAL(triggered()));
    connect(recentButtons[3], SIGNAL(clicked()),
            mh->action(QQMenuHelper::Recent4), SIGNAL(triggered()));
    connect(recentButtons[4], SIGNAL(clicked()),
            mh->action(QQMenuHelper::Recent5), SIGNAL(triggered()));
    vlayout->addWidget(recentBox);
    vlayout->addStretch(1);
    hlayout->addStretch(1);
    noFileWidget->setWidget(buttonPanel);
    mainStack->addWidget(noFileWidget);

    setUnifiedTitleAndToolBarOnMac(true);
    showFileSelector();
    updateCaption();
    restoreWindowSettings(settings);
    setAcceptDrops(true);
    delete settings;
    vsfManager = new VSFManager(this);
}

/**
 * Destructor.
 */
PortaBase::~PortaBase()
{
    QSettings settings;
    mh->saveSettings(&settings);
    saveWindowSettings(&settings);
    if (printer) {
        delete printer;
    }
}

/**
 * Restore the window position, size, etc. from the provided application
 * settings object.
 *
 * @param settings The application settings to load from
 */
void PortaBase::restoreWindowSettings(QSettings *settings)
{
    settings->beginGroup("Geometry");
    int xpos = settings->value("X", -1).toInt();
    int ypos = settings->value("Y", -1).toInt();
    if (xpos != -1 && ypos != -1) {
        move(xpos, ypos);
    }
    if (settings->value("Maximized").toBool()) {
        resize(600, 400);
        showMaximized();
    }
    else {
        int w = settings->value("Width", 600).toInt();
        int h = settings->value("Height", 400).toInt();
        resize(w, h);
    }
    if (settings->contains("State")) {
        restoreState(settings->value("State").toByteArray());
        // make sure the toolbar isn't and can't be hidden
        if (toolbar->isHidden()) {
            toolbar->show();
        }
        toolbar->toggleViewAction()->setEnabled(false);
        toolbar->toggleViewAction()->setVisible(false);
    }
    settings->endGroup();
}

/**
 * Save the window position, size, etc. to the provided application
 * settings object.
 *
 * @param settings The application settings to save to
 */
void PortaBase::saveWindowSettings(QSettings *settings)
{
    settings->beginGroup("Geometry");
    settings->setValue("Maximized", isMaximized());
    settings->setValue("X", x());
    settings->setValue("Y", y());
    settings->setValue("Width", width());
    settings->setValue("Height", height());
    settings->setValue("State", saveState());
    settings->endGroup();
}

/**
 * Switch from normal mode to fullscreen or vice versa.
 */
void PortaBase::toggleFullscreen()
{
    if (isFullScreen()) {
        showNormal();
    }
    else {
        showFullScreen();
    }
}

/**
 * Launch the "Columns Editor" dialog to define or modify the structure of
 * the current database's main data table.
 *
 * @return True if changes were made and accepted, false otherwise
 */
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
        }
        else {
            db->setViewColumnSequence("_all", db->listColumns());
            viewAllColumns();
        }
        showDataViewer();
        setEdited(true);
        return true;
    }
    else {
        return false;
    }
}

/**
 * Launch a dialog to add, edit, or delete the enumerated data types in the
 * current database.  Called when the "Edit Enums" menu item is triggered.
 */
void PortaBase::editEnums()
{
    EnumManager manager(db, this);
    viewer->closeView();
    if (manager.exec()) {
        manager.applyChanges();
    }
    if (manager.changesMade()) {
        viewer->setDatabase(db);
        setEdited(true);
    }
    else {
        viewer->setView(db->currentView());
    }
}

/**
 * Launch a dialog which displays some basic statistics about the current
 * database.  Called when the "Properties" menu item is triggered.
 */
void PortaBase::viewProperties()
{
    QFile file(doc);
    QString message = tr("Name") + ": " + QFileInfo(file).fileName() + "\n";
    int size = file.size();
    QString units;
    if (size < 1024) {
        units = QFileSystemModel::tr("%1 bytes");
    }
    else if (size < 1024 * 1024) {
        size /= 1024;
        units = QFileSystemModel::tr("%1 KB");
    }
    else {
        size /= 1024 * 1024;
        units = QFileSystemModel::tr("%1 MB");
    }
    QLocale locale = QLocale::system();
    QString sizeString = locale.toString(size);
    message += QString("%1: %2\n").arg(tr("Size")).arg(units.arg(sizeString));
    int count = db->getData().GetSize();
    message += tr("Rows") + ": " + locale.toString(count) + "\n";
    count = db->listColumns().count();
    message += tr("Columns") + ": " + locale.toString(count) + "\n";
    count = db->listViews().count();
    message += tr("Views") + ": " + locale.toString(count) + "\n";
    QStringList sortings = db->listSortings();
    sortings.removeAll("_single");
    count = sortings.count();
    message += tr("Sortings") + ": " + locale.toString(count) + "\n";
    QStringList filters = db->listFilters();
    filters.removeAll("_simple");
    count = filters.count();
    message += tr("Filters") + ": " + locale.toString(count) + "\n";
    count = db->listEnums().count();
    message += tr("Enums") + ": " + locale.toString(count);
    QString title = tr("File Properties") + " - " + qApp->applicationName();
    QMessageBox mb(title, message, QMessageBox::NoIcon,
                   QMessageBox::Ok, QMessageBox::NoButton,
                   QMessageBox::NoButton, this);
    mb.setMinimumWidth(200);
    mb.exec();
}

/**
 * Launch a dialog which allows the user to customize many of the application
 * settings.  Called when the "Preferences" menu item is triggered.
 */
void PortaBase::editPreferences()
{
    Preferences prefs(this);
    if (prefs.exec()) {
        QFont font = prefs.applyChanges();
#if !defined(Q_WS_MAC)
        setFont(font);
        viewer->updateButtonSizes();
        mh->fileMenu()->setFont(font);
        noFileWidget->setFont(font);
        mh->helpMenu()->setFont(font);
#endif
        QSettings settings;
        settings.beginGroup("General");
        confirmDeletions = settings.value("ConfirmDeletions", true).toBool();
        booleanToggle = settings.value("BooleanToggle", false).toBool();
        bool pagedDisplay = settings.value("PagedDisplay", false).toBool();
        bool singleClickShow = settings.value("SingleClickShow", true).toBool();
        viewer->allowBooleanToggle(booleanToggle);
        viewer->usePages(pagedDisplay);
        viewer->showWithSingleClick(singleClickShow);
        if (!doc.isEmpty()) {
            showDataViewer();
            db->updatePreferences();
            viewer->resetTable();
        }
    }
}

/**
 * Finish the process of creating a new PortaBase file at the specified
 * location.  Called after the user has triggered the "New" file menu item
 * and already selected a directory and filename.
 *
 * @param file The path of the file to be created
 */
void PortaBase::newFile(const QString &file)
{
    createFile(ImportDialog::NoSource, file);
}

/**
 * Create a new PortaBase file based on the data found in a PortaBase-format
 * XML file or a MobileDB file.  Called when the "Import" menu item is
 * triggered while no database is open.
 */
void PortaBase::import()
{
    QStringList types;
    types.append(tr("XML"));
    types.append(tr("MobileDB"));
    bool ok = false;
    QString type = QInputDialog::getItem(this, MenuActions::tr("Import"),
                                         tr("Import from:"),
                                         types, 0, false, &ok);
    if (!ok) {
        return;
    }
    if (type == types[0]) {
        createFile(ImportDialog::XML);
    }
    else {
        createFile(ImportDialog::MobileDB);
    }
}

/**
 * Create a new PortaBase file from the specified data source at the given
 * location.  First asks if the new file is to be encrypted or not.  Once the
 * file is created, it is opened as the current database.
 *
 * @param source The source of data for the new file (may be "None")
 * @param file The path of the new file to be created
 */
void PortaBase::createFile(ImportDialog::DataSource source,
                           const QString &file)
{
    bool encrypted = false;
    QString f(file);
    if (f.isNull()) {
        f = createNewFile();
        if (file.isEmpty()) {
            return;
        }
    }
    QMessageBox crypt(qApp->applicationName(), tr("Encrypt the file?"),
                      QMessageBox::NoIcon, QMessageBox::Yes,
                      QMessageBox::No | QMessageBox::Default,
                      QMessageBox::NoButton, this);
    int result = crypt.exec();
    if (result == QMessageBox::Cancel) {
        return;
    }
    else if (result == QMessageBox::Yes) {
        encrypted = true;
    }
    doc = f;
    bool ok = true;
    Database::OpenResult openResult;
    db = new Database(doc, &openResult, encrypted);
    if (encrypted) {
        PasswordDialog passdlg(db, PasswordDialog::NewPassword, this);
        bool finished = false;
        while (!finished) {
            if (!passdlg.exec()) {
                finished = true;
                ok = false;
            }
            else {
                finished = passdlg.validate();
            }
        }
    }
    if (ok) {
        db->load();
        if (source == ImportDialog::NoSource) {
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
    }
    else {
        delete db;
        db = 0;
        QFile::remove(doc);
        doc = "";
    }
}

/**
 * Finish the work of updating the display for a database which was imported
 * from an outside source (such as an XML or MobileDB file).
 *
 * @param db The database being created
 */
void PortaBase::finishNewFile(Database *db)
{
    viewer->setDatabase(db);
    showDataViewer();
    setEdited(true);
}

/**
 * Open the PortaBase file which is at the specified location.
 *
 * @param file The path of the file to be opened
 */
void PortaBase::openFile(const QString &file)
{
    if (!doc.isEmpty()) {
        // currently only support one open file at a time
        return;
    }
    Database::OpenResult openResult;
    Database *temp = new Database(file, &openResult);
    if (openResult == Database::NewerVersion) {
        QMessageBox::warning(this, qApp->applicationName(),
                             tr("This file uses a newer version of the\nPortaBase format than this version\nof PortaBase supports; please\nupgrade"));
        delete temp;
        return;
    }
    else if (openResult == Database::Encrypted) {
        PasswordDialog passdlg(temp, PasswordDialog::OpenFile, this);
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
    doc = file;
    if (db) {
        delete db;
    }
    db = temp;
    viewer->setDatabase(db);
    showDataViewer();
    updateCaption();
}

/**
 * Update the text in the window's title bar.
 */
void PortaBase::updateCaption()
{
    if (doc.isEmpty()) {
        setWindowTitle(qApp->applicationName());
    }
    else {
        QString name = QFileInfo(doc).fileName();
        setWindowTitle(name + "[*] - " + qApp->applicationName());
    }
}

/**
 * Close the data viewer and return to the "No file selected" label.
 */
void PortaBase::closeViewer()
{
    viewer->closeView();
    showFileSelector();
    doc = "";
    delete db;
    db = 0;
    updateCaption();
}

/**
 * Update the recent file buttons on the file selector screen.  Called
 * whenever QQMenuHelper updates the recent files menu (a new file is created,
 * a file is opened, the file selector mode is re-entered, or a recent file
 * is discovered to no longer exist).
 */
void PortaBase::updateRecentFileButtons()
{
    QStringList recentFiles;
    recentFiles << mh->action(QQMenuHelper::Recent1)->text();
    recentFiles << mh->action(QQMenuHelper::Recent2)->text();
    recentFiles << mh->action(QQMenuHelper::Recent3)->text();
    recentFiles << mh->action(QQMenuHelper::Recent4)->text();
    recentFiles << mh->action(QQMenuHelper::Recent5)->text();
    for (int i = 0; i < MAX_RECENT_FILES; i++) {
        QString path = recentFiles[i];
        QFileInfo info(path);
        recentButtons[i]->setText(info.baseName());
        recentButtons[i]->setToolTip(path);
        recentButtons[i]->setVisible(!path.isEmpty());
    }
    bool recentFilesExist = !recentButtons[0]->text().isEmpty();
    recentBox->setVisible(recentFilesExist);
#if defined(Q_WS_HILDON) || defined(Q_WS_MAEMO_5)
    newButton->setVisible(!recentFilesExist);
    openButton->setVisible(!recentFilesExist);
    importButton->setVisible(!recentFilesExist);
#endif
}

/**
 * Enter "select a file" mode, where there is no currently open database.
 */
void PortaBase::showFileSelector()
{
    findAction->setEnabled(false);
    rowViewAction->setEnabled(false);

    // Top-level menu visibility
    row->menuAction()->setVisible(false);
    view->menuAction()->setVisible(false);
    sort->menuAction()->setVisible(false);
    filter->menuAction()->setVisible(false);

    // File menu
#if !defined(Q_WS_HILDON) && !defined(Q_WS_MAEMO_5)
    fileSeparatorAction->setVisible(true);
#endif
    importAction->setVisible(true);
    changePassAction->setVisible(false);
    dataImportAction->setVisible(false);
    exportAction->setVisible(false);
    deleteRowsAction->setVisible(false);
    editColsAction->setVisible(false);
    manageEnumsAction->setVisible(false);
    slideshowAction->setVisible(false);
    propsAction->setVisible(false);
    printPreviewAction->setVisible(false);
    printPreviewAction->setEnabled(false);
    printAction->setVisible(false);
    printAction->setEnabled(false);
    mh->updateFileSelectorMenu();

    // Toolbar
    showAllFillerActions();
    rowAddAction->setVisible(false);
    rowEditAction->setVisible(false);
    rowDeleteAction->setVisible(false);
    rowCopyAction->setVisible(false);
    findAction->setVisible(false);
    viewsAction->setVisible(false);
    sortingsAction->setVisible(false);
    filtersAction->setVisible(false);

    mainStack->setCurrentWidget(noFileWidget);
}

/**
 * Enter "database open" mode.
 */
void PortaBase::showDataViewer()
{
    findAction->setEnabled(true);
    // rowViewAction status is handled by the data grid widget

    // Top-level menu visibility
    row->menuAction()->setVisible(true);
    view->menuAction()->setVisible(true);
    sort->menuAction()->setVisible(true);
    filter->menuAction()->setVisible(true);

    // File menu
    if (db->encryption()) {
        changePassAction->setVisible(true);
    }
    dataImportAction->setVisible(true);
    exportAction->setVisible(true);
    deleteRowsAction->setVisible(true);
    editColsAction->setVisible(true);
    manageEnumsAction->setVisible(true);
    slideshowAction->setVisible(true);
    propsAction->setVisible(true);
    fileSeparatorAction->setVisible(false);
    importAction->setVisible(false);
#if !defined(Q_WS_HILDON) && !defined(Q_WS_MAEMO_5)
    printPreviewAction->setEnabled(true);
    printPreviewAction->setVisible(true);
    printAction->setEnabled(true);
    printAction->setVisible(true);
#endif
    mh->updateDocumentFileMenu();

    // Toolbar
    rowAddAction->setVisible(true);
#if !defined(Q_WS_HILDON) && !defined(Q_WS_MAEMO_5)
    rowEditAction->setVisible(true);
    rowDeleteAction->setVisible(true);
    rowCopyAction->setVisible(true);
#endif
    viewsAction->setVisible(true);
    sortingsAction->setVisible(true);
    filtersAction->setVisible(true);
    findAction->setVisible(true);
    for (int i = 0; i < 6; i++) {
        fillerActions[i]->setVisible(false);
    }

    mainStack->setCurrentWidget(viewer);
    viewer->updateColWidths();
    rebuildViewMenu();
    rebuildSortMenu();
    rebuildFilterMenu();
}

/**
 * Window close event handler.  Makes sure that the user is given the
 * opportunity to save their changes if they forgot to do so.
 *
 * @param e The window close event
 */
void PortaBase::closeEvent(QCloseEvent *e)
{
    if (mainStack->currentWidget() == viewer) {
        if (isWindowModified()) {
            QMessageBox::StandardButton choice;
            choice = QMessageBox::warning(this, qApp->applicationName(),
                                          tr("Save changes?"),
                                          QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                          QMessageBox::Cancel);
            if (choice == QMessageBox::Yes) {
                save();
            }
            else if (choice == QMessageBox::Cancel) {
                e->ignore();
                return;
            }
            else {
                setEdited(false);
            }
        }
        e->ignore();
        closeViewer();
    }
    else {
        e->accept();
    }
}

/**
 * Quit the application.  Called when the "Quit" action is triggered.  Makes
 * sure that the user is given the opportunity to save their changes if they
 * forgot to do so.
 */
void PortaBase::quit()
{
    if (!doc.isEmpty()) {
        // close the current file; might be cancelled
        close();
    }
    if (doc.isEmpty()) {
        // if there isn't an open file, go ahead and exit
        close();
    }
}

/**
 * Delete the currently selected data row, if any.  If the setting to confirm
 * deletions is checked, a dialog will appear asking the user to confirm that
 * they really wanted to do that.  Called when the "Delete" item in the "Row"
 * menu is triggered.
 */
void PortaBase::deleteRow()
{
    if (confirmDeletions) {
        int choice = QMessageBox::warning(this, qApp->applicationName(),
                                          tr("Delete this row?"),
                                          QMessageBox::Yes|QMessageBox::No,
                                          QMessageBox::No);
        if (choice != QMessageBox::Yes) {
            return;
        }
    }
    viewer->deleteRow();
}

/**
 * Launch the row editor to create a new data row, but pre-filled with the
 * data from the currently selected row.
 */
void PortaBase::copyRow()
{
    viewer->editRow(-1, true);
}

/**
 * Delete all data rows that match the current filter.  If the setting to
 * confirm deletions is checked, a dialog will appear asking the user to
 * confirm that they really wanted to do that.  Called when the "Delete Rows
 * In Filter" menu item is triggered.
 */
void PortaBase::deleteAllRows()
{
    if (confirmDeletions) {
        int choice = QMessageBox::warning(this, qApp->applicationName(),
                                tr("Delete all rows in the\ncurrent filter?"),
                                QMessageBox::Yes|QMessageBox::No,
                                QMessageBox::No);
        if (choice != QMessageBox::Yes) {
            return;
        }
    }
    viewer->deleteAllRows();
}

/**
 * Commit all changes made to the database since the last time the file was
 * saved.
 */
void PortaBase::save()
{
    viewer->saveViewSettings();
    db->commit();
    setEdited(false);
}

/**
 * Change the password of an encrypted PortaBase file.  Called when the
 * "Change Password" menu item is triggered.
 */
void PortaBase::changePassword()
{
    PasswordDialog passdlg(db, PasswordDialog::ChangePassword, this);
    if (passdlg.exec()) {
        if (passdlg.validate()) {
            setEdited(true);
        }
    }
}

/**
 * Import rows of data from a CSV file.  Called when the "Import" menu item
 * is triggered while a database is open.
 */
void PortaBase::dataImport()
{
    ImportDialog dialog(ImportDialog::CSV, db, this);
    if (dialog.exec()) {
        viewer->resetTable();
        setEdited(true);
    }
}

/**
 * Export data from the open PortaBase file to another format.  The user will
 * be given the option to export either the rows in the current filter to a
 * CSV file or the entire database to an XML file.  Called when the "Export"
 * menu item is triggered.
 */
void PortaBase::dataExport()
{
    QStringList types;
    types.append(tr("CSV") + "(" + tr("rows in current filter") + ")");
    types.append(tr("XML"));
    bool ok = false;
    QString type = QInputDialog::getItem(this, MenuActions::tr("Export"),
                                         tr("Export to:"),
                                         types, 0, false, &ok);
    if (!ok) {
        return;
    }
    QString description = tr("Text files with comma separated values");
    QString extension = "csv";
    if (type == types[1]) {
        description = tr("XML files");
        extension = "xml";
    }
    QString output = createNewFile(description, extension);
    if (output.isEmpty()) {
        return;
    }
    if (extension == "csv") {
        viewer->exportToCSV(output);
    }
    else {
        viewer->exportToXML(output);
    }
}

/**
 * Launch a file dialog which allows the user to create a new file of the
 * specified type.  Basically just delegates the work to
 * <code>QQMenuHelper</code>, which serves as the central repository of data
 * on recently used files and directories.
 *
 * @param description The description of the file type as it is to appear
 *                    in the file dialog (null for the application default)
 * @param extension The file extension of the file type (do not include
 *                  the period, use null for the application default)
 * @return The path of the file to be created (or an empty string if none)
 */
QString PortaBase::createNewFile(const QString &description,
                                 const QString &extension)
{
  return mh->createNewFile(description, extension);
}

/**
 * Switch to the "All Columns" database view.  Called when that view is
 * selected from the "View" menu.
 */
void PortaBase::viewAllColumns()
{
    viewer->setView("_all");
    updateViewMenu();
    setEdited(true);
}

/**
 * Switch to the "All Rows" filter.  Called when that filter is selected
 * from the "Filter" menu.
 */
void PortaBase::viewAllRows()
{
    viewer->setFilter("_allrows");
    updateFilterMenu();
    setEdited(true);
}

/**
 * Launch the VSFManager dialog to manage the view selection.
 */
void PortaBase::changeView()
{
    vsfManager->setSubject(db, VSFManager::View);
    vsfManager->setActions(viewAddAction, viewEditAction, viewDeleteAction);
    vsfManager->exec();
}

/**
 * Switch to the database view whose entry in the "View" menu has the
 * specified ID.  Called when that menu item is triggered.
 *
 * @param action The menu action that was triggered to select this view
 */
void PortaBase::changeView(QAction *action)
{
    int index = viewActions.indexOf(action);
    if (index != -1) {
        changeView(viewNames[index]);
    }
}

/**
 * Switch to the named database view.  Used by menu selections and the
 * VSFManager dialog.
 *
 * @param name The name of the view to switch to
 */
void PortaBase::changeView(const QString &name)
{
    viewer->setView(name, true);
    updateViewMenu();
    // there might be a default sorting and/or filter...
    updateSortMenu();
    updateFilterMenu();
    setEdited(true);
}

/**
 * Launch the VSFManager dialog to manage the sorting selection.
 */
void PortaBase::changeSorting()
{
    vsfManager->setSubject(db, VSFManager::Sorting);
    vsfManager->setActions(sortAddAction, sortEditAction, sortDeleteAction);
    vsfManager->exec();
}

/**
 * Switch to the sorting whose entry in the "Sort" menu has the
 * specified ID.  Called when that menu item is triggered.
 *
 * @param action The menu action that was triggered to select this sorting
 */
void PortaBase::changeSorting(QAction *action)
{
    int index = sortActions.indexOf(action);
    if (index != -1) {
        changeSorting(sortNames[index]);
    }
}

/**
 * Switch to the named sorting.  Used by menu selections and the
 * VSFManager dialog.
 *
 * @param name The name of the sorting to switch to
 */
void PortaBase::changeSorting(const QString &name)
{
    viewer->setSorting(name);
    updateSortMenu();
    setEdited(true);
}

/**
 * Launch the VSFManager dialog to manage the filter selection.
 */
void PortaBase::changeFilter()
{
    vsfManager->setSubject(db, VSFManager::Filter);
    vsfManager->setActions(filterAddAction, filterEditAction, filterDeleteAction);
    vsfManager->exec();
}

/**
 * Switch to the filter whose entry in the "Filter" menu has the
 * specified ID.  Called when that menu item is triggered.
 *
 * @param action The menu action that was triggered to select this filter
 */
void PortaBase::changeFilter(QAction *action)
{
    int index = filterActions.indexOf(action);
    if (index != -1) {
        changeFilter(filterNames[index]);
    }
}

/**
 * Switch to the named filter.  Used by menu selections and the
 * VSFManager dialog.
 *
 * @param name The name of the filter to switch to
 */
void PortaBase::changeFilter(const QString &name)
{
    viewer->setFilter(name);
    updateFilterMenu();
    setEdited(true);
}

/**
 * Update the "View" menu to list the views currently defined in the open
 * database.
 */
void PortaBase::rebuildViewMenu()
{
    // remove old view names
    int count = viewActions.count();
    int i;
    for (i = 0; i < count; i++) {
        view->removeAction(viewActions[i]);
    }
    viewActions.clear();
    // add new view names
    viewNames = db->listViews();
    viewNames.removeAll("_all");
    count = viewNames.count();
    for (i = 0; i < count; i++) {
        QAction *action = view->addAction(viewNames[i]);
        action->setCheckable(true);
        viewActions.append(action);
    }
    updateViewMenu();
}

/**
 * Update the "Sort" menu to list the sortings currently defined in the
 * open database.
 */
void PortaBase::rebuildSortMenu()
{
    // remove old sorting names
    int count = sortActions.count();
    int i;
    for (i = 0; i < count; i++) {
        sort->removeAction(sortActions[i]);
    }
    sortActions.clear();
    // add new sorting names
    sortNames = db->listSortings();
    sortNames.removeAll("_single");
    count = sortNames.count();
    for (i = 0; i < count; i++) {
        QAction *action = sort->addAction(sortNames[i]);
        action->setCheckable(true);
        sortActions.append(action);
    }
    updateSortMenu();
}

/**
 * Update the "Filter" menu to list the filters currently defined in the
 * open database.
 */
void PortaBase::rebuildFilterMenu()
{
    // remove old filter names
    int count = filterActions.count();
    int i;
    for (i = 0; i < count; i++) {
        filter->removeAction(filterActions[i]);
    }
    filterActions.clear();
    // add new filter names
    filterNames = db->listFilters();
    filterNames.removeAll("_allrows");
    filterNames.removeAll("_simple");
    count = filterNames.count();
    for (i = 0; i < count; i++) {
        QAction *action = filter->addAction(filterNames[i]);
        action->setCheckable(true);
        filterActions.append(action);
    }
    updateFilterMenu();
}

/**
 * Update the "View" menu to indicate the currently selected database view.
 */
void PortaBase::updateViewMenu()
{
    QString viewName = db->currentView();
    if (viewName == "_all") {
        viewAllColsAction->setChecked(true);
        viewEditAction->setEnabled(false);
        viewDeleteAction->setEnabled(false);
    }
    else {
        viewAllColsAction->setChecked(false);
        viewEditAction->setEnabled(true);
        viewDeleteAction->setEnabled(true);
    }
    int count = viewNames.count();
    for (int i = 0; i < count; i++) {
        viewActions[i]->setChecked(viewName == viewNames[i]);
    }
}

/**
 * Update the "Sort" menu to indicate the currently selected sorting.
 */
void PortaBase::updateSortMenu()
{
    QString sortName = db->currentSorting();
    if (sortName.isEmpty() || sortName == "_single") {
        sortEditAction->setEnabled(false);
        sortDeleteAction->setEnabled(false);
    }
    else {
        sortEditAction->setEnabled(true);
        sortDeleteAction->setEnabled(true);
    }
    int count = sortNames.count();
    for (int i = 0; i < count; i++) {
        sortActions[i]->setChecked(sortName == sortNames[i]);
    }
}

/**
 * Update the "Filter" menu to indicate the currently selected filter.
 */
void PortaBase::updateFilterMenu()
{
    QString filterName = db->currentFilter();
    if (filterName == "_allrows") {
        filterAllRowsAction->setChecked(true);
        filterEditAction->setEnabled(false);
        filterDeleteAction->setEnabled(false);
    }
    else if (filterName == "_simple") {
        filterAllRowsAction->setChecked(false);
        filterEditAction->setEnabled(false);
        filterDeleteAction->setEnabled(false);
    }
    else {
        filterAllRowsAction->setChecked(false);
        filterEditAction->setEnabled(true);
        filterDeleteAction->setEnabled(true);
    }
    int count = filterNames.count();
    for (int i = 0; i < count; i++) {
        filterActions[i]->setChecked(filterName == filterNames[i]);
    }
}

/**
 * Launch the view editor to modify the currently selected view.  Called when
 * the "View" menu's "Edit" item is triggered.
 */
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
        viewer->setView(newName, true);
        // view menu is unchanged unless the view's name changed
        if (viewName != newName) {
            rebuildViewMenu();
        }
        setEdited(true);
    }
}

/**
 * Launch the sorting editor to modify the currently selected sorting.
 * Called when the "Sort" menu's "Edit" item is triggered.
 */
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
        setEdited(true);
    }
}

/**
 * Launch the filter editor to modify the currently selected filter.
 * Called when the "Filter" menu's "Edit" item is triggered.
 */
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
        setEdited(true);
    }
}

/**
 * Launch the view editor dialog to add a new view to the database.  Called
 * when the "View" menu's "Add" item is triggered.
 */
void PortaBase::addView()
{
    ViewEditor editor(this);
    QStringList empty;
    if (editor.edit(db, "", empty, "_none", "_none")) {
        editor.applyChanges();
        viewer->setView(editor.getName(), true);
        rebuildViewMenu();
        setEdited(true);
    }
}

/**
 * Launch the sorting editor dialog to add a new sorting to the database.
 * Called when the "Sorting" menu's "Add" item is triggered.
 */
void PortaBase::addSorting()
{
    SortEditor editor(this);
    if (editor.edit(db, "")) {
        editor.applyChanges();
        viewer->setSorting(editor.getName());
        rebuildSortMenu();
        setEdited(true);
    }
}

/**
 * Launch the filter editor dialog to add a new filter to the database.
 * Called when the "Filter" menu's "Add" item is triggered.
 */
void PortaBase::addFilter()
{
    FilterEditor editor(this);
    if (editor.edit(db, "")) {
        editor.applyChanges();
        viewer->setFilter(editor.getName());
        rebuildFilterMenu();
        setEdited(true);
    }
}

/**
 * Delete the currently selected view from the database and return to the all
 * columns view.  If the setting to confirm deletions is checked, a dialog
 * will appear asking the user to confirm that they really wanted to do that.
 * Called when the "View" menu's "Delete" item is triggered.
 */
void PortaBase::deleteView()
{
    if (confirmDeletions) {
        int choice = QMessageBox::warning(this, qApp->applicationName(),
                                          tr("Delete this view?"),
                                          QMessageBox::Yes|QMessageBox::No,
                                          QMessageBox::No);
        if (choice != QMessageBox::Yes) {
            return;
        }
    }
    viewer->closeView();
    db->deleteView(db->currentView());
    viewer->setView("_all");
    rebuildViewMenu();
    setEdited(true);
}

/**
 * Delete the currently selected sorting from the database and return to an
 * unsorted state.  If the setting to confirm deletions is checked, a dialog
 * will appear asking the user to confirm that they really wanted to do that.
 * Called when the "Sort" menu's "Delete" item is triggered.
 */
void PortaBase::deleteSorting()
{
    if (confirmDeletions) {
        int choice = QMessageBox::warning(this, qApp->applicationName(),
                                          tr("Delete this sorting?"),
                                          QMessageBox::Yes|QMessageBox::No,
                                          QMessageBox::No);
        if (choice != QMessageBox::Yes) {
            return;
        }
    }
    db->deleteSorting(db->currentSorting());
    viewer->setSorting("");
    rebuildSortMenu();
    setEdited(true);
}

/**
 * Delete the currently selected filter from the database and return to the
 * all rows filter.  If the setting to confirm deletions is checked, a dialog
 * will appear asking the user to confirm that they really wanted to do that.
 * Called when the "Filter" menu's "Delete" item is triggered.
 */
void PortaBase::deleteFilter()
{
    if (confirmDeletions) {
        int choice = QMessageBox::warning(this, qApp->applicationName(),
                                          tr("Delete this filter?"),
                                          QMessageBox::Yes|QMessageBox::No,
                                          QMessageBox::No);
        if (choice != QMessageBox::Yes) {
            return;
        }
    }
    db->deleteFilter(db->currentFilter());
    viewer->setFilter("_allrows");
    rebuildFilterMenu();
    setEdited(true);
}

/**
 * Launch a dialog which allows the user to perform a simple filter based
 * on a single column, without needing to hassle with the filter editor
 * dialog.  Called when the "Quick Filter" menu item or toolbar button is
 * triggered.
 */
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
        setEdited(true);
    }
    else {
        delete condition;
    }
}

/**
 * Set whether or not the open database has been modified since it was last
 * saved.  Makes appropriate changes to the title bar and the save action's
 * status.
 *
 * @param y True if the database has been modified, false otherwise
 */
void PortaBase::setEdited(bool y)
{
    mh->setEdited(y);
}

/**
 * Set whether or not a data row is currently selected.  Makes appropriate
 * changes to the status of the row-related actions (edit, copy, view, and
 * delete).
 *
 * @param y True if a row is currently selected, false otherwise
 */
void PortaBase::setRowSelected(bool y)
{
    rowEditAction->setEnabled(y);
    rowDeleteAction->setEnabled(y);
    rowCopyAction->setEnabled(y);
    rowViewAction->setEnabled(y);
}

/**
 * Drag event handler which determines if PortaBase allows a particular item
 * to be dragged over it (in preparation for a drop, usually).  Currently
 * accepts any URL drag events, but nothing else (text, images, etc.)  We'll
 * figure out if it's actually a PortaBase file or not when a drop is
 * attempted.  Note that drags aren't accepted if there is already an open
 * file; it has to be closed first.
 *
 * @param event A drag event
 */
void PortaBase::dragEnterEvent(QDragEnterEvent *event)
{
    if (doc.isEmpty() && event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

/**
 * Drop event handler which opens a dropped PortaBase file if there isn't
 * already an open database.
 *
 * @param event A drop event
 */
void PortaBase::dropEvent(QDropEvent *event)
{
    if (!doc.isEmpty()) {
        return;
    }
    if (event->mimeData()->hasUrls()) {
        QUrl url = event->mimeData()->urls()[0];
        QString file = url.toLocalFile();
        bool valid = true;
        if (file.isEmpty()) {
            valid = false;
        }
        else if (file.length() < 5) {
            valid = false;
        }
        else if (file.right(4) != ".pob") {
            valid = false;
        }
        if (!valid) {
            QMessageBox::warning(this, qApp->applicationName(),
                                 tr("Not a PortaBase file"));
            return;
        }
        event->acceptProposedAction();
        openFile(file);
    }
}

/**
 * Main event handler, overridden to support application icon click and drag
 * events.  Only useful on Mac OS X, where it allows copying or linking the
 * currently open file (with a drag) or viewing the path to the current file
 * (with a command-click).  Cribbed almost verbatim from Qt Quarterly:
 * http://doc.trolltech.com/qq/qq18-macfeatures.html
 *
 * @param event The event to be processed
 * @return True if the event was recognized, false otherwise
 */
bool PortaBase::event(QEvent *event)
{
#if defined(Q_WS_MAC)
    if (!isActiveWindow() || doc.isEmpty()) {
        return QMainWindow::event(event);
    }
    switch (event->type()) {
        case QEvent::IconDrag: {
            event->accept();
            Qt::KeyboardModifiers modifiers = qApp->keyboardModifiers();
            if (modifiers == Qt::NoModifier) {
                QDrag *drag = new QDrag(this);
                QMimeData *data = new QMimeData();
                data->setUrls(QList<QUrl>() << QUrl::fromLocalFile(doc));
                drag->setMimeData(data);
                QPixmap cursorPixmap(":/icons/document_large.png");
                drag->setPixmap(cursorPixmap);
                QPoint hotspot(cursorPixmap.width() - 5, 5);
                drag->setHotSpot(hotspot);
                drag->start(Qt::LinkAction | Qt::CopyAction);
            }
            else if (modifiers == Qt::ControlModifier) {
                QMenu menu(this);
                connect(&menu, SIGNAL(triggered(QAction *)), this, SLOT(openAt(QAction *)));

                QFileInfo info(doc);
                QAction *action = menu.addAction(info.fileName());
                action->setIcon(QIcon(":/icons/document_small.png"));

                QStringList folders = info.absolutePath().split('/');
                QStringListIterator it(folders);
                it.toBack();
                while (it.hasPrevious()) {
                    QString string = it.previous();
                    QIcon icon;
                    if (!string.isEmpty()) {
                        icon = style()->standardIcon(QStyle::SP_DirClosedIcon, 0, this);
                    }
                    else { // At the root
                        string = "/";
                        icon = style()->standardIcon(QStyle::SP_DriveHDIcon, 0, this);
                    }
                    action = menu.addAction(string);
                    action->setIcon(icon);
                }
                QPoint pos(QCursor::pos().x() - 20, frameGeometry().y());
                menu.exec(pos);
            }
            else {
                event->ignore();
            }
            return true;
        }
        default: {
            return QMainWindow::event(event);
        }
    }
#else
    return QMainWindow::event(event);
#endif
}

/**
 * Handler for parent directory selections from the application icon menu.
 * Only called on Mac OS X.
 *
 * @param action The application icon menu action that was triggered
 */
void PortaBase::openAt(QAction *action)
{
    QString path = doc.left(doc.indexOf(action->text())) + action->text();
    if (path == doc) {
        return;
    }
    QProcess process;
    process.start("/usr/bin/open", QStringList() << path, QIODevice::ReadOnly);
    process.waitForFinished();
}

/**
 * Load the main application settings, already set to read from the
 * top-level "portabase" group.  May be empty if this is the first time
 * running PortaBase in this environment.
 *
 * @return PortaBase's configurable application settings
 */
QSettings *PortaBase::getSettings()
{
    QSettings *settings = new QSettings();
    if (settings->contains("Font/Name")) {
        // No settings migration needed, just send it back as is
        return settings;
    }
    if (settings->contains("portabase/Font/Name")) {
        // remove old hierarchy layer which was needed for Qtopia support
        QStringList keys = settings->allKeys();
        int count = keys.count();
        for (int i = 0; i < count; i++) {
            QString key = keys[i];
            if (key.startsWith("portabase/")) {
                QString newKey = key.right(key.size() - 10);
                settings->setValue(newKey, settings->value(key));
            }
            else if (key.startsWith("qpe/")) {
                QString newKey = key.right(key.size() - 4);
                settings->setValue(newKey, settings->value(key));
            }
        }
        settings->remove("portabase");
        settings->remove("qpe");
    }
    if (!settings->contains("Font/Name")) {
        // new installation or upgrade from version using old preferences...
        // try migrating from original format
        OldConfig old("portabase");
        if (old.exists()) {
            old.migrate("Colors", *settings);
            old.migrate("Files", *settings);
            old.migrate("Font", *settings);
            old.migrate("General", *settings);
        }
        OldConfig oldQpe("qpe");
        if (oldQpe.exists()) {
            oldQpe.migrate("Date", *settings);
            oldQpe.migrate("Time", *settings);
        }
    }
    if (settings->contains("Date/Separator")) {
        // migrate old date and time settings to something simpler
        QString sep = settings->value("Date/Separator", "/").toString();
        int order = settings->value("Date/ShortOrder", 0x0111).toInt();
        QStringList parts;
        if (order == 0x0111) {
            parts << "d" << "M" << "yyyy";
        }
        else if (order == 0x010A) {
            parts << "M" << "d" << "yyyy";
        }
        else {
            parts << "yyyy" << "MM" << "dd";
        }
        settings->setValue("DateTime/ShortDateFormat", parts.join(sep));
        QString timeFormat("hh:mm");
        if (settings->value("General/ShowSeconds", false).toBool()) {
            timeFormat += ":ss";
        }
        if (settings->value("Time/AMPM", true).toBool()) {
            timeFormat += " AP";
        }
        settings->setValue("DateTime/TimeFormat", timeFormat);
        settings->remove("General/ShowSeconds");
        settings->remove("Date");
    }
    // Move last old "Time" group setting, if present
    if (settings->contains("Time/MONDAY")) {
        settings->setValue("DateTime/MONDAY", settings->value("Time/MONDAY"));
    }
    settings->remove("Time");
    // Make sure that updated settings are immediately available elsewhere
    settings->sync();
    return settings;
}

/**
 * Create the filler actions needed to prevent the toolbar ever shrinking
 * below the max number of showable actions on the Mac (since with the
 * unified toolbar, there doesn't seem to be a way to increase the number
 * of displayed buttons once it shrinks).
 */
void PortaBase::createFillerActions() {
    for (int i = 0; i < 6; i++) {
        fillerActions[i] = new QAction("", this);
#if defined(Q_WS_MAC)
        toolbar->addAction(fillerActions[i]);
#endif
    }
}

/**
 * Show all of the Mac toolbar filler actions; the ones not needed will be
 * hidden again later.
 */
void PortaBase::showAllFillerActions() {
#if defined(Q_WS_MAC)
    for (int i = 0; i < 6; i++) {
        fillerActions[i]->setVisible(true);
    }
#endif
}

/**
 * Show a print preview dialog.  In addition to showing what the printed
 * output will look like, this dialog also lets the user set up printing
 * options such as zoom factor, orientation, and paper size.  Configured
 * settings are preserved until PortaBase exits.
 */
void PortaBase::printPreview()
{
    if (!printer) {
        printer = new QPrinter(QPrinter::HighResolution);
    }
    QPrintPreviewDialog dialog(printer, this);
    connect(&dialog, SIGNAL(paintRequested(QPrinter*)),
            this, SLOT(print(QPrinter*)));
    dialog.exec();
}

/**
 * Print a subset of the current file's data that is appropriate for the current
 * settings.  Respects the current view, sorting, and filter (but prints all
 * matching rows, not just those shown on the current page).
 */
void PortaBase::print()
{
    if (!printer) {
        printer = new QPrinter(QPrinter::HighResolution);
    }
    QPrintDialog dialog(printer, this);
    if (dialog.exec() != QDialog::Accepted) {
        statusBar()->showMessage(tr("Printing aborted"), 2000);
        return;
    }
    statusBar()->showMessage(tr("Printing") + "...");
    print(printer);
    statusBar()->showMessage(tr("Printing completed"), 2000);
}

/**
 * Do the work of actually printing the file, after the page layout has been
 * set up.
 *
 * @param p The printer object to use
 */
void PortaBase::print(QPrinter *p)
{
    QTextDocument document;
    document.addResource(QTextDocument::ImageResource,
                         QUrl("icon://checked.png"),
                         QPixmap(":/icons/checked.png"));
    document.addResource(QTextDocument::ImageResource,
                         QUrl("icon://unchecked.png"),
                         QPixmap(":/icons/unchecked.png"));
    document.addResource(QTextDocument::ImageResource,
                         QUrl("icon://image.png"),
                         QPixmap(":/icons/image.png"));
    document.setHtml(viewer->toHtml());
    document.print(p);
}

/**
 * Launch a small dialog with information about PortaBase itself.  Called
 * when the "About PortaBase" menu item is triggered.
 */
void PortaBase::aboutPortaBase()
{
    QString appName = qApp->applicationName();
    QString text = appName + " 2.0b1\n\n" + tr("Copyright (C)")
                   + " 2002-2010\nJeremy Bowman\n\n"
                   + tr("Web site at http://portabase.sourceforge.net");
    QMessageBox::about(this, tr("About %1").arg(appName), text);
}

/**
 * Handler for Maemo D-Bus messages to open a specified file.
 */
void PortaBase::mime_open(const QString &url)
{
    QString path = QUrl(url).toLocalFile();
    if (!QFile::exists(path)) {
        QString message("%1:\n%2");
        message = message.arg(tr("No such file exists"));
        QMessageBox::warning(this, qApp->applicationName(),
                             message.arg(url));
        return;
    }
    openFile(path);
}

/**
 * Handler for Maemo D-Bus messages to become the currently-shown application.
 */
int PortaBase::top_application()
{
    activateWindow();
    return 0;
}
