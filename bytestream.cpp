/*
 * bytestream.cpp
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qstring.h>
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include "bytestream.h"

// Constructor for reading
ByteStream::ByteStream(void *data, int size) : content((t4_byte*)data), contentSize(size), location(0), writing(false)
{

}

// Constructor for writing
ByteStream::ByteStream() : content(0), contentSize(0), location(0), writing(true)
{

}

ByteStream::~ByteStream()
{
    // written content not freed here...must be done by caller of getContent()
}

int ByteStream::Read(void *buffer, int length)
{
    if (content == 0) {
        return 0;
    }
    int size = length;
    if (location + length > contentSize) {
        size = contentSize - location;
    }
    memcpy(buffer, content + location, size);
    location += size;
    return size;
}

bool ByteStream::Write(const void *buffer, int length)
{
    contentSize += length;
    t4_byte *newContent = (t4_byte*)realloc(content, contentSize);
    if (!newContent) {
        return false;
    }
    content = newContent;
    memcpy(content + location, buffer, length);
    location += length;
    return true;
}

t4_byte *ByteStream::getContent(int *size)
{
    *size = contentSize;
    return content;
}
