/*
 * imageeditor.cpp
 *
 * (c) 2003-2004,2008-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file imageeditor.cpp
 * Source file for ImageEditor
 */

#include <QApplication>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QImageReader>
#include <QLabel>
#include <QMatrix>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QWidget>
#include "imageeditor.h"
#include "imagewidget.h"
#include "../factory.h"

/**
 * Constructor.
 *
 * @param parent This dialog's parent widget
 */
ImageEditor::ImageEditor(QWidget *parent)
    : PBDialog(tr("Image Editor"), parent), path("")
{
    paramsRow = new QWidget(this);
    QHBoxLayout *hbox = Factory::hBoxLayout(paramsRow, true);
    vbox->addWidget(paramsRow);
    hbox->addWidget(new QLabel(tr("Width"), paramsRow));
    widthBox = new QSpinBox(paramsRow);
    widthBox->setRange(1, 800);
    hbox->addWidget(widthBox);
    hbox->addWidget(new QLabel(tr("Height"), paramsRow));
    heightBox = new QSpinBox(paramsRow);
    heightBox->setRange(1, 600);
    hbox->addWidget(heightBox);

    hbox->addWidget(new QLabel(tr("Rotate"), paramsRow));
    rotateBox = new QComboBox(paramsRow);
    hbox->addWidget(rotateBox);
    rotateBox->addItem("0");
    rotateBox->addItem("90");
    rotateBox->addItem("180");
    rotateBox->addItem("270");

    QPushButton *refreshButton = new QPushButton(tr("Refresh"), paramsRow);
    hbox->addWidget(refreshButton);
    connect(refreshButton, SIGNAL(clicked()), this, SLOT(updateImage()));

    QScrollArea *scroll = new QScrollArea(this);
    vbox->addWidget(scroll);
    display = new ImageWidget(scroll);
    scroll->setWidget(display);
    QPixmap pm;
    display->setPixmap(pm);

    okCancelRow = finishLayout();
}

/**
 * Open in this dialog the image contained in the specified file.
 *
 * @param file The path to a JPEG or PNG image file
 */
int ImageEditor::edit(const QString &file)
{
    path = file;
    QImageReader reader(file);
    format = reader.format().toUpper();
    QSize size = reader.size();
    oldWidth = size.width();
    oldHeight = size.height();
    int width = oldWidth;
    int height = oldHeight;
    // Scale the image down so as to not swamp devices with limited memory when
    // loaded (only works for JPEGs, big PNG images will be rejected)
    int scaleDenom = 1;
    while (width * height > 800 * 600) {
        scaleDenom *= 2;
        // I doubt if many people have 6400 x 4800 images, but it's probably
        // only a matter of time before digital cameras get there...
        if (scaleDenom > 8 || format != "JPEG") {
            QMessageBox::warning(this, qApp->applicationName(),
                                 tr("Image is too large to import"));
            return QDialog::Rejected;
        }
        width /= 2;
        height /= 2;
    }
    widthBox->setMaximum(width);
    widthBox->setValue(width);
    heightBox->setMaximum(height);
    heightBox->setValue(height);
    rotateBox->setCurrentIndex(0);
    reader.setScaledSize(QSize(width, height));
    image = reader.read();
    qApp->processEvents();
    QPixmap pm;
    pm.fromImage(image);
    display->setPixmap(pm);
    int margin = 5;
#if defined(Q_WS_WIN)
    margin += 16;
#endif
    resize(qMax(pm.width() + margin, paramsRow->sizeHint().width() + margin),
           paramsRow->height() + pm.height() + okCancelRow->height() + margin);
    return exec();
}

/**
 * Update the displayed image with any changes made to the settings (size,
 * rotation, etc.)
 */
void ImageEditor::updateImage()
{
    int width = widthBox->value();
    int height = heightBox->value();
    int angle = rotateBox->currentIndex() * 90;
    if (angle == 90 || angle == 270) {
        int temp = width;
        width = height;
        height = temp;
    }
    QImageReader reader(path);
    reader.setScaledSize(QSize(width, height));
    image = reader.read();
    if (angle) {
        QMatrix matrix;
        matrix.rotate(angle);
        image = image.transformed(matrix);
    }
    qApp->processEvents();
    QPixmap pm;
    pm.fromImage(image);
    display->setPixmap(pm);
}

/**
 * Get the image as it's currently displayed.
 *
 * @return The currently shown image
 */
QImage ImageEditor::getImage()
{
    return image;
}

/**
 * Get the path to the file from which the current image was loaded.
 *
 * @return The path to the image file
 */
QString ImageEditor::getFile()
{
    return path;
}

/**
 * A string representing the original format of the current image, typically
 * either "JPEG" or "PNG".
 *
 * @return The short name of the current image's format
 */
QString ImageEditor::getFormat()
{
    return format;
}

/**
 * Determine if any changes to the image have been specified.
 *
 * @return True if the image has been modified, false otherwise
 */
bool ImageEditor::isModified()
{
    if (rotateBox->currentIndex() != 0) {
        return true;
    }
    if (widthBox->value() != oldWidth || heightBox->value() != oldHeight) {
        return true;
    }
    return false;
}
