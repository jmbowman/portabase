PortaBase 2.1 (August 2012)
---------------------------
PortaBase (portable database) is a program for conveniently managing one-table
database files. It can be used as a shopping list, password manager, media
inventory, financial tracker, TODO list, address book, photo album, and more.
It is available for many platforms, including Linux, Mac OS X, Windows, and
Maemo (Diablo and Fremantle).

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
- Export data to CSV, HTML, and XML files
- Command-line format conversions (to and from XML, to and from CSV,
  from MobileDB, to HTML)
- Data file encryption
- Unicode support for text columns, and UI translations for 8 different
  languages
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
file format changes for PortaBase 2.0 or 2.1; files can still be moved freely
between the new release and PortaBase 1.9 on the Zaurus.  A future PortaBase
release is likely to end this compatibility, however.  For now, Maemo (as used
on the Nokia N810 and N900) is recommended for people who want to continue
using PortaBase on a pocketable device.  A rewrite that will also support
Android, iOS, and other popular mobile platforms is currently being prepared;
see the PortaBase home page for more details.

Maemo Installation
------------------
PortaBase 2.0 is available from the extras repository, and 2.1 is available
from extras-devel; you should be able to download and install it from there.
In time, version 2.1 should make its way to the extras-testing and extras
repositories as well, for users who are a little less adventurous.

Windows Installation and Upgrades
---------------------------------
Run the downloaded executable; it will launch the installation program,
just follow the directions.

Mac OS X Installation
---------------------
Double-click on the downloaded disk image to open it, and drag the application
from the disk image to where you want it (usually your Applications folder).

Debian Installation
-------------------
Install the .deb package as root using dpkg -i <filename>.

Installation on Other Linux/UNIX Systems
----------------------------------------
Download the source code tarball and follow the instructions in the contained
INSTALL file.

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

Additional software is used to package PortaBase on its various platforms:
- The Sphinx documentation generator (http://sphinx.pocoo.org) for creating
  easily-translated help pages from simple text files
- The Transifex online translation platform (https://www.transifex.com) for
  managing the UI and help file translations
- The GNU gettext utilities (http://www.gnu.org/software/gettext) for managing
  the translatable message files used by Sphinx
- Inno Setup (http://www.jrsoftware.org/isinfo.php) for creating the Windows
  installer

License
-------
PortaBase is is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

Web site
--------
http://www.portabase.org

------------------------------------------------------------------------------
-- Jeremy Bowman <jmbowman@alum.mit.edu>
