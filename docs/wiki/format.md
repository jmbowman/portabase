# PortaBase File Format

PortaBase files are [Metakit](http://www.equi4.com/metakit/) database files. As
such, they can be conveniently accessed and manipulated using the Metakit
library via programs written in C++, Python, or Tcl. They can also be used by
any program written to work with Metakit files in general, such as
[KitViewer](http://www.equi4.com/metakit/kitviewer.html). For low-level details
of the format such as endianness, arrangement of bits in the file, header
structure, etc. see the Metakit documentation and source code. What is described
here is the set of "tables" that PortaBase creates and uses inside of these
files, and a bit about how they interact to make certain PortaBase features
possible. With this information, you can write a script or program to create,
access, and/or update a PortaBase file; possible uses include:

- Use a Python web app (in Django, etc.) to display the contents of a PortaBase
  file on the web.
- Create a PortaBase file from some data source, such as a PalmOS database
  application. Unlike with CSV import, this would allow you to also transfer
  information like filters and views.
- Transfer data from PortaBase to another application's format; this is the
  inverse of the last case.
- Batch generate PortaBase files (for example, create a set of files from a
  central data repository, each one customized for a different person and
  automatically emailed to them).

The addition of support for command-line XML import and export in version 1.5
makes it even easier to programatically create and edit PortaBase files; see the
[XML format documentation](portabase_xml.md) for more information.

## Naming Conventions

Data in Metakit files is organized by columns, where columns are defined
independently of any "table". A Metakit "view" is similar to a table in the
relational database sense in that it contains rows and columns. In order to
prevent name clashes between columns used in different views, PortaBase uses the
following naming conventions:

- All views, columns, and text values named in the code begin with an
  underscore. (User-input values which could potentially clash with these are
  not allowed to begin with an underscore.)
- Column names start (after the underscore) with a 1-2 letter abbreviation of
  the name of the view in which they are used.

## Text Encoding

Strings are stored in Metakit files as null-terminated 1-byte character
sequences. These character sequences in PortaBase files represent text which has
been encoded in [UTF-8](http://www.ietf.org/rfc/rfc2279.txt) in order to support
[Unicode](http://www.unicode.org) characters beyond those used by Western
European languages.

## The *_global* View

Metakit format string:
`_global[_gversion:I,_gview:S,_gsort:S,_gfilter:S,_gcrypt:I]`

This view contains exactly one row which contains information about the database
file as a whole and the condition in which the user last left it. It uses the
following columns:

1. *_gversion (integer)* - The version of the PortaBase format used by this
   file. Because of the way Metakit lets you easily include or exclude columns
   from a view, this value can be accessed even if the column structure of the
   _global view has changed since the file was created. The [mapping between
   PortaBase versions and file format versions](#version-mapping) is later in
   this document.
2. *_gview (string)* - The name of the view currently being displayed in the
   data viewer.
3. *_gsort (string)* - The name of the sorting currently in use. This may be ""
   if none is being used.
4. *_gfilter (string)* - The name of the filter currently in use.
5. *_gcrypt (integer)* - 1 if the data file contents are encrypted, 0 otherwise.

## The *_columns* View

Metakit format string:
`_columns[_cindex:I,_cname:S,_ctype:I,_cdefault:S,_cid:I]`

This view defines the column structure of the *_data* view, described below,
which contains the rows of data entered by the user. Each row of this view
defines one column of the data table. It uses the following columns:

1. *_cindex (integer)* - The zero-based index at which the column is located in
   the row editor and the "All Columns" view.
2. *_cname (string)* - The name of the column
3. *_ctype (integer)* - An integer code representing the data type of the
   column. The [mapping between codes and data types](#data-types) is later in
   this document.
4. *_cdefault (string)* - Default value of the column, used when adding a new
   row or when the column is first added (if there are existing rows of data).
   For sequence columns, this is the next number that will be used.
5. *_cid (integer)* - ID number for this column, different from that of any
   other column defined in this view.

## The *_data* View

Metakit format string: Varies

This view is where the main database content is stored. The first column is
named "_id" and contains the unique ID number of each row. The rows are
consecutively numbered, starting at 0 and ending at one less than the number of
rows. (When a row is deleted, the rows are renumbered accordingly.) The rest of
the column structure of *_data* is determined by the data in the *_columns*
view. Each column has an identifier which is derived from the column's ID number
and data type. Decimal values and Calculation results are actually stored in two
consecutive columns; one for a floating point representation (used for sorting,
filtering, and statistics) and the other storing the string each value was
originally entered as (for display). Similarly, enum values are stored both in a
string column containing the option text and an integer column containing the
option index. Images are stored in a bytes column, followed by a string column
containing the name of the image format (either "JPEG" or "PNG").

For example, a *_columns* view that looks like this:

| \_cindex | \_cname | \_ctype | \_cdefault | \_cid |
| -------- | ------- | ------- | ---------- | ----- |
| 0        | Title   | 0       |            | 0     |
| 1        | Price   | 2       | 39.95      | 1     |
| 2        | Pages   | 1       | 400        | 2     |
| 3        | InStock | 3       | 1          | 4     |
| 4        | Cover   | 9       |            | 5     |

would yield this Metakit format string for *_data*:

`_data[_id:I,_S0:S,_F1:F,_S1:S,_I2:I,_I4:I,_B5:B,_S5:S]`

## The *_views* View

Metakit format string:
`_views[_vname:S,_vrpp:I,_vdeskrpp:I,_vsort:S,_vfilter:S]`

This view stores basic information about the PortaBase views (column subsets)
defined for this database. It uses the following columns:

1. *_vname (string)* - The name of the view
2. *_vrpp (integer)* - The maximum number of rows to show per page in the data
   viewer when opened on a cell phone or other small device
3. *_vdeskrpp (integer)* - The maximum number of rows to show per page in the
   data viewer when opened on a PC
4. *_vsort (string)* - The name of the sorting to apply when this view is
   selected ("_none" if the sorting should be left as is)
5. *_vfilter (string)* - The name of the filter to apply when this view is
   selected ("_none" if the filter should be left as is)

The rest of the information needed to define a PortaBase view is in the
*_viewcolumns* view. **Note:** there is always a PortaBase view named "_all",
which includes all the columns from *_columns* in the same order.

## The *_viewcolumns* View

Metakit format string:
`_viewcolumns[_vcview:S,_vcindex:I,_vcname:S,_vcwidth:I,_vcdeskwidth:I]`

This view contains information about each column included in a PortaBase view.
It uses the following columns:

1. *_vcview (string)* - The name of a PortaBase view (as defined in _vname from
   the *_views* view)
2. *_vcindex (integer)* - The zero-based index at which this column is located
   in the view
3. *_vcname (string)* - The name of this column (as defined in _cname from the
   *_columns* view)
4. *_vcwidth (integer)* - The display width of this column in pixels when viewed
   on a cell phone or other small device
5. *_vcdeskwidth (integer)* - The display width of this column in pixels when
   viewed on a PC

## The *_sorts* View

Metakit format string: `_sorts[_sname:S]`

This view gives basic information about the sortings defined for this database.
It currently has only one column:

1. *_sname (string)* - The name of the sorting

The rest of the information needed to define a sorting is in the *_sortcolumns*
view. **Note:** if the data in this file has ever been sorted by clicking on a
column label in the data viewer, there is a sorting named "_single" which has
one entry in *_viewcolumns* for the clicked column.

## The *_sortcolumns* View

Metakit format string: `_sortcolumns[_scsort:S,_scindex:I,_scname:S,_scdesc:I]`

This view contains information about each column included in a sorting. It uses
the following columns:

1. *_scsort (string)* - The name of a sorting (as defined in _sname from the
   *_sorts* view)
2. *_scindex (integer)* - The zero-based index at which this column is located
   in the sorting (0 is sorted first, ties are sorted by 1, etc.)
3. *_scname (string)* - The name of this column (as defined in _cname from the
   *_columns* view)
4. *_scdesc (integer)* - 0 if this column is sorted in ascending order, 1 for
   descending order

## The *_filters* View

Metakit format string: `_filters[_fname:S]`

This view gives basic information about the filters defined for this database.
It currently has only one column:

1. *_fname (string)* - The name of the filter

The rest of the information needed to define a filter is in the
*_filterconditions* view. **Note:** There is always a filter named "_allrows",
which has no entries in *_filterconditions*. There is also sometimes a filter
named "_simple", which defines the most recently performed "Quick" filter.

## The *_filterconditions* View

Metakit format string:
`_filterconditions[_fcfilter:S,_fcposition:I,_fccolumn:S,_fcoperator:I,_fcconstant:S,_fccase:I]`

This view contains information about each condition included in a filter. It
uses the following columns:

1. *_fcfilter (string)* - The name of a filter (as defined in _fname from the
   *_filters* view)
2. *_fcposition (integer)* - The zero-based index at which this condition is
   located in the filter (0 is applied first, then 1, etc.)
3. *_fccolumn (string)* - The name of the column to which this condition applies
   (as defined in _cname from the *_columns* view). May instead be "_anytext",
   indicating any text column.
4. *_fcoperator (integer)* - Integer constant representing a comparison
   operator. The [mapping between codes and operators](#condition-operators) is
   later in this document.
5. *_fcconstant (string)* - The constant against which the value of the named
   column is to be compared.
6. *_fccase (integer)* - 1 if this is a string comparison that is to be case
   sensitive, 0 otherwise. Enum comparisons are always case sensitive.

## The *_enums* View

Metakit format string: `_enums[_ename:S,_eid:I,_eindex:I]`

This view gives basic information about each enum column type which has been
defined in the file. It uses the following columns:

1. *_ename (string)* - The name of the enum.
2. *_eid (integer)* - The internal ID number of the enum; also used as the
   _ctype value of columns of this enum type. Must be 100 or greater.
3. *_eindex (integer)* - The zero-based index at which this enum is located
   among other enums in the column type list.

## The *_enumoptions* View

Metakit format string: `_enumoptions[_eoenum:I,_eoindex:I,_eotext:S]`

This view defines the options which are available for each enum column type
defined in the file. It uses the following columns:

1. *_eoenum (integer)* - The _eid value of the enum this option belongs to.
2. *_eoindex (integer)* - The zero-based index at which this option is located
   in the list of all the enum's options.
3. *_eotext (string)* - The text content of this option.

## The *_calcs* View

Metakit format string: `_calcs[_calcid:I,_calcdecimals:I]`

This view gives basic information about each calculation column which has been
defined in the file. It uses the following columns:

1. *_calcid (integer)* - The ID number of the column this calculation is used
   for, also used as the calculation ID.
2. *_calcdecimals (integer)* - The number of digits to show after the decimal
   place for calculation results.

## The *_calcnodes* View

Metakit format string:
`_calcnodes[_cnid:I,_cnnodeid:I,_cnparentid:I,_cntype:I,_cnvalue:S]`

This view describes the nodes which define calculation columns. It uses the
following columns:

1. *_cnid (integer)* - The ID number of the calculation this node belongs to.
2. *_cnnodeid (integer)* - The ID number of this node within its calculation.
   The lowest ID represents the root node; next is its first child node (if
   any), followed by that child's first child, etc.
3. *_cnparentid (integer)* - The ID number of this node's parent node; for the
   root node, 0 is used.
4. *_cntype (integer)* - The type of node this is; the [mapping between codes
   and node types](#calculation-node-types) is later in this document.
5. *_cnvalue (string)* - The data component of the node; depending on the node
   type this can be a constant value, the name of another column, or empty.

## The *_crypto* View

Metakit format string: `_crypto[_criv:B,_crhash:B,_crdata:B]`

This view is only present in files containing encrypted data. In such files, the
only views present are this and _global; the rest of the views are stored in an
encrypted block of data in this view. This view uses the following columns:

1. *_criv (bytes)* - The random initialization vector used to start CBC mode
   encryption (needed in order to decrypt the data correctly)
2. *_crhash (bytes)* - The SHA-1 hash of the original data after it was padded
   to a multiple of the Blowfish block size but before it was encrypted. Used to
   test if the data has been decrypted correctly.
3. *_crdata (bytes)* - The encrypted data.

## Version Mapping

The file format versions used by the PortaBase releases are as follows:

| PortaBase Version | \_gversion |
| ----------------- | ---------- |
| 1.0               | 1          |
| 1.1               | 2          |
| 1.2               | 3          |
| 1.3               | 4          |
| 1.4               | 5          |
| 1.4.1             | 6          |
| 1.5               | 7          |
| 1.6               | 8          |
| 1.7               | 9          |
| 1.8               | 10         |
| 1.9, 2.0          | 11         |

## Data Types

The column data types are represented as follows:

| Code             | Data Type   | Metakit column type        | Example values                                               |
| ---------------- | ----------- | -------------------------- | ------------------------------------------------------------ |
| 0                | String      | string (S)                 | "answer to", "life", "universe and everything"               |
| 1                | Integer     | integer (I)                | 6, 9, 42, -123                                               |
| 2                | Decimal     | float (F) and string (S)   | 123.45, -987.65                                              |
| 3                | Boolean     | integer (I)                | 0, 1                                                         |
| 4                | Note        | string (S)                 | This entire   cell is one   example value                    |
| 5                | Date        | integer (I)                | 20021113, 19990101                                           |
| 6                | Time        | integer (I)                | 0, 86399, 42000                                              |
| 7                | Calculation | float (F) and string (S)   | 123.45, -987.65                                              |
| 8                | Sequence    | integer (I)                | 1, 2, 3, 4, 5                                                |
| 9                | Image       | bytes (B)                  | ![old PortaBase icon](images/portabase_old.png), ![new PortaBase icon](images/portabase_new.png) |
| 100, 101, 102... | Enum        | string (S) and integer (I) | "Male", "Female", "Deposit", "Withdrawal"                    |

Note that a date value of 17520914 (the day Britain adopted the Gregorian
calendar, previously the earliest date supported by Qt's QDate class) is treated
as a null date and is displayed in PortaBase as blank. A time value is the
number of seconds past midnight, with a value of -1 representing a null time
displayed as blank.

## Condition Operators

| Code | Operation   |
| ---- | ----------- |
| 0    | =           |
| 1    | contains    |
| 2    | starts with |
| 3    | <           |
| 4    | >           |
| 5    | <=          |
| 6    | >=          |
| 7    | !=          |

## Calculation Node Types

| Code | Node type                                         |
| ---- | ------------------------------------------------- |
| 0    | Numeric constant                                  |
| 1    | Reference to a numeric column                     |
| 2    | Date constant (such as '2003-08-03')              |
| 3    | Reference to a date column                        |
| 4    | Time constant (such as '11:43:27 AM')             |
| 5    | Reference to a time column                        |
| 20   | Addition                                          |
| 21   | Subtraction                                       |
| 22   | Multiplication                                    |
| 23   | Division                                          |
| 24   | Difference between two dates, measured in days    |
| 25   | Maximum                                           |
| 26   | Minimum                                           |
| 27   | Average                                           |
| 28   | Absolute value                                    |
| 29   | Square root                                       |
| 30   | Base-10 logarithm                                 |
| 31   | Natural logarithm                                 |
| 32   | Difference between two times, measured in seconds |
| 33   | Difference between two times, measured in minutes |
| 34   | Difference between two times, measured in hours   |

## Changes

### Version 11

- The value of _ctype is now allowed to be 9 (image)
- Calculation node types 4, 5, 32, 33, and 34 are now allowed

### Version 10

- Added the _calcs view
- Added the _calcnodes view
- The value of _ctype is now allowed to be 7 (calculation) or 8 (sequence)

### Version 9

- Version number incremented just to indicate that the enum option indices in
  the data table should be correct, which sometimes wasn't true for version 8

### Version 8

- Added enum option index columns to _data
- Added _vsort and _vfilter to _views
- Added _gcrypt to _global
- Added the _crypto view (only present in encrypted files)

### Version 7

- Added _vdeskrpp to _views
- Added _vcdeskwidth to _viewcolumns

### Version 6

- Dropped _fcindex in favor of _fcposition, which has the correct data type and
  should avoid the upgrade problems that plagued version 5.

### Version 5

- Added missing ":I" after _fcindex in _filterconditions format string
- The value of _ctype is now allowed to be 6 (time)
-

### Version 4

- Changed the text encoding from Latin-1 to UTF-8
- Added the _enums view
- Added the _enumoptions view
- The value of _ctype is now allowed to be greater than 99 (enums)
- Added _cid to _columns
- Use generated column IDs for the _data view's column names instead of the
  user-entered column names (allows for better Unicode support)

### Version 3

- Added the _filters view
- Added the _filterconditions view
- Added _gfilter to _global
- The value of _ctype is now allowed to be 5 (date)
-

### Version 2

- Added the _sorts view
- Added the _sortcolumns view
- Added _gsort to _global
- The value of _ctype is now allowed to be 4 (note)
-

### Version 1

- Format used in the first public release of PortaBase
