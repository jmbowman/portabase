/*
 * imagewidget.cpp (based on code from TxImage)
 *
 * (c) 2003-2004,2008-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 * (c) 2003 by Pierpaolo Di Panfilo <pippo_dp@libero.it>
 * (c) 2003 by Cristian Di Panfilo <cridipan@virgilio.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file imagewidget.cpp
 * Source file for ImageWidget
 */

#include <QImage>
#include <QKeyEvent>
#include <QPainter>
#include <QTimer>
#include "imagewidget.h"
#include "../view.h"

#if defined(Q_WS_HILDON) || defined(Q_WS_MAEMO_5)
#include <QtDBus>
#include <mce/mode-names.h>
#include <mce/dbus-names.h>
#endif

/**
 * Constructor.
 *
 * @param parent This widget's parent widget (0 if it is to be full-screen)
 */
ImageWidget::ImageWidget(QWidget *parent)
  : QWidget(parent), currentView(0), rowIndex(0), colIndex(0),
    timer(0), ssTimer(0), slideshowDelay(0)
{
    closing = false;
    setAttribute(Qt::WA_NoSystemBackground);
}

/**
 * Set the image to be displayed.
 *
 * @param pm The image data
 * @param res True if the widget may need to be resized to accomodate the new
 *            image, false otherwise (like when shown full-screen)
 */
void ImageWidget::setPixmap(QPixmap pm, bool res)
{
    pixmap = pm;
    if (res) {
        resize(pm.width(), pm.height());
    }
}

/**
 * Set the currently selected database view.  Used to determine the sequence
 * of images to show when using slideshows or the arrow keys.
 *
 * @param view The database view currently in use
 * @param row The index of the table row to start at
 * @param column The index of the table column to start at
 */
void ImageWidget::setView(View *view, int row, int column)
{
    currentView = view;
    rowIndex = row;
    colIndex = column;
}

/**
 * Draw the image on the widget as prompted.
 */
void ImageWidget::paintEvent(QPaintEvent *)
{
    if (closing) {
        return;
    }
    QPainter p(this);
    if (!pixmap.isNull()) {
        if (pixmap.size() == size()) {
            p.drawPixmap(rect(), pixmap);
            return;
        }
        p.fillRect(rect(), QColor(0, 0, 0));
        double aspectRatio = (double)pixmap.width()/(double)pixmap.height();
        if (aspectRatio * height() > width()) {
            int h = (int)(width() / aspectRatio);
            int y = (height() - h) / 2;
            p.drawPixmap(0, y, width(), h, pixmap);
        }
        else {
            int w = (int)(aspectRatio * height());
            int x = (width() - w) / 2;
            p.drawPixmap(x, 0, w, height(), pixmap);
        }
    }
    else {
        p.fillRect(rect(), QColor(0, 0, 0));
    }
}

/**
 * Generate a "clicked()" signal when the appropriate mouse action is taken.
 */
void ImageWidget::mouseReleaseEvent(QMouseEvent *)
{
    emit clicked();
}

/**
 * Check each key press to see if it represented a navigation or exit command.
 *
 * @param keyEvent The event which was fired
 */
void ImageWidget::keyReleaseEvent(QKeyEvent *keyEvent)
{
    int key = keyEvent->key();

    if (key == Qt::Key_Escape) {
        emit CancelPressed();
        close();
    }
    else if (key == Qt::Key_Enter) {
        emit OKPressed();
    }
    else if (key == Qt::Key_Left || key == Qt::Key_Right || key == Qt::Key_Up
          || key == Qt::Key_Down) {
        processArrow(key);
    }
    keyEvent->accept();
}

/**
 * Handle arrow key presses by moving to the next or previous image in the
 * column being viewed, as appropriate.
 *
 * @param key The arrow key which was pressed
 */
void ImageWidget::processArrow(int key)
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
        updateImage();
        if (timer) {
            timer->start(slideshowDelay * 1000);
        }
    }
    if (timer && lastImage) {
        emit CancelPressed();
        close();
    }
}

/**
 * Update the displayed image to reflect any changes in the current view,
 * row index, and/or column index.
 */
void ImageWidget::updateImage()
{
    int rowId = currentView->getId(rowIndex);
    QImage image = currentView->getImage(rowId, colIndex);
    pixmap = QPixmap::fromImage(image);
    repaint();
}

/**
 * Start a slideshow with the specified delay between images.
 *
 * @param delay The number of seconds between images
 */
void ImageWidget::slideshow(int delay)
{
    slideshowDelay = delay;
    updateImage();
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(nextImage()));
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
void ImageWidget::nextImage()
{
    processArrow(Qt::Key_Right);
}

/**
 * Send a D-Bus signal on Maemo to prevent the backlight from turning off.
 */
void ImageWidget::keepScreenOn()
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
 * Don't bother to continue painting an image if this widget is being closed.
 *
 * @param e The window close event
 */
void ImageWidget::closeEvent(QCloseEvent *e)
{
    closing = true;
    if (timer) {
        delete timer;
    }
    if (ssTimer) {
        delete ssTimer;
    }
    e->accept();
}
