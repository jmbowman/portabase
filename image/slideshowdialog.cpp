/*
 * slideshowdialog.cpp
 *
 * (c) 2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#if defined(DESKTOP)
#include "../desktop/config.h"
#else
#include <qpe/config.h>
#endif

#include <qapplication.h>
#include <qcombobox.h>
#include <qgrid.h>
#include <qlabel.h>
#include <qspinbox.h>
#include "../view.h"
#include "imagewidget.h"
#include "slideshowdialog.h"

SlideshowDialog::SlideshowDialog(QStringList columns, View *view, QWidget *parent, const char *name, WFlags f) : PBDialog(tr("Slideshow"), parent, name, f), fullScreen(0)
{
    currentView = view;
    QGrid *grid = new QGrid(2, this);
    vbox->addWidget(grid);
    new QLabel(tr("Column"), grid);
    columnList = new QComboBox(FALSE, grid);
    columnList->insertStringList(columns);

    new QLabel(tr("Delay between images"), grid);
    delayBox = new QSpinBox(grid);
    delayBox->setMinValue(1);
    delayBox->setSuffix(" " + tr("seconds"));
    Config conf("portabase");
    conf.setGroup("General");
    delayBox->setValue(conf.readNumEntry("SlideshowDelay", 5));

    finishLayout(TRUE, TRUE, FALSE);
}

SlideshowDialog::~SlideshowDialog()
{

}

void SlideshowDialog::accept()
{
    if (!fullScreen) {
        QStringList columns = currentView->getColNames();
        int colIndex = columns.findIndex(columnList->currentText());
        fullScreen = new ImageWidget(0, 0, WDestructiveClose);
        fullScreen->setView(currentView, 0, colIndex);
        fullScreen->setBackgroundColor(Qt::black);
        fullScreen->slideshow(delayBox->value());
        fullScreen->resize(qApp->desktop()->size());
        hide();
        connect(fullScreen, SIGNAL(clicked()), this, SLOT(accept()));
        fullScreen->setFocus();
        fullScreen->showFullScreen();
        connect(fullScreen, SIGNAL(clicked()), fullScreen, SLOT(close()));
        Config conf("portabase");
        conf.setGroup("General");
        conf.writeEntry("SlideshowDelay", delayBox->value());
    }
    else {
        QDialog::accept();
    }
}
