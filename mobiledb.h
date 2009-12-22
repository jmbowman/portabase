/*
 * mobiledb.h
 *
 * (c) 2002,2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file mobiledb.h
 * Header file for MobileDBFile
 */

#ifndef MOBILEDB_H
#define MOBILEDB_H

#include <QStringList>
#include "pdbfile.h"

/**
 * Parser for MobileDB database files.
 */
class MobileDBFile : public PDBFile
{ 
public:
    MobileDBFile(const QString &f);
    bool read();
    int row_count();
    int col_count();
    const char* db_info(); 
    QStringList field_types();
    const int* field_lengths();
    QStringList row(int i);
    QStringList field_labels();

protected:
    bool verify_recordhdr(const unsigned char *raw_record);
    virtual bool readMobileDBHeader();
    int record_category(int i);

protected:
    int colcount; /**< The number of columns in the database */
    int rowcount; /**< The number of rows in the database */
    QStringList fieldlabels; /**< The column names */
    QStringList fieldtypes; /**< The column types (as written in the file) */
    int fieldlengths[20]; /**< The column display widths, in pixels */
};	

#endif
