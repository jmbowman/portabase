/*
 * imagewidget.h (based on code from TxImage)
 *
 * (c) 2003-2004,2008-2009,2015,2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 * (c) 2003 by Pierpaolo Di Panfilo <pippo_dp@libero.it>
 * (c) 2003 by Cristian Di Panfilo <cridipan@virgilio.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file imagewidget.h
 * Header file for ImageWidget
 */

#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QPixmap>
#include <QWidget>

class View;

/**
 * A widget for viewing an image.
 */
class ImageWidget : public QWidget
{
    Q_OBJECT
 public:
    explicit ImageWidget(QWidget *parent);

    void setPixmap(QPixmap pm, bool resize=true);

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent *);
    void mouseReleaseEvent(QMouseEvent* event);

private:
    QPixmap pixmap; /**< The image being shown */
};

#endif
