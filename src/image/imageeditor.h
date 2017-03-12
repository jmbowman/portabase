/*
 * imageeditor.h
 *
 * (c) 2003-2004,2008-2009,2015,2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file imageeditor.h
 * Header file for ImageEditor
 */

#ifndef IMAGEEDITOR_H
#define IMAGEEDITOR_H

#include <QImage>
#include "../pbdialog.h"
#include "../qqutil/qqspinbox.h"

class ImageWidget;
class QComboBox;
class QDialogButtonBox;
class QWidget;

/**
 * Dialog for editing the properties of an image that will be stored in a
 * %PortaBase file.  The image can be resized and/or rotated before saving
 * it in the database.  Currently a maximum size of 800x600 is enforced (the
 * Sharp Zaurus can't display anything much bigger than that before running
 * out of memory).
 */
class ImageEditor: public PBDialog
{
    Q_OBJECT
public:
    explicit ImageEditor(QWidget *parent = 0);

    int edit(const QString &file);
    QImage getImage();
    QString getFormat();
    bool isModified();

private slots:
    void updateImage();

private:
    QWidget *paramsRow; /**< The row of image parameter widgets */
    QDialogButtonBox *okCancelRow; /**< The OK/Cancel buttons row */
    QQSpinBox *widthBox; /**< Image width entry field */
    QQSpinBox *heightBox; /**< Image height entry field */
    QComboBox *rotateBox; /**< Rotation angle selection widget */
    ImageWidget *display; /**< Widget that displays the current image */
    QImage image; /**< The current image, as last loaded/refreshed */
    QString path; /**< Path to the current image's source file */
    QString format; /**< Abbreviation of the image's file format */
    int oldWidth; /**< The current image's original width */
    int oldHeight; /**< The current image's original height */
};

#endif
