//
//   Source file for the Qt Date Picker add-on
//
#include <qpe/config.h>
#include <qpe/resource.h>

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
#include "pbdialog.h"
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
QDatePicker::QDatePicker(QDate *inDate, QWidget *parent)
  : PBDialog(tr("Select a date"), parent, 0)
{
    // Make sure we got a valid date
    date = inDate;
    if (!date->isValid()) {
        this->setResult(-1);
        close();
    }

    // Disable the size gripper
    this->setSizeGripEnabled(FALSE);

    // Find out what day the week starts on
    Config qpeConfig("qpe");
    qpeConfig.setGroup("Time");
    startMonday = qpeConfig.readBoolEntry("MONDAY") ? 1 : 0;

    // Initialize the layout
    QHBox *hbox = new QHBox(this);
    vbox->addWidget(hbox);

    // Create the 'prev' month button
    QPushButton *prevMonthButton = new QPushButton(hbox);
    QPixmap prevPixmap = Resource::loadPixmap("portabase/QtaDatePickerPrev");
    prevMonthButton->setPixmap(prevPixmap);
    connect(prevMonthButton, SIGNAL(clicked()),
            this, SLOT(datePickerPrevMonth()));

    // Set the month label
    monthBox = new QComboBox(FALSE, hbox);
    hbox->setStretchFactor(monthBox, 1);
    monthBox->insertItem(tr("January"));
    monthBox->insertItem(tr("February"));
    monthBox->insertItem(tr("March"));
    monthBox->insertItem(tr("April"));
    monthBox->insertItem(tr("May"));
    monthBox->insertItem(tr("June"));
    monthBox->insertItem(tr("July"));
    monthBox->insertItem(tr("August"));
    monthBox->insertItem(tr("September"));
    monthBox->insertItem(tr("October"));
    monthBox->insertItem(tr("November"));
    monthBox->insertItem(tr("December"));
    monthBox->setCurrentItem(date->month() - 1);
    connect(monthBox, SIGNAL(activated(int)),
            this, SLOT(datePickerMonthChanged(int)));

    // Create the 'next' month button
    QPushButton *nextMonthButton = new QPushButton(hbox);
    QPixmap nextPixmap = Resource::loadPixmap("portabase/QtaDatePickerNext");
    nextMonthButton->setPixmap(nextPixmap);
    connect(nextMonthButton, SIGNAL(clicked()),
            this, SLOT(datePickerNextMonth()));

    // Set the year label
    yearBox = new QSpinBox(1753, 7000, 1, hbox);
    yearBox->setValue(date->year());
    connect(yearBox, SIGNAL(valueChanged(int)),
            this, SLOT(datePickerYearChanged(int)));

    // Add the day headers
    QHeader *tableHeader = new QHeader(this);
    vbox->addWidget(tableHeader, 0, Qt::AlignHCenter);
    tableHeader->setOrientation(Horizontal);
    int h = 2 * font().pointSize();
    int w = (int)(1.1 * h);
    if (!startMonday) {
        tableHeader->addLabel(tr("Su"), w);
    }
    tableHeader->addLabel(tr("M"), w);
    tableHeader->addLabel(tr("Tu"), w);
    tableHeader->addLabel(tr("W"), w);
    tableHeader->addLabel(tr("Th"), w);
    tableHeader->addLabel(tr("F"), w);
    tableHeader->addLabel(tr("Sa"), w);
    if (startMonday) {
        tableHeader->addLabel(tr("Su"), w);
    }

    // Create the main calendar table
    calendarTable = new DatePickerTable(w, h, this);
    calendarTable->setMinimumWidth(7 * w);
    calendarTable->setMinimumHeight(6 * h);
    vbox->addWidget(calendarTable, 1, Qt::AlignHCenter);

    hbox = new QHBox(this);
    vbox->addWidget(hbox);
    new QWidget(hbox);

    // Create the 'OK' button
    QPushButton *okButton = new QPushButton(PBDialog::tr("OK"), hbox);
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    new QWidget(hbox);

    // Create the 'Today' button
    QPushButton *todayButton = new QPushButton(tr("Today"), hbox);
    connect(todayButton, SIGNAL(clicked()), this, SLOT(gotoToday()));
    new QWidget(hbox);

    // Create the 'Cancel' button
    QPushButton *cancelButton = new QPushButton(PBDialog::tr("Cancel"), hbox);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    new QWidget(hbox);

    // Populate the month layout array
    this->datePickerMonthArray(date);

    finishLayout(FALSE, FALSE, FALSE);
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
void QDatePicker::datePickerMonthArray(QDate *date)
{
    int nDayCount = date->daysInMonth();
    int year = date->year();
    int month = date->month();
    QDate *tempDate = new QDate;
    tempDate->currentDate();
    tempDate->setYMD(year, month, 1);
    int nFirstDay = tempDate->dayOfWeek();
    delete tempDate;
    if (startMonday) {
        // Qt uses Mon=1, Tue=2, etc.
        nFirstDay--;
    }
    else if (nFirstDay == 7) {
        // Change Sunday to 0
        nFirstDay = 0;
    }
    int x = 0;
    int y = 0;
    int nDay = 1;
    calendarTable->updateTodayRowCol(-1, -1);
    QDate today = QDate::currentDate();
    int todayDay = -1;
    if ((today.year() == year) && (today.month() == month)) {
        todayDay = today.day();
    }
    // Populate the leading empty cells
    // (if month doesn't start on first day of week)
    while (x < nFirstDay) {
        ubMonthArray[x++][y] = 0;
    }
    // Fill in the rest of the days
    do {
        // If this is the current day, update the cur row/col
        if (nDay == date->day()) {
            calendarTable->updateCurRowCol(y, x);
        }
        if (nDay == todayDay) {
            calendarTable->updateTodayRowCol(y, x);
        }
        if (nDay <= nDayCount) {
            ubMonthArray[x][y] = nDay++;
        }
        else {
            ubMonthArray[x][y] = 0;
        }
        x++;
        if (x > 6) {
            x = 0;
            y++;
        }
    } while (y < 6);
}

void QDatePicker::datePickerNextMonth()
{
    // Note: Qt numbers months starting at 1 for Jan, not 0!
    // Increment the month
    // Check if the month is > 12
    // If it is, change it to 1 and increment the year
    int nTempMonth = (date->month()) + 1;
    int nTempYear = date->year();
    if (nTempMonth > 12) {
        nTempMonth = 1;
        nTempYear++;
        yearBox->setValue(nTempYear);
    }
    date->setYMD(nTempYear, nTempMonth, 1);
    monthBox->setCurrentItem(nTempMonth - 1);

    //  Re-populate the month array
    //  Re-draw the table
    datePickerMonthArray(date);
    calendarTable->updateTable();
}

void QDatePicker::datePickerPrevMonth()
{
    // Note: Qt numbers months starting at 1 for Jan, not 0!
    // Decrement the month
    // Check if the month is == 0
    // If it is, change it to 12 and decrement the year
    int nTempMonth = (date->month()) - 1;
    int nTempYear = date->year();
    if (nTempMonth == 0) {
        nTempMonth = 12;
        nTempYear--;
        yearBox->setValue(nTempYear);
    }
    date->setYMD(nTempYear, nTempMonth, 1);
    monthBox->setCurrentItem(nTempMonth - 1);

    //  Re-populate the month array
    //  Re-draw the table
    datePickerMonthArray(date);
    calendarTable->updateTable();
}

void QDatePicker::datePickerMonthChanged(int selection)
{
    int nTempMonth = selection + 1;
    int nTempYear = date->year();
    date->setYMD(nTempYear, nTempMonth, 1);

    //  Re-populate the month array
    //  Re-draw the table
    datePickerMonthArray(date);
    calendarTable->updateTable();
}

void QDatePicker::datePickerYearChanged(int year)
{
    // Change the month to 1 (Jan)
    // Set the new date
    monthBox->setCurrentItem(0);
    date->setYMD(year, 1, 1);

    //  Re-populate the month array
    //  Re-draw the table
    datePickerMonthArray(date);
    calendarTable->updateTable();
}

void QDatePicker::gotoToday()
{
    // Change the date selection to today's date
    QDate today = QDate::currentDate();
    yearBox->setValue(today.year());
    monthBox->setCurrentItem(today.month() - 1);
    date->setYMD(today.year(), today.month(), today.day());

    //  Re-populate the month array
    //  Re-draw the table
    datePickerMonthArray(date);
    calendarTable->updateTable();
}

//===================================================================
//
// Calendar table object
//
//===================================================================

// Constructor for table
DatePickerTable::DatePickerTable(int w, int h, QWidget *parent)
    : QTableView(parent)
{
    // Set the table layout
    setNumCols(7);
    setNumRows(6);
    setCellWidth(w);
    setCellHeight(h);
    setBackgroundMode(PaletteBase);
}
//
// Cell painter handler
//
void DatePickerTable::paintCell(QPainter* p, int nRow, int nCol)
{
//
// Draw rectangles to outline the cells
//
    int x = cellWidth(nCol) - 1;
    int y = cellHeight(nRow) - 1;
    p->drawRect(0, 0, x, y);
//
// Add appropriate date to cell
//
    char szDay[3];
    if (ubMonthArray[nCol][nRow]) {
        sprintf(szDay, "%d", ubMonthArray[nCol][nRow]);
    }
    else {
        strcpy(szDay, " ");
    }
//
// If this is the current day cell
//    Highlight it
//
    if (nCol == nCurCol && nRow == nCurRow) {
        QBrush backgroundBrush(gray);
        p->setBrush(backgroundBrush);
        QPen Pen = p->pen();
        Pen.setWidth(2);
        QColor penColor("black");
        Pen.setColor(penColor);
        p->drawRect(0, 0, x, y);
        p->drawText(0, 0, (x+1), y, AlignCenter, szDay);
        p->setBrush(NoBrush);
    }
    else if (nCol == todayCol && nRow == todayRow) {
        QBrush backgroundBrush(cyan);
        p->setBrush(backgroundBrush);
        QPen Pen = p->pen();
        Pen.setWidth(2);
        QColor penColor("black");
        Pen.setColor(penColor);
        p->drawRect(0, 0, x, y);
        p->drawText(0, 0, (x+1), y, AlignCenter, szDay);
        p->setBrush(NoBrush);
    }
    else {
        p->drawText(0, 0, (x+1), y, AlignCenter, szDay);
    }
}
//
// Mouse event handler
//
void DatePickerTable::mousePressEvent(QMouseEvent *event)
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
    int nSelRow = findRow(clickedPos.y());
    int nSelCol = findCol(clickedPos.x());
//
// Make sure they selected a cell with a date!
//
    if (ubMonthArray[nSelCol][nSelRow]) {
        nCurRow = nSelRow;
        nCurCol = nSelCol;
//
// If the current selection has changed
//    Update both (old & new)
//    Update the return day
//
        if (nCurRow != nOldRow || nCurCol != nOldCol) {
            updateCell(nOldRow, nOldCol);
            updateCell(nCurRow, nCurCol);
            int nTempMonth = date->month();
            int nTempYear = date->year();
            date->setYMD(nTempYear, nTempMonth,
                         ubMonthArray[nSelCol][nSelRow]);
        }
    }
}

void DatePickerTable::updateTable()
{
//
//   Call 'updateCell' for all cells
//
    int nRow, nCol;
    for (nRow = 0; nRow < 6; nRow++) {
        for (nCol = 0; nCol < 7; nCol++) {
            updateCell(nRow, nCol);
        }
    }
}
//
// Update the current row/col
//
void DatePickerTable::updateCurRowCol(int nRow, int nCol)
{
    nCurRow = nRow;
    nCurCol = nCol;
}

void DatePickerTable::updateTodayRowCol(int nRow, int nCol)
{
    todayRow = nRow;
    todayCol = nCol;
}
