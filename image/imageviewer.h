/*
 * imageviewer.h
 *
 * (c) 2003-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef IMAGE_VIEWER_H
#define IMAGE_VIEWER_H

#include <qimage.h>
#include <qpixmap.h>
#include "../pbdialog.h"

class ImageWidget;
class View;

class ImageViewer: public PBDialog
{
    Q_OBJECT
public:
    ImageViewer(bool allowFullScreen, QWidget *parent, const char *name = 0);
    ~ImageViewer();

    void setImage(const QImage &image);
    void setView(View *view, int row, int column);

protected:
    void keyReleaseEvent(QKeyEvent *e);

private slots:
    void showFullScreen();
    void processArrow(int key);

private:
    ImageWidget *display;
    ImageWidget *fullScreen;
    QPixmap pm;
    View *currentView;
    int rowIndex;
    int colIndex;
};

#endif
