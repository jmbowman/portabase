/*
 * imageviewer.cpp
 *
 * (c) 2003-2004,2008-2010,2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
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
#include "../qqutil/qqfactory.h"
#include "../view.h"

/**
 * The scroll area containing the image.  Responds to clicks outside the
 * image display by returning from fullscreen mode to normal dialog mode.
 */
class ClickableScrollArea: public QScrollArea
{
public:
    ClickableScrollArea(ImageViewer *parent);

protected:
    void mouseReleaseEvent(QMouseEvent* event);
};

/**
 * Constructor.
 *
 * @param parent The parent ImageViewer dialog
 */
ClickableScrollArea::ClickableScrollArea(ImageViewer *parent)
    : QScrollArea(parent)
{

}

/**
 * Make the parent dialog exit fullscreen mode when the area around the
 * image is clicked.
 *
 * @param event The triggering mouse event
 */
void ClickableScrollArea::mouseReleaseEvent(QMouseEvent *event)
{
    ImageViewer *viewer = qobject_cast<ImageViewer*>(parentWidget());
    viewer->showNormal();
}

/**
 * Constructor.
 *
 * @param allowFullScreen True if a click is to allow full-screen display
 * @param parent This dialog's parent widget
 */
ImageViewer::ImageViewer(bool allowFullScreen, QWidget *parent)
    : PBDialog(tr("Image Viewer"), parent), currentView(0), rowIndex(0), colIndex(0)
{
    scroll = new ClickableScrollArea(this);
    QQFactory::configureScrollArea(scroll);
    setAutoFillBackground(true);
    vbox->addWidget(scroll);
    display = new ImageWidget(scroll);
    QPixmap pm;
    display->setPixmap(pm);
    scroll->setAlignment(Qt::AlignCenter);
    scroll->setWidget(display);
    if (allowFullScreen) {
        connect(display, SIGNAL(clicked()), this, SLOT(showFullScreen()));
    }
#if defined(Q_WS_MAEMO_5)
    // Allow the image to auto-rotate
    setAttribute(Qt::WA_Maemo5AutoOrientation, true);
#endif

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
#ifndef MOBILE
    if (!isFullScreen()) {
        resize(pm.width() + 16, pm.height() + okCancelRow->height() + 8);
    }
#endif
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
    disconnect(display, SIGNAL(clicked()), this, SLOT(showFullScreen()));
    connect(scroll, SIGNAL(clicked()), this, SLOT(showNormal()));
    connect(display, SIGNAL(clicked()), this, SLOT(showNormal()));
#if !defined(MOBILE)
    okCancelRow->hide();
#endif
    QPalette scrollPalette(palette());
    scrollPalette.setColor(QPalette::Window, Qt::black);
    setPalette(scrollPalette);
    scroll->setFrameShape(QFrame::NoFrame);
    PBDialog::showFullScreen();
}

/**
 * Exit full-screen mode.
 */
void ImageViewer::showNormal()
{
    disconnect(scroll, SIGNAL(clicked()), this, SLOT(showNormal()));
    disconnect(display, SIGNAL(clicked()), this, SLOT(showNormal()));
    connect(display, SIGNAL(clicked()), this, SLOT(showFullScreen()));
    setPalette(parentWidget()->palette());
    scroll->setFrameShape(QFrame::StyledPanel);
#if defined(MOBILE)
    PBDialog::showFullScreen();
#else
    okCancelRow->show();
    PBDialog::showNormal();
    // Size appropriately for the current image
    if (!isFullScreen()) {
        resize(pm.width() + 16, pm.height() + okCancelRow->height() + 8);
    }
#endif
}

/**
 * Always exit fullscreen mode before closing.
 *
 * @param event The window close event
 */
void ImageViewer::reject()
{
    if (isFullScreen()) {
        showNormal();
    }
    PBDialog::reject();
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
    int rowCount = currentView->totalRowCount();
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
