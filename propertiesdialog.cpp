/*
 * propertiesdialog.cpp
 *
 * (c) 2011 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file
 * Source file for PropertiesDialog
 */

#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QLocale>
#include <QPushButton>
#include <QTextEdit>
#include "database.h"
#include "factory.h"
#include "formatting.h"
#include "propertiesdialog.h"
#include "viewdisplay.h"

/**
 * Constructor.
 *
 * @param parent This dialog's parent widget
 */
PropertiesDialog::PropertiesDialog(const QString &filePath, Database *db,
                                   ViewDisplay *viewer, QWidget *parent)
  : PBDialog(tr("File Properties"), parent), vd(viewer)
{
    QTextEdit *display = Factory::textDisplay(this);
    vbox->addWidget(display);

    QStringList content;
    content.append("<qt><table cellspacing=\"0\">");
    QString row("<tr><td align=\"right\"><font color=\"#0000ff\">%1: </font></td><td>%2</td></tr>");

    QFile file(filePath);
    QFileInfo info(file);
    content.append(row.arg(tr("Name")).arg(info.fileName()));
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
    content.append(row.arg(tr("Size")).arg(units.arg(sizeString)));

    QString modified = Formatting::dateTimeToString(info.lastModified());
    content.append(row.arg(tr("Modified")).arg(modified));

    int count = db->getData().GetSize();
    QString rowsInFilter = locale.toString(vd->rowCount());
    QString inCurrent = tr("%1 in current filter").arg(rowsInFilter);
    content.append(row.arg(tr("Rows")).arg(QString("%1 (%2)")
                                           .arg(locale.toString(count))
                                           .arg(inCurrent)));

    count = db->listColumns().count();
    QString colsInView = locale.toString(vd->columnCount());
    inCurrent = tr("%1 in current view").arg(colsInView);
    content.append(row.arg(tr("Columns")).arg(QString("%1 (%2)")
                                              .arg(locale.toString(count))
                                              .arg(inCurrent)));

    count = db->listViews().count();
    content.append(row.arg(tr("Views")).arg(locale.toString(count)));

    QStringList sortings = db->listSortings();
    sortings.removeAll("_single");
    count = sortings.count();
    content.append(row.arg(tr("Sortings")).arg(locale.toString(count)));

    QStringList filters = db->listFilters();
    filters.removeAll("_simple");
    count = filters.count();
    content.append(row.arg(tr("Filters")).arg(locale.toString(count)));

    count = db->listEnums().count();
    content.append(row.arg(tr("Enums")).arg(locale.toString(count)));

    content.append("</table></qt>");
    display->setHtml(content.join(""));

    QHBoxLayout *hbox = Factory::hBoxLayout(vbox);
    hbox->addStretch(1);
    QPushButton *button = new QPushButton(tr("Column Statistics"), this);
    connect(button, SIGNAL(clicked()), this, SLOT(showColumnStatistics()));
    hbox->addWidget(button);
    hbox->addStretch(1);

    finishLayout(true, false);
}

/**
 * Handler for the button to show the column statistics dialog.
 */
void PropertiesDialog::showColumnStatistics()
{
    vd->showStatistics(true);
}
