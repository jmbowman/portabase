/*
 * bytestream.h
 *
 * (c) 2003,2008 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file bytestream.h
 * Header file for ByteStream
 */

#ifndef BYTESTREAM_H
#define BYTESTREAM_H

#include <mk4.h>
#include <QByteArray>

/**
 * This class allows Metakit storage objects to be loaded from and saved to
 * byte arrays.  Used in encrypted databases, where the entire database
 * is saved to such a byte array, encrypted, and stored in a wrapper Metakit
 * file.
 */
class ByteStream: public c4_Stream
{
public:
    ByteStream(const QByteArray &data);
    ByteStream();
    ~ByteStream();

    int Read(void*, int);
    bool Write(const void*, int);
    QByteArray getContent();

private:
    QByteArray content; /**< The stored byte array */
    int location; /**< The current read or write location in the content */
    bool writing; /**< True if the content is being written to */
};

#endif
