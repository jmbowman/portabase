/*
 * crypto.cpp
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qobject.h>
#include <stdio.h>
#include <stdlib.h>
#include "bytestream.h"
#include "crypto.h"
#include <beecrypt/blockmode.h>

Crypto::Crypto(c4_Storage *outer, c4_Storage *inner) : container(outer), content(inner), crIv("_criv"), crHash("_crhash"), crData("_crdata")
{
    bcipher = blockCipherFind("Blowfish");
    hashfunc = hashFunctionFind("SHA-1");
    prng = randomGeneratorFind("FIPS 186");
    hashFunctionContextInit(&hfc, hashfunc);
#if !defined(Q_WS_WIN)
    // don't block on /dev/random waiting for enough entropy...
    setenv("BEECRYPT_ENTROPY", "urandom", 1);
#endif
    randomGeneratorContextInit(&rgc, prng);
    encryptParam = malloc(bcipher->paramsize);
    decryptParam = malloc(bcipher->paramsize);
    initVector = (byte*)malloc(bcipher->blocksize);
    crypto = container->GetAs("_crypto[_criv:B,_crhash:B,_crdata:B]");
}

Crypto::~Crypto()
{
    hashFunctionContextFree(&hfc);
    randomGeneratorContextFree(&rgc);
    free(encryptParam);
    free(decryptParam);
    free(initVector);
}

QString Crypto::open()
{
    // load raw data from the file
    c4_Bytes ivField = crIv (crypto[0]);
    memcpy(initVector, ivField.Contents(), ivField.Size());
    c4_Bytes hashField = crHash (crypto[0]);
    mp32ninit(&dataHash, (uint32)(hashField.Size() / sizeof(uint32)),
              (uint32*)(hashField.Contents()));
    c4_Bytes dataField = crData (crypto[0]);

    // decrypt the data
    int resultSize = 0;
    bool passError = false;
    byte *decrypted = decrypt((byte*)(dataField.Contents()), dataField.Size(),
                              &resultSize, &passError);
    mp32nfree(&dataHash);
    if (passError) {
        return QObject::tr("Incorrect password");
    }
    if (!decrypted) {
        return QObject::tr("Error in decrypting data");
    }

    // load the decrypted data into the storage object
    ByteStream input(decrypted, resultSize);
    if (!content->LoadFrom(input)) {
        free(decrypted);
        return QObject::tr("Error in loading data");
    }
    free(decrypted);
    return "";
}

void Crypto::save()
{
    // serialize the storage object to a byte array
    ByteStream output;
    content->SaveTo(output);
    int rawSize;
    byte *rawData = (byte*)output.getContent(&rawSize);

    // encrypt the data
    int encryptedSize;
    byte *encrypted = encrypt(rawData, rawSize, &encryptedSize);

    // delete any old data from the file, add the new data
    if (crypto.GetSize() > 0) {
        crypto.RemoveAt(0);
    }
    c4_Row row;
    crIv (row) = c4_Bytes(initVector, bcipher->blocksize);
    crHash (row) = c4_Bytes(dataHash.data, dataHash.size * sizeof(uint32));
    crData (row) = c4_Bytes(encrypted, encryptedSize);
    crypto.Add(row);
    free(encrypted);
}

QString Crypto::setPassword(const QString &pass, bool newPass)
{
    if (newPass && pass.length() < 6) {
        return QObject::tr("Password must be at least 6 characters long");
    }
    password = pass;
    QCString utf8pass = pass.utf8();
    mp32nzero(&passHash);
    hashFunctionContextUpdate(&hfc, (const byte*)(utf8pass.data()),
                              utf8pass.length());
    hashFunctionContextDigest(&hfc, &passHash);
    bcipher->setup(encryptParam, passHash.data, hashfunc->digestsize * 8,
                   ENCRYPT);
    bcipher->setup(decryptParam, passHash.data, hashfunc->digestsize * 8,
                   DECRYPT);
    return "";
}

QString Crypto::changePassword(const QString &oldPass, const QString &newPass)
{
    if (oldPass != password) {
        return QObject::tr("Incorrect password");
    }
    return setPassword(newPass, TRUE);
}

byte *Crypto::encrypt(byte *data, int size, int *resultSize)
{
    if (!data || size <= 0) {
        return 0;
    }

    // pad out to an even number of blocks
    byte *padded = pad(data, size, resultSize);
    if (!padded) {
        free(data);
        return 0;
    }

    // get the hash value of the padded data for later verification
    mp32nzero(&dataHash);
    hashFunctionContextUpdate(&hfc, padded, *resultSize);
    hashFunctionContextDigest(&hfc, &dataHash);

    // prepare storage for the encrypted data
    byte *result = (byte*)malloc(*resultSize);
    if (!result) {
        free(padded);
        return 0;
    }

    // set the CBC initialization vector
    randomGeneratorContextNext(&rgc, (uint32*)initVector,
                               (bcipher->blocksize) / sizeof(uint32));
    bcipher->setiv(encryptParam, (uint32*)initVector);

    // encrypt the data
    int blockCount = (*resultSize) / (bcipher->blocksize);
    blockEncrypt(bcipher, encryptParam, CBC, blockCount, (uint32*)result,
                 (uint32*)padded);
    free(padded);
    return result;
}

byte *Crypto::decrypt(byte *data, int size, int *resultSize, bool *passError)
{
    *passError = false;
    if (!data || size <= 0) {
        return 0;
    }

    // prepare storage for the decrypted data
    byte *result = (byte*)malloc(size);
    if (!result) {
        return 0;
    }

    // decrypt the data
    int blockCount = size / (bcipher->blocksize);
    bcipher->setiv(decryptParam, (uint32*)initVector);
    blockDecrypt(bcipher, decryptParam, CBC, blockCount, (uint32*)result,
                 (uint32*)data);

    // calculate and test the hash value of the decrypted data
    mp32number testHash;
    mp32nzero(&testHash);
    hashFunctionContextUpdate(&hfc, result, size);
    hashFunctionContextDigest(&hfc, &testHash);
    if (memcmp(testHash.data, dataHash.data, testHash.size * sizeof(uint32))
            != 0) {
        *passError = true;
        free(result);
        return 0;
    }

    // find out how much of the result is padding and return the result
    *resultSize = unpaddedLength(result, size);
    return result;
}

byte *Crypto::pad(byte *data, int dataSize, int *newSize)
{
    if (!data || dataSize <= 0) {
        return 0;
    }
    int blockSize = bcipher->blocksize;
    int padding = blockSize - ((dataSize + 1) % blockSize);
    if (padding == blockSize) {
        padding = 0;
    }
    *newSize = dataSize + 1 + padding;
    byte *result = (byte*)realloc(data, *newSize);
    if (!result) {
        return 0;
    }
    *(result + dataSize) = (byte)1;
    int i;
    for (i = dataSize + 1; i < *newSize; i++) {
        *(result + i) = (byte)0;
    }
    return result;
}

int Crypto::unpaddedLength(byte *data, int size)
{
    if (!data || size <= 0) {
        return 0;
    }
    int result = size;
    // skip zeroes used as padding
    while (*(data + result - 1) == (byte)0 && result > 1) {
        result--;
    }
    // skip the "1" used to mark the start of the padding
    result--;
    return result;
}

void Crypto::printBytes(QString label, byte *data, int size)
{
    printf(label + ":\n");
    QString dataString("");
    for (int i = 0; i < size; i++) {
        dataString += QString::number((int)(data[i]), 16);
    }
    printf(dataString + "\n");
}
