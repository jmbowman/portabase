# PortaBase XML Format

Starting with version 1.5, PortaBase can export the entire content of a database
file to an XML file (including views, filters, sortings, etc.) It can also
import data from XML files in the same format. Furthermore, PortaBase files can
be exported to XML and created from XML via command-line options to the
portabase program, so it is possible to write scripts that can read and create
PortaBase files. This allows you to manipulate PortaBase data files in a broad
variety of ways, including:

- Conversion of exported XML files to HTML, using styles and formatting of your
  choice. Two examples using [XSLT](https://www.w3.org/Style/XSL/) are
  [portabase_view.xslt](assets/portabase_view.xslt) (which shows the rows and columns
  present in the current filter and view at time of export) and
  [portabase_format.xslt](assets/portabase_format.xslt) (which shows the full structure
  of the database file, useful for understanding the format and debugging
  problems).
- Manipulation of exported files using any tool or language capable of parsing
  XML. Possible examples include a Java program using
  [iText](https://www.lowagie.com/iText/) to generate a PDF file, or a Python
  script that adds entries from a contact database to the
  [KDE](https://www.kde.org/) addressbook.
- Edit the exported data (and filters, views,...) by hand in a text editor (or
  using a script, etc.) and create a new PortaBase file based on the results.
- Use a program to automatically generate PortaBase files, maybe based on
  information from a central database or a web site. For example, you could
  write a program scheduled to run each night which would query databases or web
  sites for updated information, create a PortaBase file containing the results,
  and ftp or email that file to someplace you can access from your Maemo
  handheld device.

## Format

The XML format used by PortaBase is defined in [portabase.rng](assets/portabase.rng).
This is a schema file using RelaxNG, a schema language similar to but better
than the W3C's XML Schema language; for more information, see the
[RelaxNG home page](https://relaxng.org). It's a fairly literal representation of the [MetaKit
views used internally to store the data](format.md), with a few exceptions:

- The leading underscores of all view and column names are omitted, i.e.
  `columns` instead of `_columns`.
- Information kept purely for performance benefits (like the floating point data
  columns for Decimal fields) or as implementation details (like data row ID
  numbers) is not included.
- The `<data>` section uses one-letter element and attribute names since these are
  repeated many, many times in any file of respectable size. This helps keep the
  XML files from getting ridiculously large.
- Columns and rows not shown in the current view and filter at the time of
  export are marked with `h="y"` attributes (short for `hidden="yes"`). This is
  purely for the convenience of external tools (like the above XSLT files) so
  that you don't need to recreate the filter and view logic just to show the
  current appearance. This also means that if you want the smallest XML file
  possible, you should switch to the "All Columns" view and the "All Rows"
  filter before exporting.
- `<d>` (date) and `<t>` (time) elements are exported with an "s" attribute
  containing the string representation of the date or time, using the
  preferences in place when the file was exported. Again, purely for the
  convenience of processing tools.
- Rows are added to the file in the current sorting order (if any) instead of
  the order in which they appear in the data file. This is more meaningful, and
  (like the "h" attribute) helps reproduce the current appearance without
  reimplementing the application logic.

## Notes

There's a pretty wide variety of XSLT processors out there that you can use to
apply the stylesheets mentioned above to PortaBase XML files. I've been using
xsltproc, the command-line front end to [libxslt](https://xmlsoft.org/XSLT/); it's
free, really fast (being written in C), seems to be pretty feature-complete, and
is available for Linux, Windows, etc.

## Changes

### PortaBase 1.9

- Added `<p>` element as a possible child of `<r>`.

### PortaBase 1.8

- Added optional `<calcs>` and `<calcnodes>` elements.

### PortaBase 1.6

- Added optional `<vsort>` and `<vfilter>` elements.

### PortaBase 1.5

- First PortaBase version to support XML export; thus `<gversion>` in a valid
  PortaBase XML file will never be less than 7.

## Request for Feedback

- If you can think of any improvements to this format that make it useful for
  other purposes without overly complicating it, please let me know.
- If you develop a tool (Python script, XSLT file, Java program, etc.) for
  manipulating this format that you think other people might find useful, please
  email me about it so I can add it to this page.
