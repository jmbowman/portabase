/*
 * mobiledb.h
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef MOBILEDB_H
#define MOBILEDB_H
#include "pdbfile.h"

class MobileDBFile : public PDBFile { 
	public:
		MobileDBFile(QString f);
		virtual ~MobileDBFile();
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
		bool valid_mdb;
		unsigned short version;
		int colcount;
		int rowcount;
		QStringList fieldlabels;
		QStringList fieldtypes;
		int fieldlengths[20];
};	

#endif
