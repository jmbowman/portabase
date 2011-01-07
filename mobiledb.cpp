/*
 * mobiledb.cpp
 *
 * (c) 2002-2003,2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file mobiledb.cpp
 * Source file for MobileDBFile
 */

#include <QFile>
#include "mobiledb.h"

#define FIELDLABELS 1
#define DATARECORD 2
#define DATARECORD_HIDDEN 3
#define PREFERENCES 4
#define DATATYPE 5
#define FIELDLENGTHS 6 

/**
 * Constructor.
 *
 * @param f Path to the MobileDB file to be parsed
 */
MobileDBFile::MobileDBFile(const QString &f) : PDBFile(f), colcount(0),
    rowcount(0)
{
    for (int i = 0; i < 20; i++) {
        fieldlengths[i] = 0;
    }
}

/**
 * Open the PDB file and load basic metadata about the database from it.
 *
 * @return True if successfully parsed, false otherwise
 */
bool MobileDBFile::read() {
	return (PDBFile::read() ? readMobileDBHeader() : false);
}

/**
 * Parse the MobileDB database metadata from the PDB file.
 *
 * @return True if successfully parsed, false otherwise
 */
bool MobileDBFile::readMobileDBHeader() { 
	if (strcmp((const char *)db_type, "Mdb1") != 0) {
		return false;
	}
	if (strcmp((const char *)create_id, "Mdb1") != 0) {
		return false;
	}
	//dig out the version number
	version = 0;

	//count rows of data
	rowcount = 0;
	int i;
	for (i = 0; i < number_records; i++) {
		if ((record_list[i].record_attributes & 0x7) == DATARECORD ||  
		    (record_list[i].record_attributes & 0x7) == DATARECORD_HIDDEN) {
			rowcount++;
		}
	}
	//count number of columns and assign then to the fields_labels;
	int record_id = record_category(FIELDLABELS);
	if (record_id < 0) {
		return false;
	}
	unsigned int recordSize = record_list[record_id].record_size;
	fd->seek(record_list[record_id].record_data_offset);
	unsigned char *raw_record = new unsigned char [recordSize];
	if (fd->read((char*)raw_record, recordSize) == -1) {
		delete[] raw_record;
		return false;
	}
	if (!verify_recordhdr(raw_record)) {
		delete[] raw_record;
		return false;
	}
	unsigned char *p = raw_record + 7;
	unsigned char *end = raw_record + recordSize;
	colcount = 0;
	while (p < end && *p != 0xff ) { 
		colcount++;
		p++;
		QString label = QString::fromLatin1((char*)p);
		fieldlabels.append(label);
		p += label.length() + 1;
	};
	delete [] raw_record;
	//read field types
	//
	record_id = record_category(DATATYPE);
	if (record_id < 0) {
		return false;
	}
	recordSize = record_list[record_id].record_size;
	fd->seek(record_list[record_id].record_data_offset);
	raw_record = new unsigned char [recordSize];
	if (fd->read((char*)raw_record, recordSize) == -1) {
		delete[] raw_record;
		return false;
	}
	if (!verify_recordhdr(raw_record)) {
		delete[] raw_record;
		return false;
	}
	p = raw_record + 7;
	end = raw_record + recordSize;
	for (i = 0; i < colcount && p < end && *p != 0xff; i++) {
		p++;
		QString type = QString::fromLatin1((char*)p);
		fieldtypes.append(type);
		p += type.length() + 1;
	};
	delete [] raw_record;
	//read field size
	//
	record_id = record_category(FIELDLENGTHS);
	if (record_id < 0) {
		return false;
	}
	recordSize = record_list[record_id].record_size;
	fd->seek(record_list[record_id].record_data_offset);
	raw_record = new unsigned char [recordSize];
	if (fd->read((char*)raw_record, recordSize) == -1) {
		delete[] raw_record;
		return false;
	}
	if (!verify_recordhdr(raw_record)) {
		delete[] raw_record;
		return false;
	}
	p = raw_record + 7;
	end = raw_record + recordSize;
	for (i = 0; i < colcount && p < end && *p != 0xff; i++) {   
		int index  = *p;
		p++;
		QString length = QString::fromLatin1((char*)p);
		fieldlengths[index] = length.toInt();
		p += length.length() + 1;
	};
	delete [] raw_record;
	return true;	
}

/**
 * Get the number of rows in the database.
 *
 * @return The number of database rows
 */
int MobileDBFile::row_count() const
{
	return rowcount;
}

/**
 * Get the index of the PDB database record which contains the specified
 * type of data.
 *
 * @param cat_num The ID of the type of data to retrieve
 */
int MobileDBFile::record_category(int cat_num)
{
	for (int i = 0; i < number_records; i++) {
		if ((record_list[i].record_attributes &0x7) == cat_num ) {
			return i;
		}
	}
	return -1;
}

/**
 * Make sure that the header of the provided PDB record data is valid.
 *
 * @return True if the data is valid, false otherwise.
 */
bool MobileDBFile::verify_recordhdr(const unsigned char *raw_record) const {
	if (raw_record[0] != 0xff || raw_record[1] != 0xff ||
		raw_record[2] != 0xff || raw_record[3] != 0x01 
		|| raw_record[4] != 0xff || raw_record[5] != 0x00
		|| raw_record[6] != 0x00 ) {
		return false;
	}
	return true;
}

/**
 * Get the number of columns in the database.
 *
 * @return The number of database columns.
 */
int MobileDBFile::col_count() const
{
	return colcount;
}

/**
 * Get the specified row of the database.
 *
 * @param i The index of the row to retrieve
 * @return List of column values for the desired row
 */
QStringList MobileDBFile::row(int i)
{
	QStringList rowdata;
	if (i >= rowcount || i < 0) {
		return rowdata;
	}
	int offset = 0;
	for (int h = 0; h < number_records; h++) {
		if ((record_list[h].record_attributes &0x7) == DATARECORD || 
			(record_list[h].record_attributes &0x7) == DATARECORD_HIDDEN) {
			if (offset == i) {
				i = h;
				break;
			}
			else {
				offset++;
			}
		}
	}
	fd->seek(record_list[i].record_data_offset);
	unsigned int recordSize = record_list[i].record_size;
	unsigned char *raw_record = new unsigned char [recordSize];
	if (fd->read((char*)raw_record, recordSize) == -1) {
		delete[] raw_record;
		return rowdata;
	}
	//unpack row data;
	if (!verify_recordhdr(raw_record)) {
		delete[] raw_record;
		return rowdata;
	}
	unsigned char *p = raw_record + 7;
	unsigned char *end = raw_record + recordSize;
	for(int x = 0; x < colcount && p < end && *p != 0xff; x++) { 
		p++;
		QString cell = QString::fromLatin1((char*)p);
		rowdata.append(cell);
		p += cell.length() + 1;
	};
	delete [] raw_record;
	return rowdata;
}
	
/**
 * Get the Column types (as MobileDB understands them) of all columns in the
 * database.
 *
 * @return List of the database's column types
 */
QStringList MobileDBFile::field_types() const
{
	return fieldtypes;
}

/**
 * Get the display widths for the database columns, in pixels.
 *
 * @return List of column display widths
 */
const int *MobileDBFile::field_lengths() const {
	return fieldlengths;
}

/**
 * Get the names of each column in the database.
 *
 * @return List of column names
 */
QStringList MobileDBFile::field_labels() const
{
	return fieldlabels;
}
