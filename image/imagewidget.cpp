/*
 * imagewidget.cpp
 *
 * Changes (c) 2003-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * Original source:
************************************************************************
** TxImage - Image Viewer
** Copyright (C) 2003
** Pierpaolo Di Panfilo <pippo_dp@libero.it>
** Cristian Di Panfilo <cridipan@virgilio.it>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.  To obtain a
** copy of the GNU Library General Public License, write to the Free
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
**
*************************************************************************/

#include <qimage.h>
#include <qpainter.h>
#include <qtimer.h>
#include "imagewidget.h"
#include "../view.h"

ImageWidget::ImageWidget(QWidget *parent, const char *name, WFlags f)
           : QWidget(parent, name, f), currentView(0), rowIndex(0), colIndex(0), timer(0), slideshowDelay(0)
{
    closing = FALSE;
    setBackgroundMode(NoBackground);
}

ImageWidget::~ImageWidget()
{

}

void ImageWidget::setPixmap(QPixmap pm, bool res)
{
    pixmap = pm;
    // res says if the widget needs to be resized
    // TRUE in imageviewer, FALSE for fullscreen show (slideshow)
    if (res) {
        resize(pm.width(), pm.height());
    }
}

void ImageWidget::setView(View *view, int row, int column)
{
    currentView = view;
    rowIndex = row;
    colIndex = column;
}

void ImageWidget::paintEvent(QPaintEvent *)
{
    if (closing) {
        return;
    }
    else if (!pixmap.isNull()) {
       int x = (width() - pixmap.width()) / 2;
       int y = (height() - pixmap.height()) / 2;
       bitBlt(this, x, y, &pixmap, 0, 0, pixmap.width(),
              pixmap.height(), CopyROP);
    }
    else {
        QPainter p(this);
        p.fillRect(rect(), QColor(0, 0, 0));
    }
}

void ImageWidget::mouseReleaseEvent(QMouseEvent *)
{
    emit clicked();
}

void ImageWidget::keyReleaseEvent(QKeyEvent *keyEvent)
{
    int key = keyEvent->key();

    if (key == Key_Escape) {
        emit CancelPressed();
        close();
    }
    else if (key == Key_Enter) {
        emit OKPressed();
    }
    else if (key == Key_Left || key == Key_Right || key == Key_Up || key == Key_Down) {
        processArrow(key);
    }
    keyEvent->accept();
}

void ImageWidget::processArrow(int key)
{
    if (!currentView) {
        return;
    }
    int rowCount = currentView->getRowCount();
    bool changed = FALSE;
    bool lastImage = FALSE;
    if (key == Key_Left || key == Key_Up) {
        if (rowIndex != 0) {
            rowIndex--;
            changed = TRUE;
        }
    }
    else if (key == Key_Right || key == Key_Down) {
        if (rowIndex != rowCount - 1) {
            rowIndex++;
            changed = TRUE;
        }
        else {
            lastImage = TRUE;
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

void ImageWidget::updateImage()
{
    int rowId = currentView->getId(rowIndex);
    QImage image = currentView->getImage(rowId, colIndex);
    pixmap.setOptimization(QPixmap::NormalOptim);
    pixmap.convertFromImage(image);
    repaint();
}

void ImageWidget::slideshow(int delay)
{
    slideshowDelay = delay;
    updateImage();
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(nextImage()));
    timer->start(delay * 1000);
}

void ImageWidget::nextImage()
{
    processArrow(Key_Right);
}

void ImageWidget::closeEvent(QCloseEvent * e)
{
    closing = TRUE;
    pixmap.resize(0, 0);
    e->accept();
}
