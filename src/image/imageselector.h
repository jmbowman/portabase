/*
 * imageselector.h
 *
 * (c) 2003-2004,2008-2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file imageselector.h
 * Header file for ImageSelector
 */

#ifndef IMAGE_SELECTOR_H
#define IMAGE_SELECTOR_H

#include <QImage>
#include <QStackedWidget>

class Database;
class QPushButton;

/**
 * Widget that represents an image field in the RowEditor.  Starts off by
 * displaying a "Select an image" button, then once an image file is selected
 * changes to a row of three buttons: "View", "Change", and "Delete".
 * Clicking the "Delete" button reverts this widget back to its initial
 * state (the "Select an image" button).
 */
class ImageSelector: public QStackedWidget
{
    Q_OBJECT
public:
    ImageSelector(Database *dbase, QWidget *parent = 0);

    void setField(int row, const QString &column);
    void saveImage(int id);
    QString getFormat();
    void setFormat(const QString &newFormat);

private slots:
    void selectImage();
    void viewImage();
    void deleteImage();

private:
    Database *db; /**< The database being edited */
    int rowId; /**< The ID of the row being edited */
    QString colName; /**< The name of the column being edited */
    QWidget *editButtons; /**< The row of View/Change/Delete buttons */
    QPushButton *selectButton; /**< The "Select an image" button */
    QImage image; /**< The currently selected image (may not be loaded yet) */
    QString path; /**< The path to the selected image file */
    QString format; /**< Format of the selected image: "JPEG" or "PNG" */
    bool changed; /**< True if the image has been changed or deleted */
};

#endif
