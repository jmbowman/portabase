/*
 * calcdateeditor.cpp
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#if defined(DESKTOP)
#include <qlabel.h>
#include <qpushbutton.h>
#include "../desktop/resource.h"
#endif

#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qdatetime.h>
#include <qgrid.h>
#include <qhbox.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include "calcnode.h"
#include "calcdateeditor.h"
#include "../datatypes.h"
#include "../datewidget.h"

CalcDateEditor::CalcDateEditor(const QStringList &colNames, int *colTypes, QWidget *parent, const char *name, WFlags f)
  : QDialog(parent, name, TRUE, f)
{
    setCaption(tr("Calculation Node Editor") + " - " + tr("PortaBase"));
    QVBoxLayout *vbox = new QVBoxLayout(this);
#if defined(Q_WS_WIN)
    setSizeGripEnabled(TRUE);
    vbox->setMargin(8);
    vbox->addWidget(new QLabel("<center><b>" + tr("Calculation Node Editor")
                               + "</b></center>", this));
#endif
    group = new QButtonGroup(this);
    group->hide();
    QGrid *grid = new QGrid(2, this);
    vbox->addWidget(grid);
    QRadioButton *colButton = new QRadioButton(tr("Column"), grid);
    group->insert(colButton, 0);
    columnList = new QComboBox(FALSE, grid);
    int count = colNames.count();
    int i;
    for (i = 0; i < count; i++) {
        int type = colTypes[i];
        if (type == DATE) {
            columnList->insertItem(colNames[i]);
        }
    }

    group->insert(new QRadioButton(tr("Constant"), grid), 1);
    dateWidget = new DateWidget(grid);

    if (columnList->count() > 0) {
        group->setButton(0);
    }
    else {
        // no date columns, so can't select one
        group->setButton(1);
        colButton->setEnabled(FALSE);
    }

#if defined(DESKTOP)
    vbox->addWidget(new QLabel(" ", this));
    QHBox *hbox = new QHBox(this);
    vbox->addWidget(hbox);
    new QWidget(hbox);
    QPushButton *okButton = new QPushButton(tr("OK"), hbox);
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    new QWidget(hbox);
    QPushButton *cancelButton = new QPushButton(tr("Cancel"), hbox);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    new QWidget(hbox);
    vbox->setResizeMode(QLayout::FreeResize);
    setMinimumWidth(parent->width() / 2);
    setIcon(Resource::loadPixmap("portabase"));
#endif
}

CalcDateEditor::~CalcDateEditor()
{

}

void CalcDateEditor::reset()
{
    group->setButton(0);
    if (columnList->count() > 0) {
        columnList->setCurrentItem(0);
    }
    QDate today = QDate::currentDate();
    dateWidget->setDate(today);
}

CalcNode *CalcDateEditor::createNode()
{
    CalcNode *node = new CalcNode(0, "");
    updateNode(node);
    return node;
}

void CalcDateEditor::updateNode(CalcNode *node)
{
    int selection = group->id(group->selected());
    if (selection == 0) {
        node->setType(CALC_DATE_COLUMN);
        node->setValue(columnList->currentText());
    }
    else {
        node->setType(CALC_DATE_CONSTANT);
        node->setValue(QString::number(dateWidget->getDate()));
    }
}

void CalcDateEditor::setNode(CalcNode *node)
{
    reset();
    int type = node->type();
    QString value = node->value();
    if (type == CALC_DATE_CONSTANT) {
        group->setButton(1);
        dateWidget->setDate(value.toInt());
    }
    else if (type == CALC_DATE_COLUMN) {
        group->setButton(0);
        int count = columnList->count();
        for (int i = 0; i < count; i++) {
            if (columnList->text(i) == value) {
                columnList->setCurrentItem(i);
                break;
            }
        }
    }
}
