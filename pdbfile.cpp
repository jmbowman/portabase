/*
 * pdbfile.cpp
 *
 * (c) 2002-2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qfile.h>
#include <qmessagebox.h>
#include <time.h>
#include "pdbfile.h"

PDBFile::PDBFile(QString _f) : fd(NULL), file_name(_f), file_attributes(0), version(0), create_date(0), modification_date(0), backup_date(0), modification_number(0), appinfo_offset(0), sortinfo_offset(0), unique_seed(0), next_record_list_id(0), number_records(0), record_list(NULL)
{
	memset(pdb_name, 0, sizeof(pdb_name));
	memset(db_type, 0, sizeof(db_type));
	memset(create_id, 0, sizeof(create_id));
};

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

QString PDBFile::pdbname()
{
	return QString::fromLatin1((char*)pdb_name);
}

bool PDBFile::read()
{ 
	fd = new QFile(file_name);
	if (!fd->open(IO_ReadOnly)) {
		return false;
	}
        file_size = fd->size();
	return readHeader() ? readRecordList() : false;
}

unsigned short PDBFile::toshort(unsigned char *v)
{
	return ((unsigned short)(v[0] << 8)) + v[1];
}

unsigned int PDBFile::toint(unsigned char *v) {
	return	((unsigned int)(v[0] << 24)) + 
		((unsigned int)(v[1] << 16)) + 
		((unsigned int)(v[2] << 8)) + 
		((unsigned int)(v[3] << 0));
}

time_t PDBFile::palm2unix_time(unsigned int p)
{
	//number of seconds between 01/01/1904 and 01/01/1970
	return p - 2082844886LU; 
}

bool PDBFile::readHeader()
{
	unsigned int offset = 0;
	unsigned char header [78];
	if (fd->readBlock((char*)header, sizeof(header)) == -1) {
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
bool PDBFile::readRecordList()
{ 
	if (fd == NULL) {
		return false;
	}
	record_list = new PDBFile::RecordInfo[number_records];
	unsigned char record_buf[8];
        int i;
	for (i = 0 ; i < number_records; i++) {
		if (fd->readBlock((char*)record_buf,
                                  sizeof(record_buf)) == -1) {
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
	record_list[number_records - 1].record_size = file_size - record_list[number_records - 1].record_data_offset;
	return true;
}

void PDBFile::dumpHeader() {
	printf("PDB File -> %s\n", pdb_name);
	printf("FILE Attributes-> %x\n", file_attributes);
	printf("Version -> %d\n", version);
	time_t t = palm2unix_time(create_date);
	printf("Creation Date -> %s", ctime(&t));
	t = palm2unix_time(modification_date);
	printf("Modification Date -> %s", ctime(&t));
	t = palm2unix_time(backup_date);
	printf("Backup Date -> %s", ctime(&t));
	printf("Modification Number -> %d\n", modification_number);
	printf("AppInfo Offset -> %o\n", appinfo_offset);
	printf("SortInfo Offset -> %o\n", sortinfo_offset);
	printf("DB Type -> %s\n", db_type);
	printf("Creator Id -> %s\n", create_id);
	printf("Unique Seed -> %d\n", unique_seed);
	printf("Next Record List Id -> %d\n", next_record_list_id);
	printf("Number of Records -> %d\n", number_records);
}

void PDBFile::dumpRecordList() {
	printf("Record List\nOffset\tAttributes\tUniqueId\n");
	for(int i = 0 ; i < number_records; i++ ) {
		printf("%d\t%d\t%x\t\%d\t%x\n", i,
			record_list[i].record_data_offset,
			record_list[i].record_attributes >> 4 ,
			record_list[i].record_attributes & 0x7,
			record_list[i].unique_id);
	}
}
