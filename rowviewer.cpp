/*
 * rowviewer.cpp
 *
 * (c) 2002-2004,2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file rowviewer.cpp
 * Source file for RowViewer
 */

#include <QAbstractButton>
#include <QApplication>
#include <QComboBox>
#include <QClipboard>
#include <QIcon>
#include <QKeyEvent>
#include <QMessageBox>
#include <QTextDocument>
#include <QTextEdit>
#include <QUrl>
#include "database.h"
#include "datatypes.h"
#include "factory.h"
#include "formatting.h"
#include "menuactions.h"
#include "portabase.h"
#include "rowviewer.h"
#include "view.h"
#include "viewdisplay.h"
#include "image/imageutils.h"

#if defined(Q_WS_MAEMO_5)
#include <QAbstractKineticScroller>
#endif

/**
 * Constructor.
 *
 * @param dbase The database being viewed
 * @param parent The view display widget which is this dialog's parent
 */
RowViewer::RowViewer(Database *dbase, ViewDisplay *parent)
  : PBDialog(tr("Row Viewer"), parent), db(dbase), display(parent), currentView(0)
{
    tv = new QTextEdit(this);
    tv->setReadOnly(true);
    // Make the boolean value images available in case we need them
    tv->document()->addResource(QTextDocument::ImageResource,
                                QUrl("img://icons/checked.png"),
                                QPixmap(":/icons/checked.png"));
    tv->document()->addResource(QTextDocument::ImageResource,
                                QUrl("img://icons/unchecked.png"),
                                QPixmap(":/icons/unchecked.png"));
#if defined(Q_WS_MAEMO_5)
    QVariant ksProp = tv->property("kineticScroller");
    QAbstractKineticScroller *ks = ksProp.value<QAbstractKineticScroller *>();
    if (ks) {
        ks->setEnabled(true);
    }
#endif
    vbox->addWidget(tv);

    QHBoxLayout *hbox = Factory::hBoxLayout(vbox);
    prevButton = Factory::button(this);
    prevButton->setIcon(QIcon(":/icons/back.png"));
    prevButton->setToolTip(tr("Previous row"));
    connect(prevButton, SIGNAL(clicked()), this, SLOT(previousRow()));
    hbox->addWidget(prevButton);

    QAbstractButton *editButton = Factory::button(this);
    editButton->setIcon(QIcon(":/icons/edit.png"));
    editButton->setToolTip(tr("Edit this row"));
    connect(editButton, SIGNAL(clicked()), this, SLOT(editRow()));
    hbox->addWidget(editButton);

    QAbstractButton *copyButton = Factory::button(this);
    copyButton->setIcon(QIcon(":/icons/copy_row.png"));
    copyButton->setToolTip(tr("Copy this row"));
    connect(copyButton, SIGNAL(clicked()), this, SLOT(copyRow()));
    hbox->addWidget(copyButton);

    QAbstractButton *deleteButton = Factory::button(this);
    deleteButton->setIcon(QIcon(":/icons/delete.png"));
    deleteButton->setToolTip(tr("Delete this row"));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteRow()));
    hbox->addWidget(deleteButton);

    QStringList viewNames = db->listViews();
    viewNames.removeAll("_all");
    viewNames.prepend(MenuActions::tr("All Columns"));
    viewBox = new QComboBox(this);
    viewBox->addItems(viewNames);
    connect(viewBox, SIGNAL(activated(int)), this, SLOT(viewChanged(int)));
    hbox->addWidget(viewBox, 1);

#if !defined(Q_WS_MAEMO_5)
    // kinetic scrolling takes precedence over text copy on Fremantle
    QAbstractButton *copyTextButton = Factory::button(this);
    copyTextButton->setIcon(QIcon(":/icons/copy_text.png"));
    copyTextButton->setToolTip(tr("Copy the selected text"));
    connect(copyTextButton, SIGNAL(clicked()), tv, SLOT(copy()));
    hbox->addWidget(copyTextButton);
#endif

    nextButton = Factory::button(this);
    nextButton->setIcon(QIcon(":/icons/forward.png"));
    nextButton->setToolTip(tr("Next row"));
    connect(nextButton, SIGNAL(clicked()), this, SLOT(nextRow()));
    hbox->addWidget(nextButton);

    finishLayout(true, false);
    tv->setFocus();
}

/**
 * Destructor.
 */
RowViewer::~RowViewer()
{
    if (currentView) {
        delete currentView;
    }
}

/**
 * Launch this dialog to view a particular record.
 *
 * @param originalView The currently selected view
 * @param rowIndex Index within the current filter of the record to view
 */
void RowViewer::viewRow(View *originalView, int rowIndex)
{
    view = originalView;
    if (currentView) {
        delete currentView;
    }
    QString viewName = view->getName();
    currentView = db->getView(viewName, false, false);
    currentView->copyStateFrom(view);
    if (viewName == "_all") {
        viewBox->setCurrentIndex(0);
    }
    else {
        int count = viewBox->count();
        for (int i = 1; i < count; i++) {
            if (viewBox->itemText(i) == viewName) {
                viewBox->setCurrentIndex(i);
                break;
            }
        }
    }
    index = rowIndex;
    rowCount = view->totalRowCount();
    updateContent();
    exec();
}

/**
 * Update the display based on which record is now to be shown, etc.
 */
void RowViewer::updateContent()
{
    prevButton->setEnabled(index != 0);
    nextButton->setEnabled(index != rowCount - 1);
    QStringList values = currentView->getRow(index);
    QStringList colNames = currentView->getColNames();
    int *colTypes = currentView->getColTypes();
    QString str = "<qt><table cellspacing=0>";
    int count = colNames.count();
    int imageIndex = 0;
    for (int i = 0; i < count; i++) {
        if (i % 2 == 0) {
            str += "<tr>";
        }
        else {
            str += "<tr bgcolor=\"lightgray\">";
        }
        str += "<td><font color=#0000ff>";
        str += prepareString(colNames[i]);
        str += ": </font></td><td valign=\"middle\">";
        int type = colTypes[i];
        if (type == BOOLEAN) {
            if (values[i].toInt()) {
                str += "<img src=\"img://icons/checked.png\">";
            }
            else {
                str += "<img src=\"img://icons/unchecked.png\">";
            }
        }
        else if (type == IMAGE) {
            QString format = values[i];
            if (!format.isEmpty()) {
                int rowId = currentView->getId(index);
                QString name = colNames[i];
                QImage image = ImageUtils::load(db, rowId, name, format);
                QString imageId = QString("img://image%1").arg(imageIndex);
                tv->document()->addResource(QTextDocument::ImageResource,
                                            QUrl(imageId), image);
                str += QString("<img src=\"%1\">").arg(imageId);
                if (!usedImageIds.contains(imageId)) {
                    usedImageIds.append(imageId);
                }
                imageIndex++;
            }
        }
        else {
            str += prepareString(values[i]);
        }
        str += "</td></tr>";
    }
    str += "</table></qt>";
    tv->setHtml(str);
    tv->setFocus();
}

/**
 * Format the provided field value for insertion into an HTML table cell.
 *
 * @param content The field value from the database
 * @return An HTML fragment suitable for use in the display widget
 */
QString RowViewer::prepareString(const QString &content)
{
    QString result = "";
    int length = content.length();
    for (int i = 0; i < length; i++) {
        const QChar c = content[i];
        if (c == '\n') {
            result += "<br>";
        }
        else if (c == '<') {
            result += "&lt;";
        }
        else if (c == '>') {
            result += "&gt;";
        }
        else if (c == '&') {
            result += "&amp;";
        }
        else {
            result += c;
        }
    }
    return result;
}

/**
 * Display the next record in the current filter.  Called when the next record
 * button is clicked.
 */
void RowViewer::nextRow()
{
    index++;
    updateContent();
}

/**
 * Display the previous record in the current filter.  Called when the
 * previous record button is clicked.
 */
void RowViewer::previousRow()
{
    index--;
    updateContent();
}

/**
 * Launch the row editor to edit the currently displayed record.
 */
void RowViewer::editRow()
{
    int rowId = view->getId(index);
    if (display->editRow(rowId, false, this)) {
        accept();
    }
    else {
        tv->setFocus();
    }
}

/**
 * Launch the row editor dialog with a copy of the currently displayed record.
 */
void RowViewer::copyRow()
{
    int rowId = view->getId(index);
    if (display->editRow(rowId, true, this)) {
        accept();
    }
    else {
        tv->setFocus();
    }
}

/**
 * Delete the currently displayed record and return to the data viwer.
 */
void RowViewer::deleteRow()
{
    QSettings settings;
    bool confirmDeletions = settings.value("General/ConfirmDeletions",
                                           true).toBool();
    if (confirmDeletions) {
        int choice = QMessageBox::warning(this, qApp->applicationName(),
                                          PortaBase::tr("Delete this row?"),
                                          QMessageBox::Yes|QMessageBox::No,
                                          QMessageBox::No);
        if (choice != QMessageBox::Yes) {
            return;
        }
    }
    display->deleteRow();
    accept();
}

/**
 * Update the data display to reflect a change in the selected database view.
 *
 * @param index The index of the newly selected view in the selection list
 */
void RowViewer::viewChanged(int index)
{
    if (currentView) {
        delete currentView;
    }
    QString name = viewBox->itemText(index);
    if (index == 0) {
        name = "_all";
    }
    currentView = db->getView(name, false, false);
    currentView->copyStateFrom(view);
    updateContent();
}

/**
 * Handler for keyboard key release events.  Ensures that pressing the left
 * and right arrows navigate to the previous and next records in the filter,
 * respectively.
 *
 * @param e The keyboard event which occurred
 */
void RowViewer::keyReleaseEvent(QKeyEvent *e)
{
    int key = e->key();
    if (key == Qt::Key_Left) {
        if (index != 0) {
            previousRow();
        }
    }
    else if (key == Qt::Key_Right) {
        if (index != rowCount - 1) {
            nextRow();
        }
    }
    else if (key == Qt::Key_Space) {
        editRow();
    }
    else {
        e->ignore();
    }
}
