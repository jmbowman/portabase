//
//   Source file for the Qt Date Picker add-on
//
#if defined(DESKTOP)
#include "desktop/config.h"
#include "desktop/resource.h"
#else
#include <qpe/config.h>
#include <qpe/resource.h>
#endif

#include <qbrush.h>
#include <qcolor.h>
#include <qcombobox.h>
#include <qdatetime.h>
#include <qhbox.h>
#include <qheader.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <stdio.h>
#include "QtaDatePicker.h"
//
// Globals
//
unsigned char ubMonthArray[7][6];
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

	// Set the title
	this->setCaption( tr("Select a date") );

	// Find out what day the week starts on
	Config qpeConfig("qpe");
	qpeConfig.setGroup("Time");
	startMonday = qpeConfig.readBoolEntry("MONDAY") ? 1 : 0;

	// Initialize the layout
	QVBoxLayout *vbox = new QVBoxLayout(this);
	vbox->setResizeMode(QLayout::FreeResize);
	QHBox *hbox = new QHBox(this);
	vbox->addWidget(hbox);

	// Create the 'prev' month button
	prevMonthButton = new QPushButton(hbox);
	QPixmap prevPixmap = Resource::loadPixmap("portabase/QtaDatePickerPrev");
	prevMonthButton->setPixmap(prevPixmap);
	connect(prevMonthButton, SIGNAL(clicked()), this, 
		SLOT(datePickerPrevMonth()));

	// Set the month label
	monthBox = new QComboBox(FALSE, hbox);
	monthBox->insertItem( tr("January") );
	monthBox->insertItem( tr("February") );
	monthBox->insertItem( tr("March") );
	monthBox->insertItem( tr("April") );
	monthBox->insertItem( tr("May") );
	monthBox->insertItem( tr("June") );
	monthBox->insertItem( tr("July") );
	monthBox->insertItem( tr("August") );
	monthBox->insertItem( tr("September") );
	monthBox->insertItem( tr("October") );
	monthBox->insertItem( tr("November") );
	monthBox->insertItem( tr("December") );
	monthBox->setCurrentItem( date->month() - 1 );
	connect(monthBox, SIGNAL(activated(int)),
                this, SLOT(datePickerMonthChanged(int)));

	// Create the 'next' month button
	nextMonthButton = new QPushButton(hbox);
	QPixmap nextPixmap = Resource::loadPixmap("portabase/QtaDatePickerNext");
	nextMonthButton->setPixmap(nextPixmap);
	connect(nextMonthButton, SIGNAL(clicked()), this, 
		SLOT(datePickerNextMonth()));

	// Set the year label
	yearBox = new QSpinBox(1753, 7000, 1, hbox);
	yearBox->setValue(date->year());
	connect(yearBox, SIGNAL(valueChanged(int)),
                this, SLOT(datePickerYearChanged(int)));

	// Add the day headers
	tableHeader = new QHeader( this );
        vbox->addWidget(tableHeader, 0, Qt::AlignHCenter);
	tableHeader->setOrientation( Horizontal );
	if (!startMonday) {
		tableHeader->addLabel( tr("S"), 22 );
	}
	tableHeader->addLabel( tr("M"), 22 );
	tableHeader->addLabel( tr("T"), 22 );
	tableHeader->addLabel( tr("W"), 22 );
	tableHeader->addLabel( tr("T"), 22 );
	tableHeader->addLabel( tr("F"), 22 );
	tableHeader->addLabel( tr("S"), 22 );
	if (startMonday) {
		tableHeader->addLabel( tr("S"), 22 );
	}

	// Create the main calendar table
	calendarTable = new DatePickerTable( this );
        calendarTable->setMinimumWidth(154);
        calendarTable->setMinimumHeight(120);
        vbox->addWidget(calendarTable, 1, Qt::AlignHCenter);

        hbox = new QHBox(this);
        vbox->addWidget(hbox);
        new QWidget(hbox);

	// Create the 'OK' button
	okButton = new QPushButton(tr("OK"), hbox);
	connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
        new QWidget(hbox);

	// Create the 'Cancel' button
	cancelButton = new QPushButton(tr("Cancel"), hbox);
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
        new QWidget(hbox);

	// Populate the month layout array
	this->datePickerMonthArray( date );
#ifdef DEKSTOP
	setIcon(Resource::loadPixmap("portabase"));
#endif
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
	if ( startMonday ) {
		// Qt uses Mon=1, Tue=2, etc.
		nFirstDay--;
	}
	else if ( nFirstDay == 7 ) {
		// Change Sunday to 0
		nFirstDay = 0;
	}
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
                yearBox->setValue( nTempYear );
		}
	date->setYMD( nTempYear, nTempMonth, 1 );
	monthBox->setCurrentItem( nTempMonth - 1 );
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
                yearBox->setValue( nTempYear );
		}
	date->setYMD( nTempYear, nTempMonth, 1 );
	monthBox->setCurrentItem( nTempMonth - 1 );
//
//  Re-populate the month array
//  Re-draw the table
//
	datePickerMonthArray( date );
	calendarTable->updateTable();
}
void QDatePicker::datePickerMonthChanged(int selection)
{
        int nTempMonth = selection + 1;
	int nTempYear = date->year();
	date->setYMD( nTempYear, nTempMonth, 1 );
//
//  Re-populate the month array
//  Re-draw the table
//
	datePickerMonthArray( date );
	calendarTable->updateTable();
}
void QDatePicker::datePickerYearChanged(int year)
{
// Change the month to 1 (Jan)
// Set the new date	
	monthBox->setCurrentItem( 0 );
	date->setYMD( year, 1, 1 );
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
