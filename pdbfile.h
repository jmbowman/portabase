/*
 * pdbfile.h
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef PDBFILE_H
#define PDBFILE_H

#include <sys/types.h>

class QFile;

class PDBFile {
	public:
			PDBFile(QString);
			virtual ~PDBFile();
			virtual bool read();
			void dumpHeader();
			void dumpRecordList();
			virtual QString pdbname();
	protected:
		struct RecordInfo { 
			unsigned int record_data_offset;
			unsigned char record_attributes;
			unsigned int unique_id;
			unsigned int record_size;
		};
	protected:
		bool readHeader();
		bool readRecordList();
		unsigned short toshort(unsigned char*);		
		unsigned int toint(unsigned char*);		
		time_t palm2unix_time(unsigned int);		
	protected:
		QFile *fd;
		QString file_name;
                unsigned int file_size;
		unsigned char pdb_name[33];
		unsigned short file_attributes;
		unsigned short version;
		unsigned int create_date;
		unsigned int modification_date;
		unsigned int backup_date;
		unsigned int modification_number;
		unsigned int appinfo_offset;
		unsigned int sortinfo_offset;
		unsigned char db_type[5];
		unsigned char create_id[5];
		unsigned int unique_seed;
		unsigned int next_record_list_id;
		unsigned short number_records;
		struct RecordInfo *record_list;
};

#endif
