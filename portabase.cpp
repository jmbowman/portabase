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

#include <qpe/filemanager.h>
#include <qpe/fileselector.h>
#include <qpe/qpemenubar.h>
#include <qpe/qpetoolbar.h>
#include <qpe/resource.h>

#include <qaction.h>
#include <qinputdialog.h>
#include <qmainwindow.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qwidgetstack.h>
#include "database.h"
#include "dbeditor.h"
#include "importdialog.h"
#include "inputdialog.h"
#include "portabase.h"
#include "viewdisplay.h"
#include "vieweditor.h"

PortaBase::PortaBase(QWidget *parent, const char *name, WFlags f)
    : QMainWindow(parent, name, f), db(0), doc(0), isEdited(FALSE)
{
    doc = 0;
    setToolBarsMovable(FALSE);

    QPEToolBar *bar = new QPEToolBar(this);
    bar->setHorizontalStretchable(TRUE);
    menu = bar;

    QPEMenuBar *mb = new QPEMenuBar(bar);
    QPopupMenu *file = new QPopupMenu(this);
    QPopupMenu *row = new QPopupMenu(this);
    view = new QPopupMenu(this);
    view->setCheckable(TRUE);

    toolbar = new QPEToolBar(this);

    fileSaveAction = new QAction(tr("Save"),
                                 Resource::loadPixmap("portabase/save"),
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

    act = new QAction(tr("Edit Columns"), QString::null, 0, this, 0);
    connect(act, SIGNAL(activated()), this, SLOT(editColumns()));
    act->addTo(file);

    QIconSet addIcons = Resource::loadIconSet("new");
    act = new QAction(tr("Add"), addIcons, QString::null, 0, this, 0);
    connect(act, SIGNAL(activated()), this, SLOT(addRow()));
    act->addTo(toolbar);
    act->addTo(row);

    QIconSet editIcons = Resource::loadIconSet("edit");
    rowEditAction = new QAction(tr("Edit"), editIcons, QString::null, 0,
                                this, 0);
    connect(rowEditAction, SIGNAL(activated()), this, SLOT(editRow()));
    rowEditAction->addTo(toolbar);
    rowEditAction->addTo(row);

    QIconSet deleteIcons = Resource::loadIconSet("trash");
    rowDeleteAction = new QAction(tr("Delete"), deleteIcons, QString::null,
                                  0, this, 0);
    connect(rowDeleteAction, SIGNAL(activated()), this, SLOT(deleteRow()));
    rowDeleteAction->addTo(toolbar);
    rowDeleteAction->addTo(row);

    view->insertItem(addIcons, tr("Add"), this, SLOT(addView()));
    editViewId = view->insertItem(editIcons, tr("Edit"),
                                  this, SLOT(editView()));
    deleteViewId = view->insertItem(deleteIcons, tr("Delete"),
                                    this, SLOT(deleteView()));
    view->insertSeparator();
    allColumnsViewId = view->insertItem(tr("All Columns"), this,
                                        SLOT(viewAllColumns()));
    connect(view, SIGNAL(activated(int)), this, SLOT(changeView(int)));

    mb->insertItem(tr("File"), file);
    mb->insertItem(tr("Row"), row);
    mb->insertItem(tr("View"), view);
                                
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
        }
        else {
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
    viewer->deleteRow();
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

void PortaBase::changeView(int id)
{
    int index = viewIds.findIndex(id);
    if (index != -1) {
        viewer->setView(viewNames[index]);
        updateViewMenu();
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

void PortaBase::deleteView()
{
    viewer->closeView();
    db->deleteView(db->currentView());
    viewer->setView("_all");
    rebuildViewMenu();
    updateViewMenu();
    setEdited(TRUE);
}

void PortaBase::setEdited(bool y)
{
    isEdited = y;
    // current icon looks like crap disabled...need a better one
    //fileSaveAction->setEnabled(y);
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
