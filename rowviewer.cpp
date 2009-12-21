/*
 * rowviewer.cpp
 *
 * (c) 2002-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qapplication.h>
#include <qcombobox.h>
#include <qclipboard.h>
#include <qhbox.h>
#include <qmime.h>
#include <qpushbutton.h>
#include <qtextview.h>
#include <qtoolbutton.h>
#include "database.h"
#include "datatypes.h"
#include "menuactions.h"
#include "portabase.h"
#include "rowviewer.h"
#include "view.h"
#include "viewdisplay.h"
#include "image/imageutils.h"

#if defined(Q_WS_QWS)
#include <qpe/resource.h>
#else
#include "desktop/resource.h"
#endif

RowViewer::RowViewer(Database *dbase, ViewDisplay *parent, const char *name)
  : PBDialog(tr("Row Viewer"), parent, name), db(dbase), display(parent), currentView(0)
{
    tv = new QTextView(this);
    vbox->addWidget(tv);
    QHBox *hbox = new QHBox(this);
    vbox->addWidget(hbox);
#if defined(Q_OS_MACX)
    prevButton = new QToolButton(hbox);
#else
    prevButton = new QPushButton(hbox);
#endif
    prevButton->setPixmap(Resource::loadPixmap("back"));
    connect(prevButton, SIGNAL(clicked()), this, SLOT(previousRow()));
#if defined(Q_OS_MACX)
    QToolButton *editButton = new QToolButton(hbox);
#else
    QPushButton *editButton = new QPushButton(hbox);
#endif
    editButton->setPixmap(Resource::loadPixmap("edit"));
    connect(editButton, SIGNAL(clicked()), this, SLOT(editRow()));

    QStringList viewNames = db->listViews();
    viewNames.remove("_all");
    viewNames.prepend(MenuActions::tr("All Columns"));
    viewBox = new QComboBox(FALSE, hbox);
    hbox->setStretchFactor(viewBox, 1);
    viewBox->insertStringList(viewNames);
    connect(viewBox, SIGNAL(activated(int)), this, SLOT(viewChanged(int)));

#if defined(Q_OS_MACX)
    QToolButton *copyButton = new QToolButton(hbox);
#else
    QPushButton *copyButton = new QPushButton(hbox);
#endif
    copyButton->setPixmap(Resource::loadPixmap("copy"));
    connect(copyButton, SIGNAL(clicked()), this, SLOT(copyText()));
#if defined(Q_OS_MACX)
    nextButton = new QToolButton(hbox);
#else
    nextButton = new QPushButton(hbox);
#endif
    nextButton->setPixmap(Resource::loadPixmap("forward"));
    connect(nextButton, SIGNAL(clicked()), this, SLOT(nextRow()));

    finishLayout(TRUE, FALSE);
    editButton->setFocus();
}

RowViewer::~RowViewer()
{
    if (currentView) {
        delete currentView;
    }
    // free up memory used by images that were shown
    int count = usedImageIds.count();
    for (int i = 0; i < count; i++) {
        tv->mimeSourceFactory()->setImage(usedImageIds[i], QImage());
    }
}

void RowViewer::viewRow(View *originalView, int rowIndex)
{
    view = originalView;
    if (currentView) {
        delete currentView;
    }
    QString viewName = view->getName();
    currentView = db->getView(viewName, FALSE, FALSE);
    currentView->copyStateFrom(view);
    if (viewName == "_all") {
        viewBox->setCurrentItem(0);
    }
    else {
        int count = viewBox->count();
        for (int i = 1; i < count; i++) {
            if (viewBox->text(i) == viewName) {
                viewBox->setCurrentItem(i);
                break;
            }
        }
    }
    index = rowIndex;
    rowCount = view->getRowCount();
    updateContent();
    exec();
}

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
        str += "<tr><td><font color=#0000ff>";
        str += prepareString(colNames[i]);
        str += ":</font></td><td>";
        int type = colTypes[i];
        if (type == BOOLEAN) {
            if (values[i].toInt()) {
                QString path = Resource::findPixmap("portabase/checked");
                str += "<img src=\"" + path + "\">";
            }
            else {
                QString path = Resource::findPixmap("portabase/unchecked");
                str += "<img src=\"" + path + "\">";
            }
        }
        else if (type == IMAGE) {
            QString format = values[i];
            if (format != "") {
                int rowId = currentView->getId(index);
                QString name = colNames[i];
                QImage image = ImageUtils::load(db, rowId, name, format);
                QString imageId = QString("image%1").arg(imageIndex);
                tv->mimeSourceFactory()->setImage(imageId, image);
                str += "<img src=\"" + imageId + "\">";
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
    tv->setText(str);
}

QString RowViewer::prepareString(QString content)
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

void RowViewer::nextRow()
{
    index++;
    updateContent();
}

void RowViewer::previousRow()
{
    index--;
    updateContent();
}

void RowViewer::editRow()
{
    int rowId = view->getId(index);
    if (display->editRow(rowId)) {
        accept();
    }
}

void RowViewer::copyText()
{
    if (tv->hasSelectedText()) {
        QClipboard *cb = QApplication::clipboard();
        cb->setText(tv->selectedText());
    }
}

void RowViewer::viewChanged(int index)
{
    if (currentView) {
        delete currentView;
    }
    QString name = viewBox->text(index);
    if (index == 0) {
        name = "_all";
    }
    currentView = db->getView(name, FALSE, FALSE);
    currentView->copyStateFrom(view);
    updateContent();
}

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
    else {
        e->ignore();
    }
}
