/*
 * rowviewer.cpp
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
#else
#include <qpe/resource.h>
#endif

#include <qhbox.h>
#include <qpushbutton.h>
#include <qtextview.h>
#include "datatypes.h"
#include "rowviewer.h"
#include "view.h"
#include "viewdisplay.h"

RowViewer::RowViewer(ViewDisplay *parent, const char *name, WFlags f)
  : PBDialog(tr("Row Viewer"), parent, name, f), display(parent)
{
    tv = new QTextView(this);
    vbox->addWidget(tv);
    QHBox *hbox = new QHBox(this);
    vbox->addWidget(hbox);
    prevButton = new QPushButton(hbox);
    prevButton->setPixmap(Resource::loadPixmap("back"));
    connect(prevButton, SIGNAL(clicked()), this, SLOT(previousRow()));
    QPushButton *editButton = new QPushButton(hbox);
    editButton->setPixmap(Resource::loadPixmap("edit"));
    connect(editButton, SIGNAL(clicked()), this, SLOT(editRow()));
    nextButton = new QPushButton(hbox);
    nextButton->setPixmap(Resource::loadPixmap("forward"));
    connect(nextButton, SIGNAL(clicked()), this, SLOT(nextRow()));

    finishLayout(TRUE, FALSE);
    editButton->setFocus();
}

RowViewer::~RowViewer()
{

}

void RowViewer::viewRow(View *currentView, int rowIndex)
{
    view = currentView;
    index = rowIndex;
    rowCount = view->getRowCount();
    updateContent();
    exec();
}

void RowViewer::updateContent()
{
    prevButton->setEnabled(index != 0);
    nextButton->setEnabled(index != rowCount - 1);
    QStringList values = view->getRow(index);
    QString str = "<qt><table cellspacing=0>";
    QStringList colNames = view->getColNames();
    int *colTypes = view->getColTypes();
    int count = colNames.count();
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
