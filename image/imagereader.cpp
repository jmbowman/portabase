/*
 * imagereader.cpp
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

#include <qdir.h>
#include <qfile.h>
#include <qimage.h>
#include <qsize.h>
#include "imagereader.h"
#include "metakitfuncs.h"

#if defined(Q_WS_WIN)
#include <fcntl.h>
#endif

JPEGReader::JPEGReader(const QString &fileName)
{
    scaleDenom = 1;

    // initialize decompression object
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    // open file
#if defined(Q_WS_WIN)
    int fd = windowsFileOpen(fileName.utf8(), _O_BINARY | _O_RDONLY);
    if (fd != -1) {
        jpegFile = _fdopen(fd, "rb");
    }
#else
    jpegFile = fopen(QFile::encodeName(fileName), "rb");
#endif
    if (!jpegFile) {
        fprintf(stderr, "JPEGReader: Can't open file %s\n", fileName);
        return;
    }
    jpeg_stdio_src(&cinfo, jpegFile);

    // read image information and initialize everything
    jpeg_read_header(&cinfo, TRUE);
}

JPEGReader::~JPEGReader()
{
    // clean up
    jpeg_destroy_decompress(&cinfo);
    fclose(jpegFile);
}

void JPEGReader::setScaling(int denom)
{
    scaleDenom = denom;
}

int JPEGReader::imageWidth()
{
    return cinfo.image_width;
}

int JPEGReader::imageHeight()
{
    return cinfo.image_height;
}

QImage JPEGReader::readImageData()
{
    // choose decompression options
    cinfo.out_color_space = JCS_RGB;
    cinfo.quantize_colors = FALSE;
    cinfo.dct_method = JDCT_IFAST/*SLOW*/;   // optionally IFAST

    // pre-scaling in the JPEG reader is likely to increase decompression speed
    cinfo.scale_num = 1;
    cinfo.scale_denom = scaleDenom;

    cinfo.do_block_smoothing = FALSE;

    // initialize decompression and allocate scanline buffers
    jpeg_start_decompress(&cinfo);

    QImage image(cinfo.output_width, cinfo.output_height, 32);
    uchar** lines = image.jumpTable();
    while (cinfo.output_scanline < cinfo.output_height) {
        (void)jpeg_read_scanlines(&cinfo, lines + cinfo.output_scanline,
                                  cinfo.output_height);
    }
    (void) jpeg_finish_decompress(&cinfo);

    if (cinfo.output_components == 3) {
        // Expand 24->32 bpp.
        for (uint j = 0; j < cinfo.output_height; j++) {
            uchar *in = image.scanLine(j) + cinfo.output_width * 3;
            QRgb *out = (QRgb*)image.scanLine(j);
            for (uint i = cinfo.output_width; i--; ) {
                in -= 3;
                out[i] = qRgb(in[0], in[1], in[2]);
            }
        }
    }

    if (cinfo.density_unit == 1) {
        image.setDotsPerMeterX(int(100. * cinfo.X_density / 2.54));
        image.setDotsPerMeterY(int(100. * cinfo.Y_density / 2.54));
    }
    else if (cinfo.density_unit == 2) {
        image.setDotsPerMeterX(int(100. * cinfo.X_density));
        image.setDotsPerMeterY(int(100. * cinfo.Y_density));
    }

    return image;
}



ImageReader::ImageReader()
{
    JPEGreader = 0;
    isJPEG = FALSE;
}

ImageReader::ImageReader(QString file)
{
    ImageReader();
    load(file);
}

ImageReader::~ImageReader()
{

}

void ImageReader::reset()
{
    image.reset();
}

void ImageReader::load(QString file, bool dimension)
{
    path = file;
    //get file extension
    int pos = file.findRev('.');
    QString mimetype = file.right(file.length() - pos - 1);
    if (mimetype == "jpeg" || mimetype == "jpg") {
        //jpeg image
        isJPEG = TRUE;
        //qDebug("loading jpeg " + file);
        JPEGreader = new JPEGReader(QDir::convertSeparators(file));
    }
    else if (!dimension) {
        //qDebug("loading " + mimetype + " " + file);
        isJPEG = FALSE;
        QImage newImage(file);
        image = newImage;
    }
}

int ImageReader::imageWidth()
{
    if (isJPEG && JPEGreader) {
        return JPEGreader->imageWidth();
    }
    else {
        return image.width();
    }
}

int ImageReader::imageHeight()
{
    if (isJPEG && JPEGreader) {
        return JPEGreader->imageHeight();
    }
    else {
        return image.height();
    }
}

QString ImageReader::format()
{
    if (isJPEG) {
        return "JPEG";
    }
    return QImageIO::imageFormat(path);
}

QSize ImageReader::scaledSize(int width, int height)
{
    //the size of the image smooth scaled to width and height
    int imgWidth = imageWidth(), imgHeight = imageHeight();
    double hs = (double)height / (double)imgHeight;
    double ws = (double)width / (double)imgWidth;
    double scaleFactor = (hs > ws) ? ws : hs;
    int smoothW = (int)(scaleFactor * imgWidth);
    int smoothH = (int)(scaleFactor * imgHeight);

    return QSize(smoothW, smoothH);
}

 /******************************************************************
 **  This function scales the image to the argument size.         **
 **  size: the dimension of the rect where the image is shown     **
 **  rotate: the image is rotated of 'degree' degrees             **
 **  degree: only 90, 180 and 270                                 **
 **  bestFit: the image must be rotated for best fit rect         **
 **           (rotate get TRUE if rotated)                        **
 *******************************************************************/

QImage ImageReader::getImage(QSize size, bool &rotate, int degree,
                             bool bestFit)
{
    int imgWidth = imageWidth(), imgHeight = imageHeight();
    int rectW = size.width(), rectH = size.height();

    //qDebug("rotate=%d degree=%d bestFit=%d", rotate, degree, bestFit);
    if (rotate && (degree == 90 || degree == 270)) {
        rectH = size.width();   //invert width and height to rotate 90 or 270
        rectW = size.height();
    }

    //scale width/height
    size = scaledSize(rectW, rectH);
    if ((imgWidth < rectW) && (imgHeight < rectH)) {  //not scale
        size.setWidth(imgWidth);
        size.setHeight(imgHeight);
    }
    else if (bestFit) { //calculate only if larger than rect
        rotate = FALSE;
        //qDebug("normal: %dx%d", size.width(), size.height());
        QSize rotated = scaledSize(rectH, rectW); //size with rotation
        //qDebug("rotated: %dx%d", rotated.width(), rotated.height());
        if ((rotated.width() * rotated.height()) > (size.width() * size.height())) {
            rotate = TRUE;
            size = rotated;
        }
        if (rotate) {
            degree = 90;
        }
    }

    if (isJPEG) {
        //pre-scaling to increase performance (only jpegs)
        int scaledWidth, scaledHeight;
        static int jpegScaleFactors[4] = {1, 2, 4, 8};
        for (int i = 0; i < 4; i++) {
            scaledWidth = imgWidth / jpegScaleFactors[i];
            scaledHeight = imgHeight / jpegScaleFactors[i];
            scaledWidth += imgWidth % jpegScaleFactors[i] ? 1 : 0;
            scaledHeight += imgHeight % jpegScaleFactors[i] ? 1 : 0;

            if ((scaledWidth >= size.width()) && (scaledHeight >= size.height())) {
                JPEGreader->setScaling(jpegScaleFactors[i]);
            }
            else {
                break;
            }
        }
        image = JPEGreader->readImageData();

        delete JPEGreader;
    }

    if (image.isNull()) {
        return image;
    }

    if (image.size() != size) {
        QImage scaledImage(image.smoothScale(size.width(), size.height()));
        image = scaledImage;
    }

    if (rotate && degree) {
        image = rotateImage(image, degree);
    }

    //qDebug("returning image: %dx%d", image.width(), image.height());
    return image;
}

QImage ImageReader::getImage(int rectw, int recth)
{
    return getImage(QSize(rectw, recth)); //without rotation
}

QImage ImageReader::getImage(QSize size)
{
    bool rotate = FALSE;
    return getImage(size, rotate); //without rotation
}

QImage ImageReader::getBestFitImage(QSize size, bool &rotated)
{
    return getImage(size, rotated, 0, TRUE);
}

QImage ImageReader::getBestFitImage(QSize size)
{
    bool rotate = FALSE;
    return getImage(size, rotate, 0, TRUE);
}

QImage ImageReader::getRotatedImage(QSize size, int degree)
{
    bool rotate = TRUE;
    if (degree) {
        return getImage(size, rotate, degree);
    }
    else {
        return getImage(size);
    }
}

QImage ImageReader::getRotatedImage(int w, int h, int degree)
{
    return getRotatedImage(QSize(w, h), degree);
}

//rotate code courtesy of KDE project.

QImage ImageReader::rotateImage(QImage img, int degree)
{
    //rotate the image 90, 180 and 270 degrees
    QImage dest;
    int x, y;

    if (img.depth() > 8)
    {
        unsigned int *srcData, *destData;
        switch (degree)
        {
            case 90:
                dest.create(img.height(), img.width(), img.depth());
                for (y = 0; y < img.height(); ++y) {
                    srcData = (unsigned int *)img.scanLine(y);
                    for (x = 0; x < img.width(); ++x) {
                        destData = (unsigned int *)dest.scanLine(x);
                        destData[img.height() - y - 1] = srcData[x];
                    }
                }
                break;
            case 180:
                dest.create(img.width(), img.height(), img.depth());
                for (y = 0; y < img.height(); ++y) {
                    srcData = (unsigned int *)img.scanLine(y);
                    destData = (unsigned int *)dest.scanLine(img.height() - y - 1);
                    for (x = 0; x < img.width(); ++x) {
                        destData[img.width() - x - 1] = srcData[x];
                    }
                }
                break;
            case 270:
                dest.create(img.height(), img.width(), img.depth());
                for (y = 0; y < img.height(); ++y) {
                    srcData = (unsigned int *)img.scanLine(y);
                    for (x = 0; x < img.width(); ++x) {
                        destData = (unsigned int *)dest.scanLine(img.width() - x - 1);
                        destData[y] = srcData[x];
                    }
                }
                break;
            default:
                dest = img;
                break;
        }
    }
    else
    {
        unsigned char *srcData, *destData;
        unsigned int *srcTable, *destTable;
        switch (degree)
        {
            case 90:
                dest.create(img.height(), img.width(), img.depth());
                dest.setNumColors(img.numColors());
                srcTable = (unsigned int *)img.colorTable();
                destTable = (unsigned int *)dest.colorTable();
                for (x = 0; x < img.numColors(); ++x) {
                    destTable[x] = srcTable[x];
                }
                for (y = 0; y < img.height(); ++y) {
                    srcData = (unsigned char *)img.scanLine(y);
                    for (x = 0; x < img.width(); ++x) {
                        destData = (unsigned char *)dest.scanLine(x);
                        destData[img.height() - y - 1] = srcData[x];
                    }
                }
                break;
            case 180:
                dest.create(img.width(), img.height(), img.depth());
                dest.setNumColors(img.numColors());
                srcTable = (unsigned int *)img.colorTable();
                destTable = (unsigned int *)dest.colorTable();
                for (x = 0; x < img.numColors(); ++x) {
                    destTable[x] = srcTable[x];
                }
                for (y = 0; y < img.height(); ++y) {
                    srcData = (unsigned char *)img.scanLine(y);
                    destData = (unsigned char *)dest.scanLine(img.height() - y - 1);
                    for (x = 0; x < img.width(); ++x) {
                        destData[img.width() - x - 1] = srcData[x];
                    }
                }
                break;
            case 270:
                dest.create(img.height(), img.width(), img.depth());
                dest.setNumColors(img.numColors());
                srcTable = (unsigned int *)img.colorTable();
                destTable = (unsigned int *)dest.colorTable();
                for (x = 0; x < img.numColors(); ++x) {
                    destTable[x] = srcTable[x];
                }
                for (y = 0; y < img.height(); ++y) {
                    srcData = (unsigned char *)img.scanLine(y);
                    for (x = 0; x < img.width(); ++x) {
                        destData = (unsigned char *)dest.scanLine(img.width() - x - 1);
                        destData[y] = srcData[x];
                    }
                }
                break;
            default:
                dest = img;
                break;
        }
    }

    //qDebug("image rotated %d degrees", degree);
    image = dest;
    return dest;
}
