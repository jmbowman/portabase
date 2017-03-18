PortaBase
=========

.. image:: https://img.shields.io/github/release/jmbowman/portabase.svg
    :target: https://github.com/jmbowman/portabase/releases

.. image:: https://img.shields.io/gitter/room/portabase/portabase.svg
    :target: https://gitter.im/portabase/portabase

.. image:: https://img.shields.io/github/license/jmbowman/portabase.svg
    :target: https://github.com/jmbowman/portabase/blob/master/COPYING

.. image:: https://img.shields.io/sourceforge/dt/portabase.svg
    :target: https://sourceforge.net/projects/portabase/files/

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

Development Status (March 2017)
-------------------------------

Development has moved from SourceForge to GitHub, the code has been updated
to support Qt 5, and a port to Android is currently in beta testing.  Good
progress has been made on automating builds and releases, so I plan to
release a small update soon for all supported platforms in order to better
support recent OS releases.  If you would like to contribute, or have
suggestions on ways to make it easier to contribute, please contact me.

Note for Maemo Users
--------------------

After 6.5 years, my Nokia N900 suffered a hardware failure such that it no
longer works as a phone.  I suspect I was one of the last holdouts on that
platform, and the Maemo Diablo and Fremantle versions of PortaBase are the
only supported platforms that still require support for Qt 4.  If you still
regularly use PortaBase on Maemo devices, please let me know; I plan to
release at least one more version that supports them, but I may not keep
doing so if it becomes a problem when adding new features (especially if
it seems like nobody is still using PortaBase on those platforms).

PortaBase 2.1 is available from the extras repository.

Note for Zaurus Users
---------------------

The complete lack of new releases of Qt or Qtopia for the Zaurus has made it
impractical to continue developing for it; every single feature would need to
be implemented separately for two different codebases.  However, there are no
file format changes for PortaBase 2.0 or 2.1; files can still be moved freely
between the new release and PortaBase 1.9 on the Zaurus.  A future PortaBase
release is likely to end this compatibility, however.  For now, Maemo (as used
on the Nokia N810 and N900) or the upcoming Android port are recommended for
people who want to continue using PortaBase on a pocketable device.  Support
for iOS, SailfishOS, and other actively-maintained mobile platforms is also
feasible, please let me know if you would like to use PortaBase on one of
these and/or help make that port happen.

Windows Installation and Upgrades
---------------------------------

Run the downloaded executable; it will launch the installation program,
just follow the directions.

Mac OS X Installation
---------------------

Double-click on the downloaded disk image to open it, and drag the application
from the disk image to where you want it (usually your Applications folder).
The first time you launch it, you may need to do so via a Control-click or
right click; this should be resolved with the next release (the installer
needs to be updated to satisfy the code signing requirements of recent
Mac OS X releases).

Debian/Ubuntu Installation
-------------------

Install the ``portabase`` package from the main package repository.
Alternatively, download the .deb package for the desired release and install
it as root using ``dpkg -i <filename>``.

Installation on Other Linux/UNIX Systems
----------------------------------------

Download the source code tarball and follow the instructions in the contained
INSTALL file.

Technical Info and Acknowledgements
-----------------------------------

PortaBase is written in C++, using the Qt library for GUI widgets, data
structures, and communication with the operating system.  It also uses:

- The Metakit (http://www.equi4.com/metakit) embedded database library for data
  storage and manipulation
- The Randomkit library (http://js2007.free.fr/code/index.html#RandomKit) for
  cryptographic-strength random number generation
- The Color Picker Qt Solution
  (originally at http://qt.nokia.com/products/appdev/add-on-products/catalog/4/Widgets/qtcolorpicker ,
  but no longer seems to be available online)
- MobileDB import code based on code from ZReader
  (originally from http://www.killefiz.de/zaurus/showdetail.php?app=751, but
  no longer available online)
- CSV import code based on code from KSpread, now renamed to Calligra Sheets
  (https://www.calligra.org/sheets/)
- Blowfish block cipher code based on the BeeCrypt library
  (http://beecrypt.sourceforge.net/)
- A modified version of the calculator widget from KMyMoney
  (https://kmymoney.org/)
- A modified version of QtActionBar (https://github.com/mbnoimi/QtActionBar)
  for the Android release

Additional software is used to package PortaBase on its various platforms:

- The Sphinx documentation generator (http://www.sphinx-doc.org) for creating
  easily-translated help pages from simple text files
- The Transifex online translation platform (https://www.transifex.com) for
  managing the UI and help file translations
- The GNU gettext utilities (http://www.gnu.org/software/gettext) for managing
  the translatable message files used by Sphinx
- Inno Setup (http://www.jrsoftware.org/isinfo.php) for creating the Windows
  installer

The application icon was created by Holly Guenther
(http://kimchikawaii.deviantart.com).

License
-------

PortaBase is is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

Web site
--------

http://portabase.org
