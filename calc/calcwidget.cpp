/*
 * calcwidget.cpp
 *
 * (c) 2003-2004,2008-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file calcwidget.cpp
 * Source file for CalcWidget
 */

#include <QLabel>
#include <QAbstractButton>
#include "calcnode.h"
#include "calcwidget.h"
#include "../database.h"
#include "../factory.h"
#include "../roweditor.h"

/**
 * Constructor.
 *
 * @param dbase The database being edited
 * @param calcName The name of the column this widget will represent
 * @param colNames Ordered list of all the database's column names
 * @param editor The RowEditor dialog in which this widget will appear
 * @param parent This widget's parent widget
 */
CalcWidget::CalcWidget(Database *dbase, const QString &calcName, const QStringList &colNames, RowEditor *editor, QWidget *parent)
  : QWidget(parent), db(dbase), rowEditor(editor)
{
    colName = calcName;
    columns = colNames;
    QHBoxLayout *layout = Factory::hBoxLayout(this, true);
    display = new QLabel(this);
    layout->addWidget(display);
    QAbstractButton *button = Factory::button(this);
    button->setIcon(QIcon(":/icons/refresh.png"));
    button->setToolTip(tr("Update calculated value"));
    layout->addWidget(button);
    connect(button, SIGNAL(clicked()), this, SLOT(calculate()));
    int height = button->height();
    setMaximumHeight(height);
}

/**
 * Get the currently displayed calculation result.
 *
 * @return The text representation of the last-calculated result
 */
QString CalcWidget::getValue()
{
    return display->text();
}

/**
 * Set the value to display for this widget.
 *
 * @param value The text to display
 */
void CalcWidget::setValue(const QString &value)
{
    display->setText(value);
}

/**
 * Recalculate the value to be displayed, based on the other values currently
 * entered in the RowEditor dialog.  Called automatically when the "Update"
 * button is clicked.
 */
void CalcWidget::calculate()
{
    if (!rowEditor->isValid()) {
        return;
    }
    int decimals = 2;
    CalcNode *root = db->loadCalc(colName, &decimals);
    QStringList row = rowEditor->getRow(false);
    double value = root->value(row, columns);
    setValue(db->formatDouble(value, decimals));
    delete root;
}
