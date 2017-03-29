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
#include <QTimer>
#include "imageviewer.h"
#include "imagewidget.h"
#include "../qqutil/qqfactory.h"
#include "../view.h"

#if defined(Q_OS_ANDROID)
#include "../qqutil/actionbar.h"
#endif

#if defined(Q_WS_HILDON) || defined(Q_WS_MAEMO_5)
#include <QtDBus>
#include <mce/mode-names.h>
#include <mce/dbus-names.h>
#endif

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
    Q_UNUSED(event)
    ImageViewer *viewer = qobject_cast<ImageViewer*>(parentWidget());
    if (viewer->isFullScreen()) {
        viewer->showNormal();
    }
}

/**
 * Constructor.
 *
 * @param allowFullScreen True if a click is to allow full-screen display
 * @param parent This dialog's parent widget
 */
ImageViewer::ImageViewer(bool allowFullScreen, QWidget *parent)
    : PBDialog(tr("Image Viewer"), parent), currentView(0), rowIndex(0),
      colIndex(0), timer(0), ssTimer(0), slideshowDelay(0)
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
 * Destructor.
 */
ImageViewer::~ImageViewer()
{
    if (timer) {
        delete timer;
    }
    if (ssTimer) {
        delete ssTimer;
    }
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
#if defined(Q_OS_ANDROID)
    actionBar->hide();
#endif
    QPalette scrollPalette(palette());
    scrollPalette.setColor(QPalette::Window, Qt::black);
    setPalette(scrollPalette);
    scroll->setFrameShape(QFrame::NoFrame);
    PBDialog::showFullScreen();
}

/**
 * Exit full-screen mode.  If a slideshow was in progress, stop it and close
 * the dialog.
 */
void ImageViewer::showNormal()
{
    disconnect(scroll, SIGNAL(clicked()), this, SLOT(showNormal()));
    disconnect(display, SIGNAL(clicked()), this, SLOT(showNormal()));
    connect(display, SIGNAL(clicked()), this, SLOT(showFullScreen()));
    setPalette(parentWidget()->palette());
    scroll->setFrameShape(QFrame::StyledPanel);
#if defined(Q_OS_ANDROID)
    actionBar->show();
#endif
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
    if (ssTimer) {
        ssTimer->stop();
    }
    if (timer) {
        timer->stop();
        PBDialog::reject();
        return;
    }
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
    bool lastImage = false;
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
        else {
            lastImage = true;
        }
    }
    if (changed) {
        int rowId = currentView->getId(rowIndex);
        setImage(currentView->getImage(rowId, colIndex));
        if (timer) {
            timer->start(slideshowDelay * 1000);
        }
    }
    if (timer && lastImage) {
        reject();
    }
}

/**
 * Start a slideshow with the specified delay between images.
 *
 * @param delay The number of seconds between images
 */
void ImageViewer::slideshow(int delay)
{
    slideshowDelay = delay;
    int rowId = currentView->getId(rowIndex);
    setImage(currentView->getImage(rowId, colIndex));
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(nextImage()));
    showFullScreen();
    timer->start(delay * 1000);
#if defined(Q_WS_HILDON) || defined(Q_WS_MAEMO_5)
    ssTimer = new QTimer(this);
    connect(ssTimer, SIGNAL(timeout()), this, SLOT(keepScreenOn()));
    ssTimer->start(30 * 1000);
#endif
}

/**
 * Go to the next image in a slideshow.
 */
void ImageViewer::nextImage()
{
    processArrow(Qt::Key_Right);
}

/**
 * Send a D-Bus signal on Maemo to prevent the backlight from turning off.
 */
void ImageViewer::keepScreenOn()
{
#if defined(Q_WS_HILDON) || defined(Q_WS_MAEMO_5)
    QDBusConnection::systemBus().call(
        QDBusMessage::createMethodCall(MCE_SERVICE,
                                       MCE_REQUEST_PATH,
                                       MCE_REQUEST_IF,
                                       MCE_PREVENT_BLANK_REQ));
#endif
}

/**
 * Respond to the screen orientation or size changing.  Resizes the displayed
 * image if appropriate.
 *
 * @param geometry The new screen dimensions (in pixels)
 */
void ImageViewer::screenGeometryChanged(const QRect &geometry)
{
    int rowId = currentView->getId(rowIndex);
    setImage(currentView->getImage(rowId, colIndex));
    PBDialog::screenGeometryChanged(geometry);
}
