/*
 * imagewidget.cpp (based on code from TxImage)
 *
 * (c) 2003-2004,2008-2010,2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
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
#include <QPainter>
#include "imagewidget.h"
#include "../view.h"

/**
 * Constructor.
 *
 * @param parent This widget's parent widget
 */
ImageWidget::ImageWidget(QWidget *parent)
  : QWidget(parent)
{
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
 * Draw the image on the widget as prompted.
 */
void ImageWidget::paintEvent(QPaintEvent *)
{
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
