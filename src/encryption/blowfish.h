/*
 * blowfish.h
 *
 * Adapted from BeeCrypt: http://sourceforge.net/projects/beecrypt
 *
 * (c) 2008-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 * (c) 1999, 2000, 2002 Beeyond Software Holding BV
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/** @file blowfish.h
 * Blowfish block cipher header file.
 *
 * For more information on this blockcipher, see:
 * "Applied Cryptography", second edition,
 *  Bruce Schneier,
 *  Wiley & Sons.
 *
 * Also see http://www.counterpane.com/blowfish.html
 */

#ifndef _BLOWFISH_H
#define _BLOWFISH_H

#include <QByteArray>

#define BLOWFISHROUNDS	16
#define BLOWFISHPSIZE	(BLOWFISHROUNDS+2)

/**
 * Holds all the parameters necessary for the %Blowfish cipher.
 */
struct blowfishParam
{
	quint32 p[BLOWFISHPSIZE]; /**< Holds the key expansion. */
	quint32 s[1024]; /**< Holds the s-boxes. */
	quint32 fdback[2]; /**< Buffer to be used by block chaining or feedback modes. */
};

/**
 * Implementation of the %Blowfish block cipher using CBC mode.  Derived
 * from the implementation in BeeCrypt, refactored to include only the
 * parts needed by %PortaBase and to take advantage of C++ code organization
 * features.
 */
class Blowfish
{
public:
    Blowfish();
    
    int setup(const QByteArray &key);
    QByteArray encrypt(const QByteArray &data, const QByteArray &iv);
    QByteArray decrypt(const QByteArray &data, const QByteArray &iv);

public:
    static size_t blockSize; /** The size of one block of data, in bytes. */

private:
    int setup(blowfishParam *bp, const quint8 *key, size_t keybits);
    int setIV(blowfishParam *bp, const quint8 *iv);
    int encrypt(blowfishParam *bp, quint32 *dst, const quint32 *src);
    int decrypt (blowfishParam *bp, quint32 *dst, const quint32 *src);
    int blockEncryptCBC(quint32 *dst, const quint32 *src, unsigned int nblocks);
    int blockDecryptCBC(quint32 *dst, const quint32 *src, unsigned int nblocks);

private:
    blowfishParam encryptParam; /**< Encryption parameters struct */
    blowfishParam decryptParam; /**< Decryption parameters struct */
};

#endif
