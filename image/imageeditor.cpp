/*
 * imageeditor.cpp
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
#include <qcombobox.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qscrollview.h>
#include <qspinbox.h>
#include "imageeditor.h"
#include "imagereader.h"
#include "imagewidget.h"

ImageEditor::ImageEditor(QWidget *parent, const char *name)
    : PBDialog(tr("Image Editor"), parent, name), path("")
{
    paramsRow = new QHBox(this);
    vbox->addWidget(paramsRow);
    new QLabel(tr("Width"), paramsRow);
    widthBox = new QSpinBox(1, 800, 1, paramsRow);
    new QLabel(tr("Height"), paramsRow);
    heightBox = new QSpinBox(1, 600, 1, paramsRow);

    new QLabel(tr("Rotate"), paramsRow);
    rotateBox = new QComboBox(FALSE, paramsRow);
    rotateBox->insertItem("0");
    rotateBox->insertItem("90");
    rotateBox->insertItem("180");
    rotateBox->insertItem("270");

    QPushButton *refreshButton = new QPushButton(tr("Refresh"), paramsRow);
    connect(refreshButton, SIGNAL(clicked()), this, SLOT(updateImage()));

    QScrollView *scroll = new QScrollView(this, 0,
                                          WResizeNoErase|WNorthWestGravity);
    vbox->addWidget(scroll);
    display = new ImageWidget(scroll->viewport());
    QPixmap pm;
    display->setPixmap(pm);
    scroll->addChild(display);

    finishLayout();
}

ImageEditor::~ImageEditor()
{

}

int ImageEditor::edit(const QString &file)
{
    path = file;
    ImageReader reader(file);
    format = reader.format();
    oldWidth = reader.imageWidth();
    oldHeight = reader.imageHeight();
    int width = oldWidth;
    int height = oldHeight;
    // Scale the image down so as to not swamp the Zaurus's memory when loaded
    // (only works for JPEGs, big PNG images will be rejected)
    int scaleDenom = 1;
    while (width * height > 800 * 600) {
        scaleDenom *= 2;
        // I doubt if many people have 6400 x 4800 images, but it's probably
        // only a matter of time before digital cameras get there...
        if (scaleDenom > 8 || format != "JPEG") {
            QMessageBox::warning(this, PBDialog::tr("PortaBase"),
                                 tr("Image is too large to import"));
            return QDialog::Rejected;
        }
        width /= 2;
        height /= 2;
    }
    widthBox->setMaxValue(width);
    widthBox->setValue(width);
    heightBox->setMaxValue(height);
    heightBox->setValue(height);
    rotateBox->setCurrentItem(0);
    image = reader.getImage(width, height);
    qApp->processEvents();
    QPixmap pm;
    pm.setOptimization(QPixmap::NormalOptim);
    pm.convertFromImage(image);
    display->setPixmap(pm);
    int margin = 5;
#if defined(Q_WS_WIN)
    margin += 16;
#endif
#if !defined(Q_WS_QWS)
    resize(QMAX(pm.width() + margin, paramsRow->sizeHint().width() + margin),
           paramsRow->height() + pm.height() + okCancelRow->height() + margin);
#endif
    return exec();
}

void ImageEditor::updateImage()
{
    image.reset();
    ImageReader reader(path);
    int width = widthBox->value();
    int height = heightBox->value();
    int angle = rotateBox->currentItem() * 90;
    if (angle == 90 || angle == 270) {
        int temp = width;
        width = height;
        height = temp;
    }
    image = reader.getRotatedImage(width, height, angle);
    qApp->processEvents();
    QPixmap pm;
    pm.setOptimization(QPixmap::NormalOptim);
    pm.convertFromImage(image);
    display->setPixmap(pm);
}

QImage ImageEditor::getImage()
{
    return image;
}

QString ImageEditor::getFile()
{
    return path;
}

QString ImageEditor::getFormat()
{
    return format;
}

bool ImageEditor::isModified()
{
    if (rotateBox->currentItem() != 0) {
        return TRUE;
    }
    if (widthBox->value() != oldWidth || heightBox->value() != oldHeight) {
        return TRUE;
    }
    return FALSE;
}
