/*
 * pdbfile.h
 *
 * (c) 2002,2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file pdbfile.h
 * Header file for PDBFile
 */

#ifndef PDBFILE_H
#define PDBFILE_H

#include <QString>
#include <sys/types.h>

class QFile;

/**
 * Parser for .pdb files (Palm OS application data).  Can be subclassed to
 * support specific Palm OS applications' data.
 */
class PDBFile
{
public:
    PDBFile(const QString &f);
    virtual ~PDBFile();
    virtual bool read();

protected:
    struct RecordInfo { 
        unsigned int record_data_offset;
        unsigned char record_attributes;
        unsigned int unique_id;
        unsigned int record_size;
    };
    bool readHeader();
    bool readRecordList();
    unsigned short toshort(unsigned char *v);		
    unsigned int toint(unsigned char *v);		
    time_t palm2unix_time(unsigned int p);

protected:
    QFile *fd; /**< The file being parsed */
    QString filename; /**< The path of the file being parsed */
    unsigned char pdb_name[33]; /**< Name field of the file header */
    unsigned short file_attributes; /**< File attributes field of the file header */
    unsigned short version; /**< Version field of the file header */
    unsigned int create_date; /**< Creation date field of the file header */
    unsigned int modification_date; /**< Modification date field of the file header */
    unsigned int backup_date; /**< Backup date field of the file header */
    unsigned int modification_number; /**< Modification number field of the file header */
    unsigned int appinfo_offset; /**< Appinfo offset field of the file header */
    unsigned int sortinfo_offset; /**< Sortinfo offset field of the file header */
    unsigned char db_type[5]; /**< Database type field of the file header */
    unsigned char create_id[5]; /**< Creator ID field of the file header */
    unsigned int unique_seed; /**< Unique seed field of the file header */
    unsigned int next_record_list_id; /**< Next record list field of the file header */
    unsigned short number_records; /**< Number of records in the file */
    struct RecordInfo *record_list; /**< Array of RecordInfo structs for the file's records */
};

#endif
