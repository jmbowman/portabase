/*
 * pdbfile.cpp
 *
 * (c) 2002-2003,2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file pdbfile.cpp
 * Source file for PDBFile
 */

#include <QFile>
#include <QMessageBox>
#include <time.h>
#include "pdbfile.h"

/**
 * Constructor.
 *
 * @param f The path of the file to be parsed
 */
PDBFile::PDBFile(const QString &f) : fd(0), filename(f), file_attributes(0),
  version(0), create_date(0), modification_date(0), backup_date(0),
  modification_number(0), appinfo_offset(0), sortinfo_offset(0),
  unique_seed(0), next_record_list_id(0), number_records(0), record_list(0)
{
	memset(pdb_name, 0, sizeof(pdb_name));
	memset(db_type, 0, sizeof(db_type));
	memset(create_id, 0, sizeof(create_id));
};

/**
 * Destructor.
 */
PDBFile::~PDBFile()
{
	if (record_list) {
		delete [] record_list;
        record_list = 0;
    }
	if (fd) {
		fd->close();
		delete fd;
        fd = 0;
	}
}

/**
 * Parse the file, loading data from the header and basic information
 * about each record in the file.
 *
 * @return True if the file was successfully parsed, false otherwise
 */
bool PDBFile::read()
{ 
	fd = new QFile(filename);
	if (!fd->open(QIODevice::ReadOnly)) {
		return false;
	}
	return readHeader() ? readRecordList() : false;
}

/**
 * Convert the provided raw file data to an unsigned short.
 *
 * @param v The data from the file to be converted
 * @return The unsigned short represented by the input data
 */
unsigned short PDBFile::toshort(unsigned char *v)
{
	return ((unsigned short)(v[0] << 8)) + v[1];
}

/**
 * Convert the provided raw file data to an unsigned int.
 *
 * @param v The data from the file to be converted
 * @return The unsigned int represented by the input data
 */
unsigned int PDBFile::toint(unsigned char *v) {
	return	((unsigned int)(v[0] << 24)) + 
		((unsigned int)(v[1] << 16)) + 
		((unsigned int)(v[2] << 8)) + 
		((unsigned int)(v[3] << 0));
}

/**
 * Convert the provided time-representing integer from the file to a UNIX
 * time in seconds since the epoch.  Conversion needed since Palm OS counts
 * seconds since 1904-01-01, but UNIX counts time since 1970-01-01.
 *
 * @param p The Palm OS time integer to be converted
 * @return The converted UNIX time integer
 */
time_t PDBFile::palm2unix_time(unsigned int p)
{
	//number of seconds between 01/01/1904 and 01/01/1970
	return p - 2082844886LU; 
}

/**
 * Parse the file's header data.
 *
 * @return True if successfully parsed, false otherwise
 */
bool PDBFile::readHeader()
{
	unsigned int offset = 0;
	unsigned char header [78];
	if (fd->read((char*)header, sizeof(header)) == -1) {
		//set error 
		return false;
	}

	memcpy(pdb_name, header + offset, sizeof(pdb_name) - 1);
	offset += sizeof(pdb_name) - 1;
	
	file_attributes = toshort(header + offset);	
	offset +=  sizeof(file_attributes);

	version = toshort(header + offset);	
	offset +=  sizeof(version);

	create_date = toint(header + offset);	
	offset +=  sizeof(create_date);

	modification_date = toint(header + offset);	
	offset +=  sizeof(modification_date);

	backup_date = toint(header + offset);	
	offset +=  sizeof(backup_date);

	appinfo_offset = toint(header + offset);	
	offset +=  sizeof(appinfo_offset);

	sortinfo_offset = toint(header + offset);	
	offset +=  sizeof(sortinfo_offset);

	modification_number = toint(header + offset);	
	offset +=  sizeof(modification_number);

	memcpy(db_type, header + offset, sizeof(db_type) - 1);
	offset += sizeof(db_type) - 1;

	memcpy(create_id, header + offset, sizeof(create_id) - 1);
	offset += sizeof(create_id) - 1;

	unique_seed = toint(header + offset);	
	offset +=  sizeof(unique_seed);

	next_record_list_id = toint(header + offset);	
	offset +=  sizeof(next_record_list_id);

	number_records = toshort(header + offset);	
	offset +=  sizeof(number_records);

	return true;
}

/*
			unsigned int record_data_offset;
			unsigned char record_attributes;
			unsigned int unique_id;
*/
/**
 * Parse the metadata about each record in the file (offset within the file,
 * attributes, unique ID, and record size).
 *
 * @return True if successfully parsed, false otherwise
 */
bool PDBFile::readRecordList()
{ 
	if (fd == NULL) {
		return false;
	}
	record_list = new PDBFile::RecordInfo[number_records];
	unsigned char record_buf[8];
    int i;
	for (i = 0 ; i < number_records; i++) {
		if (fd->read((char*)record_buf, sizeof(record_buf)) == -1) {
			return false;
		}
		record_list[i].record_data_offset = toint(record_buf);
		record_list[i].record_attributes = record_buf[4];
		record_buf[4] = 0;
		record_list[i].unique_id = toint(record_buf + 4);
	}
	//calculate record size
	for (i = 0 ; i < number_records - 1; i++) {
		record_list[i].record_size = record_list[i + 1].record_data_offset - record_list[i].record_data_offset;
	}
	record_list[number_records - 1].record_size = fd->size() - record_list[number_records - 1].record_data_offset;
	return true;
}
