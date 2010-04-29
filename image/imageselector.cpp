/*
 * imageselector.cpp
 *
 * (c) 2003-2004,2008-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file imageselector.cpp
 * Source file for ImageSelector
 */

#include <QApplication>
#include <QBuffer>
#include <QFile>
#include <QImage>
#include <QMessageBox>
#include <QPushButton>
#include "../database.h"
#include "../factory.h"
#include "imageeditor.h"
#include "imageselector.h"
#include "imageutils.h"
#include "imageviewer.h"
#include "importdialog.h"

/**
 * Constructor.
 *
 * @param dbase The database being edited
 * @param parent This widget's parent widget
 */
ImageSelector::ImageSelector(Database *dbase, QWidget *parent)
    : QStackedWidget(parent), db(dbase), format(""), changed(false)
{
    selectButton = new QPushButton(tr("Select an image"), this);
    addWidget(selectButton);
    connect(selectButton, SIGNAL(clicked()), this, SLOT(selectImage()));
    setMaximumHeight(selectButton->sizeHint().height());

    editButtons = new QWidget(this);
    QHBoxLayout *layout = Factory::hBoxLayout(editButtons, true);
    QPushButton *viewButton = new QPushButton(tr("View"), editButtons);
    layout->addWidget(viewButton);
    connect(viewButton, SIGNAL(clicked()), this, SLOT(viewImage()));
    QPushButton *changeButton = new QPushButton(tr("Change"), editButtons);
    layout->addWidget(changeButton);
    connect(changeButton, SIGNAL(clicked()), this, SLOT(selectImage()));
    QPushButton *deleteButton = new QPushButton(tr("Delete"), editButtons);
    layout->addWidget(deleteButton);
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteImage()));
    addWidget(editButtons);
    setCurrentWidget(selectButton);
}

/**
 * Specify the field that this widget is to represent.
 *
 * @param row The ID of the row containing the field (-1 if new)
 * @param column The name of the column containing the field
 */
void ImageSelector::setField(int row, const QString &column)
{
    rowId = row;
    colName = column;
    if (rowId == -1) {
        changed = true;
    }
}

/**
 * Get the format of the currently selected image.  Valid values are "" (if
 * no image has been selected), "JPEG", or "PNG".
 *
 * @return The format of the selected image
 */
QString ImageSelector::getFormat()
{
    return format;
}

/**
 * Set the format of the currently selected image.  Called by the RowEditor so
 * that the format of image data already in the database is known.
 *
 * @param newFormat The format of the image data in this field
 */
void ImageSelector::setFormat(const QString &newFormat)
{
    format = newFormat;
    if (!format.isEmpty()) {
        setCurrentWidget(editButtons);
    }
}

/**
 * Select a file from which to load an image.  Called when the "Select an
 * image" button is clicked.
 */
void ImageSelector::selectImage()
{
    ImportDialog dialog(ImportDialog::Image, db, this);
    if (dialog.exec()) {
        QString file = dialog.getPath();
        ImageEditor editor(this);
        if (!editor.edit(file)) {
            return;
        }
        format = editor.getFormat();
        image = editor.getImage();
        path = editor.isModified() ? QString("") : file;
        changed = true;
        setCurrentWidget(editButtons);
    }
}

/**
 * Launch a dialog to display the currently selected image.  Called when the
 * "View" button is clicked.
 */
void ImageSelector::viewImage()
{
    if (image.isNull()) {
        image = ImageUtils::load(db, rowId, colName, format);
    }
    ImageViewer viewer(false, this);
    viewer.setImage(image);
    viewer.exec();
}

/**
 * Delete the currently selected image from the current database field,
 * setting it back to "no image selected".
 */
void ImageSelector::deleteImage()
{
    int choice = QMessageBox::warning(this, qApp->applicationName(),
                                      tr("Delete the current image?"),
                                      QMessageBox::Yes|QMessageBox::No,
                                      QMessageBox::No);
    if (choice != QMessageBox::Yes) {
        return;
    }
    image = QImage();
    format = "";
    changed = true;
    setCurrentWidget(selectButton);
}

/**
 * Save the field's image data to the database.
 *
 * @param id The ID of the row being saved to
 */
void ImageSelector::saveImage(int id)
{
    if (!changed) {
        return;
    }
    if (!path.isEmpty()) {
        // The image wasn't modified, so just copy the file data
        QFile file(path);
        file.open(QIODevice::ReadOnly);
        QByteArray bytes = file.readAll();
        file.close();
        db->setBinaryField(id, colName, bytes);
    }
    else if (!format.isEmpty()) {
        // The image was modified, so need to encode the in-memory image data
        QByteArray bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, format.toLower().toLatin1());
        buffer.close();
        db->setBinaryField(id, colName, bytes);
    }
}
