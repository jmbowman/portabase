/*
 * imageselector.h
 *
 * (c) 2003-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef IMAGE_SELECTOR_H
#define IMAGE_SELECTOR_H

#include <qimage.h>
#include <qwidgetstack.h>

class Database;
class QHBox;
class QPushButton;

class ImageSelector: public QWidgetStack
{
    Q_OBJECT
public:
    ImageSelector(Database *dbase, QWidget *parent = 0, const char *name = 0);
    ~ImageSelector();

    void setField(int row, const QString &column);
    void saveImage(int id);
    QString getFormat();
    void setFormat(const QString &newFormat);

private slots:
    void selectImage();
    void viewImage();
    void deleteImage();

private:
    Database *db;
    int rowId;
    QString colName;
    QHBox *editButtons;
    QPushButton *selectButton;
    QImage image;
    QString path;
    QString format;
    bool changed;
};

#endif
