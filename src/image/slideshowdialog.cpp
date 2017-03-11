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
#include "imagewidget.h"
#include "slideshowdialog.h"

/**
 * Constructor.
 *
 * @param columns Names of all image columns in the current view
 * @param view The current view of the database
 * @param parent This dialog's parent widget
 */
SlideshowDialog::SlideshowDialog(QStringList columns, View *view, QWidget *parent)
  : PBDialog(tr("Slideshow"), parent, true, false), fullScreen(0)
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
    if (!fullScreen) {
        QStringList columns = currentView->getColNames();
        int colIndex = columns.indexOf(columnList->currentText());
        fullScreen = new ImageWidget(0);
        fullScreen->setAttribute(Qt::WA_DeleteOnClose);
        fullScreen->setView(currentView, 0, colIndex);
        QPalette fsPalette(fullScreen->palette());
        fsPalette.setColor(QPalette::Window, Qt::black);
        fullScreen->setPalette(fsPalette);
        fullScreen->slideshow(delayBox->value());
        fullScreen->resize(qApp->desktop()->size());
        hide();
        connect(fullScreen, SIGNAL(clicked()), this, SLOT(accept()));
        fullScreen->setFocus();
        fullScreen->showFullScreen();
        connect(fullScreen, SIGNAL(clicked()), fullScreen, SLOT(close()));
        int delay = delayBox->value();
        QSettings settings;
        settings.setValue("General/SlideshowDelay", delay);
    }
    else {
        QDialog::accept();
    }
}
