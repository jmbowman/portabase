/*
 * crypto.h
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef CRYPTO_H
#define CRYPTO_H

#include <beecrypt/beecrypt.h>
#include <mk4.h>
#include <qstring.h>

class Crypto
{
public:
    Crypto(c4_Storage *outer, c4_Storage *inner);
    ~Crypto();

    QString open();
    void save();
    QString setPassword(const QString &pass, bool newPass);
    QString changePassword(const QString &oldPass, const QString &newPass);

private:
    byte *encrypt(byte *data, int size, int *resultSize);
    byte *decrypt(byte *data, int size, int *resultSize, bool *passError);
    byte *pad(byte *data, int dataSize, int *newSize);
    int unpaddedLength(byte *data, int size);
    void printBytes(QString label, byte *data, int size);

private:
    QString password;
    mp32number passHash;
    mp32number dataHash;
    const blockCipher *bcipher;
    const hashFunction *hashfunc;
    const randomGenerator *prng;
    void *encryptParam;
    void *decryptParam;
    hashFunctionContext hfc;
    randomGeneratorContext rgc;
    byte *initVector;
    c4_Storage *container;
    c4_Storage *content;
    // _crypto view and columns
    c4_View crypto;
    c4_BytesProp crIv;
    c4_BytesProp crHash;
    c4_BytesProp crData;
};

#endif
