PortaBase 1.5 (February 2003)
----------------------------
PortaBase (portable database) is a personal database application originally
written for the Linux-based models of Sharp Zaurus PDA (and should work on
any other Linux PDA using the Qtopia environment.)  It can now also be
run as a Linux or Windows desktop application.

The main features PortaBase currently has are:
- One data table per file
- String, Integer, Decimal, Boolean, Note (multi-line text), Date, Time,
  and Enum column types
- Add, edit, and delete rows of data
- Custom data views (subsets of the columns in any order)
- Filter the displayed rows using sets of conditions
- Sort the rows by any combination of columns, each in ascending or descending
  order
- Page navigation buttons, with a custom number of rows per page
- Add, delete, rearrange, and rename columns at any time
- Specify default values for columns
- View summary statistics for columns (total, average, min, max, etc.)
- Import data from CSV, XML, and MobileDB files
- Export data to CSV and XML files
- Command-line format conversions (to and from XML, from MobileDB)
- Unicode support
- Pick any available font to use throughout the application

See the help file (help/html/portabase.html) for more information on features
and usage.  This help file is also the online help for the application,
available by clicking the "?" button at the top right of any screen in
PortaBase for the Zaurus, or Help->Help Contents in desktop PortaBase.

Zaurus Installation
-------------------
Note that there are *two* packages that must be installed in order to run
PortaBase; since many people didn't realize this for version 1.0, they are
now only distributed together in a zip file along with this README file.  First
install the libmetakit1 .ipk file onto the Zaurus, then install the
portabase .ipk file.  (The MetaKit library is packaged separately because
it is likely to be used by other applications in the near future.)  Once
both packages have been installed, PortaBase can be launched from the
Applications tab and the two .ipk files may be safely deleted.

Zaurus Upgrades
---------------
To upgrade from a previous version of PortaBase, do the following:

1) uninstall libmetakit1 and portabase (your data files will be left alone)
2) install the new libmetakit1 and portabase .ipk files
3) delete both .ipk files (you don't have to, but it frees up space)

IMPORTANT NOTE: libmetakit1 has changed for PortaBase 1.4; unlike previous
PortaBase upgrades, it must be upgraded in addition to the portabase package.

Windows Installation and Upgrades
---------------------------------
Run the downloaded executable; it will launch the installation program,
just follow the directions.

Technical Info and Acknowledgements
-----------------------------------
PortaBase is written in C++, using the Qt and Qtopia libraries for GUI widgets,
data structures, and communication with the PDA environment.  It uses the
MetaKit (http://www.equi4.com/metakit) embedded database library for data
storage and manipulation.  It also uses an enhanced version of the
QtaDatePicker widget developed by John McDonald, available from
http://prdownloads.sourceforge.net/zaurus/.  The MobileDB import code was
based on code from ZReader (http://www.codecubed.com/zreader/index.html).
The CSV import code was based on code from KSpread
(http://www.koffice.org/kspread/).

License
-------
PortaBase is is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

Web site
--------
http://portabase.sourceforge.net

------------------------------------------------------------------------------
-- Jeremy Bowman <jmbowman@alum.mit.edu>
