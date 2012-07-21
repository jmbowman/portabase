/*
 * columninfo.cpp
 *
 * (c) 2011-2012 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file
 * Source file for ColumnInfoDialog
 */

#include <QComboBox>
#include <QLabel>
#include <QSpinBox>
#include <QTextEdit>
#include "columninfo.h"
#include "factory.h"
#include "view.h"

/**
 * Constructor.
 *
 * @param parent This dialog's parent widget
 */
ColumnInfoDialog::ColumnInfoDialog(QWidget *parent)
  : PBDialog(tr("Column Statistics"), parent), view(0), edited(false)
{
    topRow = new QWidget(this);
    QHBoxLayout *hbox = Factory::hBoxLayout(topRow);
    hbox->addWidget(new QLabel(tr("Column") + " ", this));
    columns = new QComboBox(this);
    connect(columns, SIGNAL(currentIndexChanged(int)),
            this, SLOT(columnSelected(int)));
    hbox->addWidget(columns, 1);
    vbox->addWidget(topRow);

    display = Factory::textDisplay(this);
    vbox->addWidget(display);

    hbox = Factory::hBoxLayout(vbox);
    hbox->addWidget(new QLabel(tr("Width") + " ", this));
    colWidth = new QSpinBox(this);
    colWidth->setMaximum(9999);
    connect(colWidth, SIGNAL(valueChanged(int)),
            this, SLOT(widthChanged(int)));
    hbox->addWidget(colWidth);
    hbox->addStretch(1);

    finishLayout();
}

/**
 * Update the dialog's widgets and show it.
 *
 * @param currentView The database view currently in use
 * @param colName The name of the column to display; if omitted, the user can
 *                choose from a list of the ones in the view
 * @return True if any column widths were changed, false otherwise
 */
bool ColumnInfoDialog::launch(View *currentView, const QString &colName)
{
    view = currentView;
    edited = false;
    columns->clear();
    columns->addItems(view->getColNames());
    int i;
    if (colName.isNull()) {
        topRow->show();
        for (i = 0; i < columns->count(); i++) {
            columns->setItemData(i, view->getColWidth(i));
        }
        colWidth->setValue(view->getColWidth(0));
    }
    else {
        topRow->hide();
        i = columns->findText(colName);
        columns->setItemData(i, view->getColWidth(i));
        columns->setCurrentIndex(i);
        colWidth->setValue(view->getColWidth(i));
    }
    if (exec()) {
        if (edited) {
            for (i = 0; i < columns->count(); i++) {
                QVariant width = columns->itemData(i);
                if (width.isValid()) {
                    view->setColWidth(i, width.toInt());
                }
            }
            return true;
        }
    }
    return false;
}

/**
 * Update the dialog when a different column is selected from the list.
 *
 * @param index The index of the selected column in the list
 */
void ColumnInfoDialog::columnSelected(int index)
{
    if (index == -1) {
        display->clear();
        colWidth->setValue(0);
        return;
    }
    QStringList content;
    QString name = columns->itemText(index);
    content.append(QString("<center><b>%1</b></center>").arg(name));
    content.append("<table cellspacing=0>");
    content.append(view->getStatistics(index));
    content.append("</table>");
    display->clear();
    display->append(content.join(""));
    QVariant width = columns->itemData(index);
    if (width.isValid()) {
        colWidth->setValue(width.toInt());
    }
}

/**
 * Store changes to column widths as they are specified, so they can be
 * applied in bulk when the dialog is closed.
 *
 * @param newWidth The new column width from the spin box
 */
void ColumnInfoDialog::widthChanged(int newWidth)
{
    columns->setItemData(columns->currentIndex(), newWidth);
    edited = true;
}
