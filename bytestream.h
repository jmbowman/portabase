/*
 * bytestream.h
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef BYTESTREAM_H
#define BYTESTREAM_H

#include <mk4.h>

class ByteStream: public c4_Stream
{
public:
    ByteStream(void *data, int size);
    ByteStream();
    ~ByteStream();

    int Read(void*, int);
    bool Write(const void*, int);
    t4_byte *getContent(int *size);

private:
    t4_byte *content;
    int contentSize;
    int location;
    bool writing;
};

#endif
