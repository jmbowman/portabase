/*
 * imagewidget.h (based on code from TxImage)
 *
 * (c) 2003-2004,2008-2009,2015 by Jeremy Bowman <jmbowman@alum.mit.edu>
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

class QTimer;
class View;

/**
 * A widget for viewing one image at a time from a column in a View.
 * Pressing the left or up arrow key switches to the image in the same column
 * of the previous row (if any) in the View, pressing the right or down arrow
 * key goes to the next image in the other direction.  Can be displayed
 * fullscreen.
 */
class ImageWidget : public QWidget
{
    Q_OBJECT
 public:
    explicit ImageWidget(QWidget *parent);

    void setPixmap(QPixmap pm, bool resize=true);
    void setView(View *view, int row, int column);
    void slideshow(int delay);

signals:
    void clicked();
    void OKPressed();
    void CancelPressed();

protected:
    void paintEvent(QPaintEvent *);
    void keyReleaseEvent (QKeyEvent *);
    void mouseReleaseEvent(QMouseEvent* event);
    void closeEvent(QCloseEvent * e);
    bool closing;

private slots:
    void nextImage();
    void keepScreenOn();

private:
    void processArrow(int key);
    void updateImage();

private:
    QPixmap pixmap; /**< The image being shown */
    View *currentView; /**< The database view in use */
    int rowIndex; /**< The table row index of the current image */
    int colIndex; /**< The table column index of the current image */
    QTimer *timer; /**< Timer used in slideshows */
    QTimer *ssTimer; /**< Timer used on Maemo to keep screen on during slideshows */
    int slideshowDelay; /**< The delay in seconds between images in a slideshow */
};

#endif
