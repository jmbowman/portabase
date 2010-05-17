PortaBase 2.0 beta 2 (May 2010)
-------------------------------
PortaBase (portable database) is a program for conveniently managing one-table
database files. It is available for many platforms, including Linux, Mac OS X,
Windows, and Maemo (Diablo and Fremantle).

The main features PortaBase currently has are:
- One data table per file
- String, Integer, Decimal, Boolean, Note (multi-line text), Date, Time,
  Calculation, Sequence, Image, and Enum column types
- Add, edit, and delete rows of data
- Custom data views (subsets of the columns in any order)
- Filter the displayed rows using sets of conditions
- Sort the rows by any combination of columns, each in ascending or descending
  order
- Optional page navigation buttons, with a custom number of rows per page
- Add, delete, rearrange, and rename columns at any time
- Specify default values for columns
- View summary statistics for columns (total, average, min, max, etc.)
- Import data from CSV, XML, and MobileDB files
- Export data to CSV and XML files
- Command-line format conversions (to and from XML, to and from CSV,
  from MobileDB)
- Data file encryption
- Unicode support
- Pick any available font to use throughout the application (except on
  Mac OS X)
- User-specified alternating row background colors (except on Fremantle)
- Simple calculator widget for entering numeric data

See the application's help dialog for more information on features and usage.

Note for Zaurus Users
---------------------
The complete lack of new releases of Qt or Qtopia for the Zaurus has made it
impractical to continue developing for it; every single feature would need to
be implemented separately for two different codebases.  However, there are no
file format changes for PortaBase 2.0; files can still be moved freely between
the new release and PortaBase 1.9 on the Zaurus.  A future PortaBase 2.1
release is likely to end this compatibility, however.  Maemo (as used on the
Nokia N810 and N900) is recommended for people who want to continue using
PortaBase on a pocketable device.  Ports to Windows Mobile, Symbian S60,
and/or webOS may be forthcoming as well.

Maemo Installation
------------------
PortaBase is available from the extras-devel repository, you should be
able to download and install it from there.  In time, it should make its way
to the extras-testing and extras repositories as well, for users who are a
little less adventurous.

Windows Installation and Upgrades
---------------------------------
Run the downloaded executable; it will launch the installation program,
just follow the directions.

Mac OS X Installation
---------------------
Double-click on the downloaded disk image to open it, and drag the application
from the disk image to where you want it (usually your Applications folder).

Technical Info and Acknowledgements
-----------------------------------
PortaBase is written in C++, using the Qt 4 library for GUI widgets, data
structures, and communication with the operating system.  It also uses:

- The Metakit (http://www.equi4.com/metakit) embedded database library for data
  storage and manipulation
- The Randomkit library (http://js2007.free.fr/code/index.html#RandomKit) for
  cryptographic-strength random number generation
- The Color Picker Qt Solution
  (http://qt.nokia.com/products/appdev/add-on-products/catalog/4/Widgets/qtcolorpicker)
- MobileDB import code based on code from ZReader
  (http://www.killefiz.de/zaurus/showdetail.php?app=751)
- CSV import code based on code from KSpread (http://www.koffice.org/kspread/)
- Blowfish block cipher code based on the BeeCrypt library
  (http://beecrypt.sourceforge.net/)
- A modified version of the calculator widget from KMyMoney2
  (http://kmymoney2.sourceforge.net/)

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
