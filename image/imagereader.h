/*
 * imagereader.h
 *
 * Changes (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * Original source:
************************************************************************
** TxImage - Image Viewer
** Copyright (C) 2003
** Pierpaolo Di Panfilo <pippo_dp@libero.it>
** Cristian Di Panfilo <cridipan@virgilio.it>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.  To obtain a
** copy of the GNU Library General Public License, write to the Free
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
**
*************************************************************************/


#ifndef _IMAGEREADER_H
#define _IMAGEREADER_H


#include <stdio.h>
extern "C" {
#define XMD_H   // get around typedef mismatch between libjpeg and Qt
#include <jpeglib.h>
#ifdef const
#undef const
#endif
}

#include <qimage.h>

class QString;

class JPEGReader
{
public:
    JPEGReader(const char *fileName);
    virtual ~JPEGReader();
    virtual int imageWidth();
    virtual int imageHeight();
    virtual QImage readImageData();
    void setScaling(int denom);

protected:
    FILE *jpegFile;
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    int scaleDenom;
    QImage image;
};


class ImageReader
{
public:
    ImageReader();
    ImageReader(QString file);
    ~ImageReader();
    void reset();
    void load(QString file, bool dimension=0);
    int imageWidth();
    int imageHeight();
    QString format();
    QSize scaledSize(int w, int h);
    QImage getImage(QSize size, bool &rotate, int degree=0, bool bestFit=0);
    QImage getImage(QSize size);
    QImage getImage(int rectw, int recth);
    QImage getBestFitImage(QSize size, bool &rotated);
    QImage getBestFitImage(QSize size);
    QImage getRotatedImage(QSize size, int degree);
    QImage getRotatedImage(int w, int h, int degree);
    QImage rotateImage(QImage img, int degree);

private:
    bool isJPEG;
    QString path;
    QImage image;
    JPEGReader *JPEGreader;
};

#endif
