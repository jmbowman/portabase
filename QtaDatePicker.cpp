//
//   Source file for the Qt Date Picker add-on
//
#include <qpe/resource.h>
#include <qdialog.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtableview.h>
#include <qpainter.h>
#include <qheader.h>
#include <qpixmap.h>
#include <qdatetime.h>
#include <qmessagebox.h>
#include <qcolor.h>
#include <qbrush.h>
#include <stdio.h>
#include "QtaDatePicker.h"
//
// Globals
//
char szMonth[12][10];
unsigned char ubMonthArray[7][6];
char szYear[5];
QDate *date;

//===================================================================
//
// Date picker main object dialog
//
//===================================================================
QDatePicker::QDatePicker( QDate *inDate ): QDialog( 0, 0, TRUE )
{
	// Make sure we got a valid date
	date = inDate;
	if ( !date->isValid() )
		{
		this->setResult( -1 );
		close();
		}

	// Disable the size gripper
	this->setSizeGripEnabled( FALSE );

	// Set our size
	this->resize( 160, 195 );

	// Set the title
	this->setCaption( tr("Select a date") );

	// Create the main calendar table
	calendarTable = new DatePickerTable( this );
	calendarTable->setGeometry( 3, 46, 154, 120 );
//	calendarTable->setFrameStyle( StyledPanel|Sunken );

	// Set the year label
	sprintf( szYear, "%d", date->year() );
	yearLabel = new QLabel( this );
	yearLabel->setText( tr(szYear) );
	yearLabel->setGeometry( 102, 2, 50, 20 );
	yearLabel->setAlignment( AlignCenter );

	// Set the month label
	monthLabel = new QLabel( this );
	strcpy( szMonth[0], tr("January") );
	strcpy( szMonth[1], tr("February") );
	strcpy( szMonth[2], tr("March") );
	strcpy( szMonth[3], tr("April") );
	strcpy( szMonth[4], tr("May") );
	strcpy( szMonth[5], tr("June") );
	strcpy( szMonth[6], tr("July") );
	strcpy( szMonth[7], tr("August") );
	strcpy( szMonth[8], tr("September") );
	strcpy( szMonth[9], tr("October") );
	strcpy( szMonth[10], tr("November") );
	strcpy( szMonth[11], tr("December") );
	monthLabel->setText( szMonth[date->month()-1] );
	monthLabel->setGeometry( 21, 2, 50, 20 );
	monthLabel->setAlignment( AlignCenter );

	// Create the 'prev' & 'next' month buttons
	prevMonthButton = new QPushButton( this );
	QPixmap prevPixmap = Resource::loadPixmap("portabase/QtaDatePickerPrev");
	prevMonthButton->setGeometry( 0, 1, 22, 22 );
	prevMonthButton->setPixmap( prevPixmap );
	connect( prevMonthButton, SIGNAL(clicked()), this, 
		SLOT(datePickerPrevMonth()) );

	nextMonthButton = new QPushButton( this );
	QPixmap nextPixmap = Resource::loadPixmap("portabase/QtaDatePickerNext");
	nextMonthButton->setGeometry( 70, 1, 22, 22 );
	nextMonthButton->setPixmap( nextPixmap );
	connect( nextMonthButton, SIGNAL(clicked()), this, 
		SLOT(datePickerNextMonth()) );

	// Create the 'prev' & 'next' year buttons
	prevYearButton = new QPushButton( this );
	prevYearButton->setGeometry( 90, 1, 22, 22 );
	prevYearButton->setPixmap( prevPixmap );
	connect(prevYearButton, SIGNAL(clicked()), this, 
		SLOT(datePickerPrevYear()));

	nextYearButton = new QPushButton( this );
	nextYearButton->setGeometry( 140, 1, 22, 22 );
	nextYearButton->setPixmap( nextPixmap );
	connect(nextYearButton, SIGNAL(clicked()), this, 
		SLOT(datePickerNextYear()));

	// Add the day headers
	tableHeader = new QHeader( this );
	tableHeader->setGeometry( 3, 25, 154, 20 );
	tableHeader->setOrientation( Horizontal );
	tableHeader->addLabel( tr("S"), 22 );
	tableHeader->addLabel( tr("M"), 22 );
	tableHeader->addLabel( tr("T"), 22 );
	tableHeader->addLabel( tr("W"), 22 );
	tableHeader->addLabel( tr("T"), 22 );
	tableHeader->addLabel( tr("F"), 22 );
	tableHeader->addLabel( tr("S"), 22 );

	// Create the 'OK' button
	okButton = new QPushButton( tr("OK"), this );
	okButton->setGeometry( 25, 170, 50, 25 );
	connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));

	// Create the 'Cancel' button
	cancelButton = new QPushButton( tr("Cancel"), this );
	cancelButton->setGeometry( 85, 170, 50, 25 );
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

	// Populate the month layout array
	this->datePickerMonthArray( date );
}
//
// Destructor
//
QDatePicker::~QDatePicker()
{
}
//
// Update the arry that holds the month info
//
void QDatePicker::datePickerMonthArray( QDate *date )
{
	int nDayCount = date->daysInMonth();
	QDate *tempDate = new QDate;
	tempDate->currentDate();
	tempDate->setYMD( date->year(), date->month(), 1 );
	int nFirstDay = tempDate->dayOfWeek();
	delete tempDate;
	if ( nFirstDay == 7 ) // Qt uses Mon=1, Tue=2, etc. - Change Sunday to 0
		nFirstDay = 0;
	int x = 0;
	int y = 0;
	int nDay = 1;
	// Populate the leading empty cells (if month doesn't start on Sunday)
	while ( x < nFirstDay )
		{
		ubMonthArray[x++][y] = 0;
		};
	// Fill in the rest of the days
	do
		{
		// If this is the current day, update the cur row/col
		if ( nDay == date->day() )
			calendarTable->updateCurRowCol( y, x );
		if ( nDay <= nDayCount )
			ubMonthArray[x][y] = nDay++;
		else
			ubMonthArray[x][y] = 0;
		x++;
		if ( x > 6 )
			{
			x = 0;
			y++;
			}
		} while ( y < 6 );
}
//
// 'OK' button handler
//
void QDatePicker::datePickerOk()
{
	accept();
	close();
}
//
// 'Cancel' button handler
//
void QDatePicker::datePickerCancel()
{
	reject();
	close();
}
void QDatePicker::datePickerNextMonth( )
{
//
// Note: Qt numbers months starting at 1 for Jan, not 0!
// Increment the month
// Check if the month is > 12
// If it is, change it to 1 and increment the year
//
	int nTempMonth = (date->month()) + 1;
	int nTempYear = date->year();
	if ( nTempMonth > 12 )
		{
		nTempMonth = 1;
		nTempYear++;
		sprintf( szYear, "%d", nTempYear );
		yearLabel->setText( tr(szYear) );
		}
	date->setYMD( nTempYear, nTempMonth, 1 );
	monthLabel->setText( szMonth[date->month()-1] );
//
//  Re-populate the month array
//  Re-draw the table
//
	datePickerMonthArray( date );
	calendarTable->updateTable();
}
void QDatePicker::datePickerPrevMonth( )
{
//
// Note: Qt numbers months starting at 1 for Jan, not 0!
// Decrement the month
// Check if the month is == 0
// If it is, change it to 12 and decrement the year
//
	int nTempMonth = (date->month()) - 1;
	int nTempYear = date->year();
	if ( nTempMonth == 0 )
		{
		nTempMonth = 12;
		nTempYear--;
		sprintf( szYear, "%d", nTempYear );
		yearLabel->setText( tr(szYear) );
		}
	date->setYMD( nTempYear, nTempMonth, 1 );
	monthLabel->setText( szMonth[date->month()-1] );
//
//  Re-populate the month array
//  Re-draw the table
//
	datePickerMonthArray( date );
	calendarTable->updateTable();
}
void QDatePicker::datePickerNextYear()
{
//
// Increment the year
// Change the month to 1 (Jan)
// Update the labels
// Set the new date
//
	int nTempYear = date->year()+1;
	sprintf( szYear, "%d", nTempYear );
	yearLabel->setText( tr(szYear) );	
	monthLabel->setText( szMonth[0] );
	date->setYMD( nTempYear, 1, 1 );
//
//  Re-populate the month array
//  Re-draw the table
//
	datePickerMonthArray( date );
	calendarTable->updateTable();
}
void QDatePicker::datePickerPrevYear()
{
//
// Decrement the year
// Change the month to 12 (Dec)
// Update the labels
// Set the new date
//
	int nTempYear = date->year()-1;
	sprintf( szYear, "%d", nTempYear );
	yearLabel->setText( tr(szYear) );	
	monthLabel->setText( szMonth[11] );
	date->setYMD( nTempYear, 12, 1 );
//
//  Re-populate the month array
//  Re-draw the table
//
	datePickerMonthArray( date );
	calendarTable->updateTable();
}
//===================================================================
//
// Calendar table object
//
//===================================================================

// Constructor for table
DatePickerTable::DatePickerTable( QWidget *parent ) 
	: QTableView( parent )
{
	// Set the table layout
	setNumCols( 7 );
	setNumRows( 6 );
	setCellWidth( 22 );
	setCellHeight( 20 );
	setBackgroundMode( PaletteBase );
//	setTableFlags( Tbl_autoVScrollBar|Tbl_autoHScrollBar );
}
//
// Cell painter handler
//
void DatePickerTable::paintCell( QPainter* p, int nRow, int nCol )
{
//
// Draw rectangles to outline the cells
//
	int x = ( cellWidth( nCol ) - 1 );
	int y = ( cellHeight( nRow) - 1 );
	p->drawRect( 0, 0, x, y );
//
// Add appropriate date to cell
//
	char szDay[3];
	if ( ubMonthArray[nCol][nRow] )
		sprintf( szDay, "%d", ubMonthArray[nCol][nRow] );
	else
		strcpy( szDay, " " );
//
// If this is the current day cell
//    Highlight it
//
	if ( nCol == nCurCol && nRow == nCurRow )
		{
		QBrush backgroundBrush( gray );
		p->setBrush( backgroundBrush );
		QPen Pen = p->pen();
		Pen.setWidth( 2 );
		QColor penColor( "black" );
		Pen.setColor( penColor );
		p->drawRect( 0, 0, x, y );
		p->drawText( 0, 0, (x+1), y, AlignCenter, szDay );
		p->setBrush( NoBrush );
		}
	else
		p->drawText( 0, 0, (x+1), y, AlignCenter, szDay );
}
//
// Mouse event handler
//
void DatePickerTable::mousePressEvent( QMouseEvent *event )
{
//
// Save the current old row & col
//
	int nOldRow = nCurRow;
	int nOldCol = nCurCol;
//
// Get the location of the click
// Xlate the position into a row & col
//
	QPoint clickedPos = event->pos();
	int nSelRow = findRow( clickedPos.y() );
	int nSelCol = findCol( clickedPos.x() );
//
// Make sure they selected a cell with a date!
//
	if ( ubMonthArray[nSelCol][nSelRow] )
		{
		nCurRow = nSelRow;
		nCurCol = nSelCol;
//
// If the current selection has changed
//    Update both (old & new)
//    Update the return day
//
		if ( nCurRow != nOldRow || nCurCol != nOldCol )
			{
			updateCell( nOldRow, nOldCol );
			updateCell( nCurRow, nCurCol );
			int nTempMonth = date->month();
			int nTempYear = date->year();
			date->setYMD( nTempYear, nTempMonth, 
				ubMonthArray[nSelCol][nSelRow] );
			}
		}
}
void DatePickerTable::updateTable(  )
{
//
//   Call 'updateCell' for all cells
//
	int nRow, nCol;
	for ( nRow=0; nRow<6; nRow++ )
		{
		for ( nCol=0; nCol<7; nCol++ )
			{
			updateCell( nRow, nCol );
			}
		}
}
//
// Update the current row/col
//
void DatePickerTable::updateCurRowCol( int nRow, int nCol )
{
	nCurRow = nRow;
	nCurCol = nCol;
}
