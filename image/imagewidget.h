/*
 * imagewidget.h
 *
 * Changes (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
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


#ifndef _IMAGEWIDGET_H
#define _IMAGEWIDGET_H

#include <qpixmap.h>
#include <qwidget.h>

class View;

class ImageWidget : public QWidget
{
    Q_OBJECT
 public:
    ImageWidget(QWidget *parent, const char *name=0, WFlags f=0);
    ~ImageWidget();

    void setPixmap(QPixmap pm, bool resize=TRUE);
    void setView(View *view, int row, int column);

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

private:
    void processArrow(int key);

private:
    QPixmap pixmap;
    View *currentView;
    int rowIndex;
    int colIndex;
};

#endif
