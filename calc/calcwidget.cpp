/*
 * calcwidget.cpp
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#if defined(DESKTOP)
#include "../desktop/resource.h"
#endif

#include <qlabel.h>
#include <qpushbutton.h>
#include "calcnode.h"
#include "calcwidget.h"
#include "../database.h"
#include "../roweditor.h"

CalcWidget::CalcWidget(Database *dbase, const QString &calcName, const QStringList &colNames, RowEditor *editor, QWidget *parent, const char *name, WFlags f)
  : QHBox(parent, name, f), db(dbase), rowEditor(editor)
{
    colName = calcName;
    columns = colNames;
    display = new QLabel(this);
    QPushButton *button = new QPushButton(tr("Update"), this);
    connect(button, SIGNAL(clicked()), this, SLOT(calculate()));
    int height = button->height();
    setMaximumHeight(height);
}

CalcWidget::~CalcWidget()
{

}

QString CalcWidget::getValue()
{
    return display->text();
}

void CalcWidget::setValue(const QString &value)
{
    display->setText(value);
}

void CalcWidget::calculate()
{
    if (!rowEditor->isValid()) {
        return;
    }
    int decimals = 2;
    CalcNode *root = db->loadCalc(colName, &decimals);
    QStringList row = rowEditor->getRow(FALSE);
    double value = root->value(row, columns);
    setValue(db->formatDouble(value, decimals));
    delete root;
}
