/*
 * imageeditor.h
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef IMAGEEDITOR_H
#define IMAGEEDITOR_H

#include <qimage.h>
#include "../pbdialog.h"

class ImageWidget;
class QComboBox;
class QHBox;
class QSpinBox;

class ImageEditor: public PBDialog
{
    Q_OBJECT
public:
    ImageEditor(QWidget *parent = 0, const char *name = 0, WFlags f = 0);
    ~ImageEditor();

    int edit(const QString &file);
    QImage getImage();
    QString getFile();
    QString getFormat();
    bool isModified();

private slots:
    void updateImage();

private:
    QHBox *paramsRow;
    QSpinBox *widthBox;
    QSpinBox *heightBox;
    QComboBox *rotateBox;
    ImageWidget *display;
    QImage image;
    QString path;
    QString format;
    int oldWidth;
    int oldHeight;
};

#endif
