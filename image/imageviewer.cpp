/*
 * imageviewer.cpp
 *
 * (c) 2003-2004,2008-2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file imageviewer.cpp
 * Source file for ImageViewer
 */

#include <QApplication>
#include <QDesktopWidget>
#include <QDialogButtonBox>
#include <QKeyEvent>
#include <QLayout>
#include <QScrollArea>
#include "imageviewer.h"
#include "imagewidget.h"
#include "../view.h"

/**
 * Constructor.
 *
 * @param allowFullScreen True if a click is to allow full-screen display
 * @param parent This dialog's parent widget
 */
ImageViewer::ImageViewer(bool allowFullScreen, QWidget *parent)
    : PBDialog(tr("Image Viewer"), parent), currentView(0), rowIndex(0), colIndex(0)
{
    QScrollArea *scroll = new QScrollArea(this);
    vbox->addWidget(scroll);
    display = new ImageWidget(scroll);
    QPixmap pm;
    display->setPixmap(pm);
    scroll->setWidget(display);
    if (allowFullScreen) {
        connect(display, SIGNAL(clicked()), this, SLOT(showFullScreen()));
    }

    okCancelRow = finishLayout(true, false);
}

/**
 * Set the image to be displayed.
 *
 * @param image The image to show
 */
void ImageViewer::setImage(const QImage &image)
{
    qApp->processEvents();
    pm = QPixmap::fromImage(image);
    display->setPixmap(pm);
    resize(pm.width() + 16, pm.height() + okCancelRow->height() + 8);
}

/**
 * Set the currently selected database view.  Used to determine the sequence
 * of images to show when using slideshows or the arrow keys.
 *
 * @param view The database view currently in use
 * @param row The index of the table row to start at
 * @param column The index of the table column to start at
 */
void ImageViewer::setView(View *view, int row, int column)
{
    currentView = view;
    rowIndex = row;
    colIndex = column;
}

/**
 * Use a full-screen display to show the current image.
 */
void ImageViewer::showFullScreen()
{
    ImageWidget *fullScreen = new ImageWidget(0);
    fullScreen->setAttribute(Qt::WA_DeleteOnClose);
    fullScreen->setView(currentView, rowIndex, colIndex);
    fullScreen->setAutoFillBackground(true);
    QPalette fsPalette(fullScreen->palette());
    fsPalette.setColor(QPalette::Window, Qt::black);
    fullScreen->setPalette(fsPalette);
    fullScreen->setPixmap(pm, false);
    fullScreen->resize(qApp->desktop()->size());
    hide();
    connect(fullScreen, SIGNAL(clicked()), this, SLOT(accept()));
    fullScreen->setFocus(Qt::OtherFocusReason);
    fullScreen->showFullScreen();
    connect(fullScreen, SIGNAL(clicked()), fullScreen, SLOT(close()));
}

/**
 * Check each key press to see if it represented a navigation command.
 *
 * @param e The event which was fired
 */
void ImageViewer::keyReleaseEvent(QKeyEvent *e)
{
    int key = e->key();
    if (!currentView) {
        e->ignore();
        return;
    }
    if (key == Qt::Key_Left || key == Qt::Key_Up || key == Qt::Key_Right
          || key == Qt::Key_Down) {
        processArrow(key);
    }
    else {
        e->ignore();
    }
}

/**
 * Handle arrow key presses by moving to the next or previous image in the
 * column being viewed, as appropriate.
 *
 * @param key The arrow key which was pressed
 */
void ImageViewer::processArrow(int key)
{
    if (!currentView) {
        return;
    }
    int rowCount = currentView->getRowCount();
    bool changed = false;
    if (key == Qt::Key_Left || key == Qt::Key_Up) {
        if (rowIndex != 0) {
            rowIndex--;
            changed = true;
        }
    }
    else if (key == Qt::Key_Right || key == Qt::Key_Down) {
        if (rowIndex != rowCount - 1) {
            rowIndex++;
            changed = true;
        }
    }
    if (changed) {
        int rowId = currentView->getId(rowIndex);
        setImage(currentView->getImage(rowId, colIndex));
    }
}
