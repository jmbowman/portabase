/*
 * mobiledb.cpp
 *
 * (c) 2002-2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qfile.h>
#include <qstringlist.h>
#include "mobiledb.h"

#define FIELDLABELS 1
#define DATARECORD 2
#define DATARECORD_HIDDEN 3
#define PREFERENCES 4
#define DATATYPE 5
#define FIELDLENGTHS 6 

MobileDBFile::MobileDBFile(QString f) : PDBFile(f), valid_mdb(false), colcount(0), rowcount(0)
{

}

MobileDBFile::~MobileDBFile()
{

}

bool MobileDBFile::read() {
	return (valid_mdb = (PDBFile::read() ? readMobileDBHeader() : false));
}

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
	fd->at(record_list[record_id].record_data_offset);
	unsigned char *raw_record = new unsigned char [recordSize];
	if (fd->readBlock((char*)raw_record, recordSize) == -1) {
		return false;
	}
	if (!verify_recordhdr(raw_record)) {
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
	fd->at(record_list[record_id].record_data_offset);
	raw_record = new unsigned char [recordSize];
	if (fd->readBlock((char*)raw_record, recordSize) == -1) {
		return false;
	}
	if (!verify_recordhdr(raw_record)) {
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
	fd->at(record_list[record_id].record_data_offset);
	raw_record = new unsigned char [recordSize];
	if (fd->readBlock((char*)raw_record, recordSize) == -1) {
		return false;
	}
	if (!verify_recordhdr(raw_record)) {
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

int MobileDBFile::row_count()
{
	return rowcount;
}

int MobileDBFile::record_category(int cat_num)
{
	for (int i = 0; i < number_records; i++) {
		if ((record_list[i].record_attributes &0x7) == cat_num ) {
			return i;
		}
	}
	return -1;
}

bool MobileDBFile::verify_recordhdr(const unsigned char *raw_record) { 
	if (raw_record[0] != 0xff || raw_record[1] != 0xff ||
		raw_record[2] != 0xff || raw_record[3] != 0x01 
		|| raw_record[4] != 0xff || raw_record[5] != 0x00
		|| raw_record[6] != 0x00 ) {
		return false;
	}
	return true;
}

int MobileDBFile::col_count()
{
	return colcount;
}

void MobileDBFile::dumpMobileDBHeader()
{ 
	if (!valid_mdb) {
		printf("Invalid MobileDB Header\n");
		return;
	}
	printf("Version %d\n", version);	
	printf("# Row %d\n", rowcount);	
	printf("# Column %d\n", colcount);	
	printf("Column Labels - ");
	int i;
	for (i = 0; i < colcount; i++) {
		printf("%s | ", fieldlabels[i].latin1());
	}
	printf("\n");
	printf("Column Types - ");
	for (i = 0; i < colcount; i++) {
		printf("%s | ", fieldtypes[i].latin1());
	}
	printf("Column Size - ");
	for (i = 0; i < colcount; i++) {
		printf("%d | ", fieldlengths[i]);
	}
	printf("\n");
}

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
	fd->at(record_list[i].record_data_offset);
        unsigned int recordSize = record_list[i].record_size;
	unsigned char *raw_record = new unsigned char [recordSize];
	if (fd->readBlock((char*)raw_record, recordSize) == -1) {
		return rowdata;
	}
	//unpack row data;
	if (!verify_recordhdr(raw_record)) {
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
	

QStringList MobileDBFile::field_types()
{
	return fieldtypes;
}

const int * MobileDBFile::field_lengths() {
       	return fieldlengths;
}

QStringList MobileDBFile::field_labels()
{
	return fieldlabels;
}

