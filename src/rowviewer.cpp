/*
 * rowviewer.cpp
 *
 * (c) 2002-2004,2010-2012,2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
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
#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QClipboard>
#include <QIcon>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPalette>
#include <QTextDocument>
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
#include "pbnetworkaccessmanager.h"
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
    hd = Factory::htmlDisplay(this);
    vbox->addWidget(hd);

    QHBoxLayout *hbox = Factory::hBoxLayout(vbox);
    prevButton = Factory::button(this);
    prevButton->setIcon(Factory::icon("back"));
    prevButton->setToolTip(tr("Previous row"));
    connect(prevButton, SIGNAL(clicked()), this, SLOT(previousRow()));
    hbox->addWidget(prevButton);

#if defined(Q_OS_ANDROID)
    QAction *editAction = new QAction(Factory::icon("edit"), tr("Edit this row"), this);
    connect(editAction, SIGNAL(triggered()), this, SLOT(editRow()));
    addButton(editAction, 0);

    QAction *copyAction = new QAction(Factory::icon("copy_row"), tr("Copy this row"), this);
    connect(copyAction, SIGNAL(triggered()), this, SLOT(copyRow()));
    addButton(copyAction, 0);

    QAction *deleteAction = new QAction(Factory::icon("delete"), tr("Delete this row"), this);
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteRow()));
    addButton(deleteAction, 0);

    // Native text copying from a read-only QTextEdit doesn't work yet
    QAction *copyTextAction = new QAction(Factory::icon("copy_text"), tr("Copy the selected text"), this);
    connect(copyTextAction, SIGNAL(triggered()), hd, SLOT(copy()));
    addButton(copyTextAction, 0);
#else
    QAbstractButton *editButton = Factory::button(this);
    editButton->setIcon(Factory::icon("edit"));
    editButton->setToolTip(tr("Edit this row"));
    connect(editButton, SIGNAL(clicked()), this, SLOT(editRow()));
    hbox->addWidget(editButton);

    QAbstractButton *copyButton = Factory::button(this);
    copyButton->setIcon(Factory::icon("copy_row"));
    copyButton->setToolTip(tr("Copy this row"));
    connect(copyButton, SIGNAL(clicked()), this, SLOT(copyRow()));
    hbox->addWidget(copyButton);

    QAbstractButton *deleteButton = Factory::button(this);
    deleteButton->setIcon(Factory::icon("delete"));
    deleteButton->setToolTip(tr("Delete this row"));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteRow()));
    hbox->addWidget(deleteButton);
#endif

    QStringList viewNames = db->listViews();
    viewNames.removeAll("_all");
    viewNames.prepend(MenuActions::tr("All Columns"));
    viewBox = Factory::comboBox(this);
    viewBox->addItems(viewNames);
    connect(viewBox, SIGNAL(activated(int)), this, SLOT(viewChanged(int)));
    hbox->addWidget(viewBox, 1);

#if defined(Q_WS_MAEMO_5)
    PBNetworkAccessManager *manager = new PBNetworkAccessManager(db);
    hd->page()->setNetworkAccessManager(manager);
#else
    // Make the boolean value images available in case we need them
    QCheckBox evenCheckBox;
    QPalette evenPalette = evenCheckBox.palette();
    evenPalette.setColor(QPalette::Window, Factory::evenRowColor);
    evenCheckBox.setPalette(evenPalette);
    evenCheckBox.setChecked(true);
    QPixmap checked_even(evenCheckBox.sizeHint());
    evenCheckBox.render(&checked_even);
    evenCheckBox.setChecked(false);
    QPixmap unchecked_even(evenCheckBox.sizeHint());
    evenCheckBox.render(&unchecked_even);

    QColor altColor = alphaBlend(Factory::evenRowColor, Factory::oddRowColor);
    QCheckBox oddCheckBox;
    QPalette oddPalette = oddCheckBox.palette();
    oddPalette.setColor(QPalette::Window, altColor);
    oddCheckBox.setPalette(oddPalette);
    oddCheckBox.setChecked(true);
    QPixmap checked_odd(oddCheckBox.sizeHint());
    oddCheckBox.render(&checked_odd);
    oddCheckBox.setChecked(false);
    QPixmap unchecked_odd(oddCheckBox.sizeHint());
    oddCheckBox.render(&unchecked_odd);

#if !defined(Q_OS_ANDROID)
    // kinetic scrolling takes precedence over text copy on Fremantle
    // Android puts this button in the action bar, done above
    QAbstractButton *copyTextButton = Factory::button(this);
    copyTextButton->setIcon(Factory::icon("copy_text"));
    copyTextButton->setToolTip(tr("Copy the selected text"));
    connect(copyTextButton, SIGNAL(clicked()), hd, SLOT(copy()));
    hbox->addWidget(copyTextButton);
#endif
    hd->document()->addResource(QTextDocument::ImageResource,
                                QUrl("qrc:/icons/checked_even.png"),
                                checked_even);
    hd->document()->addResource(QTextDocument::ImageResource,
                                QUrl("qrc:/icons/unchecked_even.png"),
                                unchecked_even);
    hd->document()->addResource(QTextDocument::ImageResource,
                                QUrl("qrc:/icons/checked_odd.png"),
                                checked_odd);
    hd->document()->addResource(QTextDocument::ImageResource,
                                QUrl("qrc:/icons/unchecked_odd.png"),
                                unchecked_odd);
#endif

    nextButton = Factory::button(this);
    nextButton->setIcon(Factory::icon("forward"));
    nextButton->setToolTip(tr("Next row"));
    connect(nextButton, SIGNAL(clicked()), this, SLOT(nextRow()));
    hbox->addWidget(nextButton);

    finishLayout(true, false);
    hd->setFocus();
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
#if defined(Q_WS_MAEMO_5)
    QStringList allColNames = db->listColumns();
#endif
    int *colTypes = currentView->getColTypes();
    QString str = "<html><body><table cellspacing=0>";
    QString evenRow = QString("<tr bgcolor=\"%1\">").arg(Factory::evenRowColor.name());
    QColor oddColor = alphaBlend(Factory::evenRowColor, Factory::oddRowColor);
    QString oddRow = QString("<tr bgcolor=\"%1\">").arg(oddColor.name());
    int count = colNames.count();
    int imageIndex = 0;
    QString rowType;
    for (int i = 0; i < count; i++) {
        if (i % 2 == 0) {
            str += evenRow;
            rowType = "_even";
        }
        else {
            str += oddRow;
            rowType = "_odd";
        }
#if defined(Q_WS_MAEMO_5)
        rowType = "";
#endif
        str += "<td><b>";
        str += prepareString(colNames[i]);
        str += ": </b></td><td valign=\"middle\">";
        int type = colTypes[i];
        if (type == BOOLEAN) {
            if (values[i].toInt()) {
                str += QString("<img src=\"qrc:/icons/checked%1.png\">").arg(rowType);
            }
            else {
                str += QString("<img src=\"qrc:/icons/unchecked%1.png\">").arg(rowType);
            }
        }
        else if (type == IMAGE) {
            QString format = values[i];
            if (!format.isEmpty()) {
                int rowId = currentView->getId(index);
                QString name = colNames[i];
#if defined(Q_WS_MAEMO_5)
                int colIndex = allColNames.indexOf(name);
                QString imageId = QString("img://%1_%2_%3").arg(rowId).arg(colIndex).arg(format);
#else
                QImage image = ImageUtils::load(db, rowId, name, format);
                QString imageId = QString("img://image%1").arg(imageIndex);
                hd->document()->addResource(QTextDocument::ImageResource,
                                            QUrl(imageId), image);
#endif
                str += QString("<img src=\"%1\">").arg(imageId);
                imageIndex++;
            }
        }
        else {
            str += prepareString(values[i]);
        }
        str += "</td></tr>";
    }
    str += "</table></body></html>";
    hd->setHtml(str);
    hd->setFocus();
}

/**
 * Combine the given background and foreground colors, taking the foreground's
 * alpha channel into account, to yield the resulting color without alpha.
 */
QColor RowViewer::alphaBlend(QColor base, QColor foreground)
{
    int alpha = foreground.alpha();
    if (alpha == 0) {
        return foreground;
    }
    int complement = 255 - alpha;
    int red = (base.red() * complement + foreground.red() * alpha) / 255;
    int green = (base.green() * complement + foreground.green() * alpha) / 255;
    int blue = (base.blue() * complement + foreground.blue() * alpha) / 255;
    return QColor(red, green, blue);
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
        hd->setFocus();
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
        hd->setFocus();
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
    int rowId = view->getId(index);
    display->deleteRow(rowId);
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
