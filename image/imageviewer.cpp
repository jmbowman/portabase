/*
 * imageviewer.cpp
 *
 * (c) 2003-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qapplication.h>
#include <qhbox.h>
#include <qscrollview.h>
#include "imageviewer.h"
#include "imagewidget.h"
#include "../view.h"

ImageViewer::ImageViewer(bool allowFullScreen, QWidget *parent, const char *name)
    : PBDialog(tr("Image Viewer"), parent, name), fullScreen(0), currentView(0), rowIndex(0), colIndex(0)
{
    QScrollView *scroll = new QScrollView(this, 0,
                                          WResizeNoErase|WNorthWestGravity);
    vbox->addWidget(scroll);
    display = new ImageWidget(scroll->viewport());
    QPixmap pm;
    display->setPixmap(pm);
    scroll->addChild(display);
    if (allowFullScreen) {
        connect(display, SIGNAL(clicked()), this, SLOT(showFullScreen()));
    }

    finishLayout(TRUE, FALSE, TRUE, 0, 0);
}

ImageViewer::~ImageViewer()
{

}

void ImageViewer::setImage(const QImage &image)
{
    qApp->processEvents();
    pm.setOptimization(QPixmap::NormalOptim);
    pm.convertFromImage(image);
    display->setPixmap(pm);
    int margin = 5;
#if defined(Q_WS_WIN)
    margin += 16;
#endif
#if !defined(Q_WS_QWS)
    resize(pm.width() + margin, pm.height() + okCancelRow->height() + margin);
#endif
}

void ImageViewer::setView(View *view, int row, int column)
{
    currentView = view;
    rowIndex = row;
    colIndex = column;
}

void ImageViewer::showFullScreen()
{
    fullScreen = new ImageWidget(0, 0, WDestructiveClose);
    fullScreen->setView(currentView, rowIndex, colIndex);
    fullScreen->setBackgroundColor(Qt::black);
    fullScreen->setPixmap(pm, FALSE);
    fullScreen->resize(qApp->desktop()->size());
    hide();
    connect(fullScreen, SIGNAL(clicked()), this, SLOT(accept()));
    fullScreen->setFocus();
    fullScreen->showFullScreen();
    connect(fullScreen, SIGNAL(clicked()), fullScreen, SLOT(close()));
}

void ImageViewer::keyReleaseEvent(QKeyEvent *e)
{
    int key = e->key();
    if (!currentView) {
        e->ignore();
        return;
    }
    if (key == Key_Left || key == Key_Up || key == Key_Right || key == Key_Down) {
        processArrow(key);
    }
    else {
        e->ignore();
    }
}

void ImageViewer::processArrow(int key)
{
    if (!currentView) {
        return;
    }
    int rowCount = currentView->getRowCount();
    bool changed = FALSE;
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
    }
    if (changed) {
        int rowId = currentView->getId(rowIndex);
        setImage(currentView->getImage(rowId, colIndex));
    }
}
