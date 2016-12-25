/*
 * bytestream.cpp
 *
 * (c) 2003,2008,2016 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file bytestream.cpp
 * Source file for ByteStream
 */

#include <qstring.h>
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include "bytestream.h"

/**
 * Constructor for reading.
 *
 * @param data The byte array to read from
 */
ByteStream::ByteStream(const QByteArray &data) : content(data), location(0)
{

}

/**
 * Constructor for writing.
 */
ByteStream::ByteStream() : location(0)
{

}

/**
 * Destructor.
 */
ByteStream::~ByteStream()
{

}

/**
 * Fetch some bytes sequentially.
 *
 * @param buffer A pointer to the location to begin copying bytes to
 * @param length The maximum number of bytes to copy
 * @return The number of bytes actually copied
 */
int ByteStream::Read(void *buffer, int length)
{
    if (content.size() == 0) {
        return 0;
    }
    int size = length;
    if (location + length > content.size()) {
        size = content.size() - location;
    }
    memcpy(buffer, content.data() + location, size);
    location += size;
    return size;
}

/**
 * Store some bytes sequentially.
 *
 * @param buffer A pointer to the byte array to copy from
 * @param length The number of bytes to copy
 * @return True if the bytes were successfully stored
 */
bool ByteStream::Write(const void *buffer, int length)
{
    content.append(QByteArray((const char*)buffer, length));
    location += length;
    return true;
}

/**
 * Get the written content.
 *
 * @return The written content
 */
QByteArray ByteStream::getContent()
{
    return content;
}
