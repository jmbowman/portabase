/*
 * imageselector.cpp
 *
 * (c) 2003-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qbuffer.h>
#include <qcstring.h>
#include <qfile.h>
#include <qhbox.h>
#include <qimage.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <stdlib.h>
#include "../database.h"
#include "../qqdialog.h"
#include "imageeditor.h"
#include "imageselector.h"
#include "imageutils.h"
#include "imageviewer.h"

#if !defined(Q_WS_QWS)
#include "../desktop/importdialog.h"
#else
#if defined(SHARP)
#include "../sharp/importdialog.h"
#else
#include "../importdialog.h"
#endif
#endif

ImageSelector::ImageSelector(Database *dbase, QWidget *parent, const char *name)
    : QWidgetStack(parent, name), db(dbase), format(""), changed(FALSE)
{
    selectButton = new QPushButton(tr("Select an image"), this);
    connect(selectButton, SIGNAL(clicked()), this, SLOT(selectImage()));
    int height = selectButton->height();
    setMaximumHeight(height);

    editButtons = new QHBox(this);
    QPushButton *viewButton = new QPushButton(tr("View"), editButtons);
    connect(viewButton, SIGNAL(clicked()), this, SLOT(viewImage()));
    QPushButton *changeButton = new QPushButton(tr("Change"), editButtons);
    connect(changeButton, SIGNAL(clicked()), this, SLOT(selectImage()));
    QPushButton *deleteButton = new QPushButton(tr("Delete"), editButtons);
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteImage()));
    raiseWidget(selectButton);
}

ImageSelector::~ImageSelector()
{

}

void ImageSelector::setField(int row, const QString &column)
{
    rowId = row;
    colName = column;
    if (rowId == -1) {
        changed = TRUE;
    }
}

QString ImageSelector::getFormat()
{
    return format;
}

void ImageSelector::setFormat(const QString &newFormat)
{
    format = newFormat;
    if (format != "") {
        raiseWidget(editButtons);
    }
}

void ImageSelector::selectImage()
{
    ImportDialog dialog(IMAGE_FILE, db, this);
    if (dialog.exec()) {
        QString file = dialog.getPath();
        ImageEditor editor(this);
        if (!editor.edit(file)) {
            return;
        }
        format = editor.getFormat();
        image = editor.getImage();
        path = editor.isModified() ? QString("") : file;
        changed = TRUE;
        raiseWidget(editButtons);
    }
}

void ImageSelector::viewImage()
{
    if (image.isNull()) {
        image = ImageUtils::load(db, rowId, colName, format);
    }
    ImageViewer viewer(FALSE, this);
    viewer.setImage(image);
    viewer.exec();
}

void ImageSelector::deleteImage()
{
    int ok = QMessageBox::warning(this, QQDialog::tr("PortaBase"),
                                  tr("Delete the current image?"),
                                  QObject::tr("Yes"), QObject::tr("No"),
                                  QString::null, 1);
    if (ok == 1) {
        return;
    }
    image.reset();
    format = "";
    changed = TRUE;
    raiseWidget(selectButton);
}

void ImageSelector::saveImage(int id)
{
    if (!changed) {
        return;
    }
    if (path != "") {
        QFile file(path);
        int size = file.size();
        char *data = (char*)malloc(size);
        file.open(IO_ReadOnly);
        int position = 0;
        while (!file.atEnd()) {
            position += file.readBlock(data + position, 1024);
        }
        file.close();
        db->setBinaryField(id, colName, data, size);
        if (data) {
            free(data);
        }
    }
    else if (format != "") {
        QByteArray array;
        QBuffer buffer(array);
        QImageIO iio(&buffer, format);
        iio.setImage(image);
        buffer.open(IO_WriteOnly);
        iio.write();
        buffer.close();
        db->setBinaryField(id, colName, array.data(), array.size());
    }
}
