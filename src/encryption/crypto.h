/*
 * crypto.h
 *
 * (c) 2003-2004,2008-201,20150 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file crypto.h
 * Header file for Crypto
 */

#ifndef CRYPTO_H
#define CRYPTO_H

#include <mk4.h>

#include <QByteArray>
#include <QObject>
#include <QString>
#include "blowfish.h"
extern "C" {
    #include "randomkit/rk_isaac.h"
}

/**
 * The heart of PortaBase's file encryption support.  An encrypted PortaBase
 * file is still a standard Metakit file, but most of the usual tables are
 * absent; instead, a field in the global properties table indicates that the
 * file is encrypted, and a separate table includes:
 *
 * <ul>
 * <li>A Blowfish-encrypted byte array of the actual database content</li>
 * <li>The initialization vector which was used in the CBC mode encryption
 *     process</li>
 * <li>The SHA-1 hash of the original data, used to verify if it has been
 *     decrypted correctly</li>
 * </ul>
 *
 * The database content is worked with as an in-memory database, and only
 * saved to the actual file in encrypted form.
 */
class Crypto : public QObject
{
    Q_OBJECT
public:
    Crypto(c4_Storage *outer, c4_Storage *inner);

    QString open(int version);
    void save();
    QString setPassword(const QString &pass, bool newPass);
    QString changePassword(const QString &oldPass, const QString &newPass);

private:
    QByteArray pad(const QByteArray &data);
    QByteArray encrypt(const QByteArray &data);
    QByteArray decrypt(const QByteArray &data, bool *passError);
    int unpaddedLength(const QByteArray &data);
    void invertIfNecessary(QByteArray dataHash, int version);
    inline quint32 swapu32(quint32 n)
    {
        return (    ((n & 0xffU) << 24) |
                ((n & 0xff00U) << 8) |
                ((n & 0xff0000U) >> 8) |
                ((n & 0xff000000U) >> 24) );
    }

private:
    static rk_isaac_state prngState;
    static bool prngInitialized;

private:
    QString password; /**< The password last given by the user */
    QByteArray dataHash; /**< The SHA-1 hash of the data before encryption */
    QByteArray initVector; /**< The CBC mode initialization vector */
    Blowfish blowfish; /**< The Blowfish block cipher */
    c4_Storage *container; /**< The Metakit storage object for the actual file */
    c4_Storage *content; /**< The Metakit storage object for the encrypted content */
    c4_View crypto; /**< The _crypto view in the Metakit file */
    c4_BytesProp crIv; /**< The _criv column in the Metakit file (initialization vector) */
    c4_BytesProp crHash; /**< The _crhash column in the Metakit file (data hash) */
    c4_BytesProp crData; /**< The _crdata column in the Metakit file (encrypted data) */
};

#endif
