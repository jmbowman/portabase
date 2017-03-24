/*
 * slideshowdialog.cpp
 *
 * (c) 2004,2009-2010,2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file slideshowdialog.cpp
 * Source file for SlideshowDialog
 */

#include <QApplication>
#include <QComboBox>
#include <QDesktopWidget>
#include <QDialogButtonBox>
#include <QLabel>
#include <QSettings>
#include "../factory.h"
#include "../qqutil/qqspinbox.h"
#include "../view.h"
#include "imageviewer.h"
#include "slideshowdialog.h"

/**
 * Constructor.
 *
 * @param columns Names of all image columns in the current view
 * @param view The current view of the database
 * @param parent This dialog's parent widget
 */
SlideshowDialog::SlideshowDialog(QStringList columns, View *view, QWidget *parent)
  : PBDialog(tr("Slideshow"), parent, true, false)
{
    currentView = view;
    QGridLayout *grid = Factory::gridLayout(vbox);
    grid->setColumnStretch(1, 1);
    grid->addWidget(new QLabel(tr("Column"), this), 0, 0);
    columnList = Factory::comboBox(this);
    columnList->addItems(columns);
    grid->addWidget(columnList, 0, 1);

    grid->addWidget(new QLabel(tr("Delay between images"), this), 1, 0);
    delayBox = new QQSpinBox(this);
    delayBox->setMinimum(1);
    delayBox->setSuffix(" " + tr("seconds"));
#if defined(Q_OS_ANDROID)
    QQSpinBox::updateStyleSheet(delayBox);
#endif
    grid->addWidget(delayBox, 1, 1, Qt::AlignLeft);
    QSettings settings;
    QVariant delay = settings.value("General/SlideshowDelay", 5);
    delayBox->setValue(delay.toInt());

#if defined(Q_OS_ANDROID)
    vbox->addStretch(1);
#endif
    finishLayout();
}

/**
 * Start the slideshow.
 */
void SlideshowDialog::accept()
{
    QStringList columns = currentView->getColNames();
    int colIndex = columns.indexOf(columnList->currentText());
    ImageViewer *fullScreen = new ImageViewer(true, parentWidget());
    fullScreen->setView(currentView, 0, colIndex);
    fullScreen->slideshow(delayBox->value());
    hide();
    int delay = delayBox->value();
    QSettings settings;
    settings.setValue("General/SlideshowDelay", delay);
    PBDialog::accept();
}
