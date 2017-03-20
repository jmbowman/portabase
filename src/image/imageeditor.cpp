/*
 * imageeditor.cpp
 *
 * (c) 2003-2004,2008-2010,2015-2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
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
#include <QWidget>
#include "imageeditor.h"
#include "imageutils.h"
#include "imagewidget.h"
#include "../qqutil/qqfactory.h"
#include "../qqutil/qqspinbox.h"

/**
 * Constructor.
 *
 * @param parent This dialog's parent widget
 */
ImageEditor::ImageEditor(QWidget *parent)
    : PBDialog(tr("Image Editor"), parent), path("")
{
    paramsRow = new QWidget(this);
    QHBoxLayout *hbox = QQFactory::hBoxLayout(paramsRow, true);
    vbox->addWidget(paramsRow);
    hbox->addWidget(new QLabel(tr("Width"), paramsRow));
    widthBox = new QQSpinBox(paramsRow);
    widthBox->setRange(1, 10000);
    hbox->addWidget(widthBox);
    hbox->addWidget(new QLabel(tr("Height"), paramsRow));
    heightBox = new QQSpinBox(paramsRow);
    heightBox->setRange(1, 10000);
    hbox->addWidget(heightBox);

    hbox->addWidget(new QLabel(tr("Rotate"), paramsRow));
    rotateBox = QQFactory::comboBox(paramsRow);
    hbox->addWidget(rotateBox);
    rotateBox->addItem("0");
    rotateBox->addItem("90");
    rotateBox->addItem("180");
    rotateBox->addItem("270");

    QPushButton *refreshButton = new QPushButton(tr("Refresh"), paramsRow);
    hbox->addWidget(refreshButton);
    connect(refreshButton, SIGNAL(clicked()), this, SLOT(updateImage()));

    QScrollArea *scroll = new QScrollArea(this);
    QQFactory::configureScrollArea(scroll);
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
    widthBox->setMaximum(width);
    widthBox->setValue(width);
    heightBox->setMaximum(height);
    heightBox->setValue(height);
    rotateBox->setCurrentIndex(0);
    image = ImageUtils::readImage(&reader);
    qApp->processEvents();
    QPixmap pm = QPixmap::fromImage(image);
    display->setPixmap(pm);
    int margin = 5;
#if defined(Q_OS_WIN)
    margin += 16;
#endif
    int okCancelRowHeight = 0;
    if (okCancelRow) {
        okCancelRowHeight = okCancelRow->height();
    }
    resize(qMax(pm.width() + margin, paramsRow->sizeHint().width() + margin),
           paramsRow->height() + pm.height() + okCancelRowHeight + margin);
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
    QImageReader reader(path);
    reader.setScaledSize(QSize(width, height));
    image = reader.read();
    if (angle) {
        QMatrix matrix;
        matrix.rotate(angle);
        image = image.transformed(matrix);
    }
    qApp->processEvents();
    QPixmap pm = QPixmap::fromImage(image);
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
