/*
 * imageviewer.h
 *
 * (c) 2003-2004,2008-2009,2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file imageviewer.h
 * Header file for ImageViewer
 */

#ifndef IMAGE_VIEWER_H
#define IMAGE_VIEWER_H

#include <QImage>
#include <QPixmap>
#include "../pbdialog.h"

class ImageWidget;
class QDialogButtonBox;
class QScrollArea;
class QTimer;
class View;

/**
 * A dialog used to display images.  Pressing the left or up arrow key switches
 * to the image in the same column of the previous row (if any) in the current
 * view, pressing the right or down arrow key goes to the next image in the
 * other direction.  It can be configured such that clicking on the displayed
 * image displays it in full-screen mode.
 */
class ImageViewer: public PBDialog
{
    Q_OBJECT
public:
    ImageViewer(bool allowFullScreen, QWidget *parent);
    ~ImageViewer();

    void setImage(const QImage &image);
    void setView(View *view, int row, int column);
    void slideshow(int delay);

public slots:
    void showFullScreen();
    void showNormal();

protected:
    void reject();
    void keyReleaseEvent(QKeyEvent *e);

private slots:
    void keepScreenOn();
    void nextImage();
    void processArrow(int key);

private:
    QScrollArea *scroll; /**< The scrollable area containing the image */
    ImageWidget *display; /**< The widget which displays the image */
    QPixmap pm; /**< The image currently being shown */
    QDialogButtonBox *okCancelRow; /**< The row of dialog buttons */
    View *currentView; /**< The view of the database currently in use */
    int rowIndex; /**< Index of the row the shown image is from */
    int colIndex; /**< Index of the column the shown image is from */
    QTimer *timer; /**< Timer used in slideshows */
    QTimer *ssTimer; /**< Timer used on Maemo to keep screen on during slideshows */
    int slideshowDelay; /**< The delay in seconds between images in a slideshow */
};

#endif
