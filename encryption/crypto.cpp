/*
 * crypto.cpp
 *
 * (c) 2003-2004,2008 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file crypto.cpp
 * Source file for Crypto
 */

#include <stdio.h>
#include <stdlib.h>

#include <QCryptographicHash>
#include "bytestream.h"
#include "crypto.h"

/**
 * Constructor.
 *
 * @param outer Metakit storage object representing the actual PortaBase file
 * @param inner Metakit storage object representing the encrypted content
 */
Crypto::Crypto(c4_Storage *outer, c4_Storage *inner) : QObject(),
  container(outer), content(inner), crIv("_criv"), crHash("_crhash"),
  crData("_crdata")
{
    if (!Crypto::prngInitialized) {
        rk_isaac_randomseed(&Crypto::prngState);
        Crypto::prngInitialized = true;
    }
    crypto = container->GetAs("_crypto[_criv:B,_crhash:B,_crdata:B]");
}

rk_isaac_state Crypto::prngState;
bool Crypto::prngInitialized = false;

/**
 * Destructor.
 */
Crypto::~Crypto()
{

}

/**
 * Attempt to open the encrypted content in a PortaBase file of the specified
 * version.
 *
 * @param version The file format version of the PortaBase file being opened
 * @return An error message if unsuccessful, or an empty string otherwise
 */
QString Crypto::open(int version)
{
    // load raw data from the file
    c4_Bytes ivField = crIv (crypto[0]);
    initVector = QByteArray((const char*)ivField.Contents(), ivField.Size());

    c4_Bytes hashField = crHash (crypto[0]);
    dataHash = QByteArray((const char*)hashField.Contents(), hashField.Size());
    invertIfNecessary(dataHash, version);

    c4_Bytes dataField = crData (crypto[0]);
    QByteArray data((const char*)dataField.Contents(), dataField.Size());

    // decrypt the data
    bool passError = false;
    QByteArray decrypted = decrypt(data, &passError);
    if (passError) {
        return tr("Incorrect password");
    }
    if (decrypted.size() <= 0) {
        return tr("Error in decrypting data");
    }

    // load the decrypted data into the storage object
    ByteStream input(decrypted);
    if (!content->LoadFrom(input)) {
        return tr("Error in loading data");
    }
    return "";
}

/**
 * Encrypt the current content of the in-memory database and save it to file.
 */
void Crypto::save()
{
    // serialize the storage object to a byte array
    ByteStream output;
    content->SaveTo(output);
    QByteArray rawData = output.getContent();

    // encrypt the data
    QByteArray encrypted = encrypt(rawData);

    // delete any old data from the file, add the new data
    if (crypto.GetSize() > 0) {
        crypto.RemoveAt(0);
    }
    c4_Row row;
    crIv (row) = c4_Bytes(initVector.data(), initVector.size());
    crHash (row) = c4_Bytes(dataHash.data(), dataHash.size());
    crData (row) = c4_Bytes(encrypted.data(), encrypted.size());
    crypto.Add(row);
}

/**
 * Set the password required to open the encrypted content.  Used when
 * creating a new file or opening an existing one (and used by
 * changePassword() when changing the password for an existing file).
 *
 * @param pass The password (or at least what the user believes it to be)
 * @param newPass True for a new password, false if just opening a file
 * @return An error message if the password is deemed inadequate, an empty
 *         string otherwise
 */
QString Crypto::setPassword(const QString &pass, bool newPass)
{
    if (newPass && pass.length() < 6) {
        return tr("Password must be at least 6 characters long");
    }
    password = pass;
    QByteArray utf8pass = pass.toUtf8();
    QByteArray passHash = QCryptographicHash::hash(utf8pass,
                                                   QCryptographicHash::Sha1);
    blowfish.setup(passHash);
    return "";
}

/**
 * Change the password of an existing file.  Requires the existing password to
 * be given correctly in order to succeed.
 *
 * @param oldPass The value entered by the user as the current password
 * @param newPass The new desired password
 * @return An error message if the operation failed, an empty string otherwise
 */
QString Crypto::changePassword(const QString &oldPass, const QString &newPass)
{
    if (oldPass != password) {
        return tr("Incorrect password");
    }
    return setPassword(newPass, true);
}

/**
 * Pad the end of the provided data so that the resulting size is an even
 * multiple of the Blowfish algorithm's block size.  A byte value of 1 is
 * always appended, followed by enough zero bytes to reach the desired size.
 *
 * @param data The data to be padded
 * @return The data with any necessary padding added
 */
QByteArray Crypto::pad(const QByteArray &data)
{
    int dataSize = data.size();
    if (dataSize <= 0) {
        return QByteArray();
    }
    int padding = Blowfish::blockSize - ((dataSize + 1) % Blowfish::blockSize);
    if (padding == (int)Blowfish::blockSize) {
        padding = 0;
    }
    QByteArray result(data);
    result.append(1);
    if (padding > 0) {
        result.append(QByteArray(padding, 0));
    }
    return result;
}

/**
 * Encrypt the provided data.
 *
 * @param data The data to be encrypted
 * @return The encrypted data
 */
QByteArray Crypto::encrypt(const QByteArray &data)
{
    if (data.size() <= 0) {
        return QByteArray();
    }

    // pad out to an even number of blocks
    QByteArray padded = pad(data);
    if (padded.size() <= 0) {
        return QByteArray();
    }

    // get the hash value of the padded data for later verification
    dataHash = QCryptographicHash::hash(padded, QCryptographicHash::Sha1);

    // set the CBC initialization vector
    initVector.resize(Blowfish::blockSize);
    rk_isaac_fill(initVector.data(), initVector.size(), &Crypto::prngState);

    // encrypt the data
    return blowfish.encrypt(padded, initVector);
}

/**
 * Decrypt the provided data.  If the SHA-1 hash of the result doesn't match
 * the one stored for the original data before encryption, the boolean
 * addressed by passError is set to true; this indicates that the supplied
 * password was incorrect.
 *
 * @param data The data to be decrypted
 * @param passError The address of a boolean used to indicate a password error
 * @return The decrypted data
 */
QByteArray Crypto::decrypt(const QByteArray &data, bool *passError)
{
    *passError = false;
    if (data.size() <= 0) {
        return QByteArray();
    }

    // decrypt the data
    QByteArray result = blowfish.decrypt(data, initVector);

    // calculate and test the hash value of the decrypted data
    QByteArray testHash = QCryptographicHash::hash(result,
                                                   QCryptographicHash::Sha1);
    if (testHash != dataHash) {
        *passError = true;
        return QByteArray();
    }

    // remove any padding and return the result
    int resultSize = unpaddedLength(result);
    result.resize(resultSize);
    return result;
}

/**
 * Determine how many bytes of the provided array are actual data, after
 * removing any padding which was added to the end (in order to achieve a
 * suitable size for Blowfish encryption).
 *
 * @param data The data to be inspected
 * @return The number of bytes of actual data
 */
int Crypto::unpaddedLength(const QByteArray &data)
{
    if (data.size() <= 0) {
        return 0;
    }
    int result = data.size();
    // skip zeroes used as padding
    while (data.at(result - 1) == (char)0 && result > 1) {
        result--;
    }
    // skip the "1" used to mark the start of the padding
    result--;
    return result;
}

/**
 * Print the given label to the console, followed by a hexadecimal
 * representation of the given byte array.  Used for debugging purposes.
 *
 * @param label The text label to send to the console
 * @param data The binary data to be displayed
 */
void Crypto::printBytes(QString label, const QByteArray &data)
{
    printf((label + ":\n").toLocal8Bit().data());
    printf(data.toHex() + "\n");
}

/**
 * For files generated using an older version of the encryption code, reverse
 * the endianness of the supplied SHA-1 hash of the data which was stored in
 * the file.  Applies for file format versions 9 and lower.
 *
 * @param dataHash The data to be potentially converted
 * @param version The format version number of the PortaBase file being used
 */
void Crypto::invertIfNecessary(QByteArray dataHash, int version)
{
    // upgraded to Beecrypt 3.0.0 in PortaBase 1.8 / file version 10
    if (version >= 10) {
        return;
    }
    quint32 *temp = (quint32*)dataHash.data();
    int count = dataHash.size() / sizeof(quint32);
    for (int i = 0; i < count; i++) {
        temp[i] = swapu32(temp[i]);
    }
}
