//
//   Include file for the Qt Date Picker add-on
//
#ifndef QTADATEPICKER_H
#define QTADATEPICKER_H
#include <qdialog.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtableview.h>
#include <qpainter.h>
#include <qheader.h>
#include <qdatetime.h>
#include <qmessagebox.h>

class QComboBox;
class QSpinBox;

//
//   Table object for the Qt 'Date Picker' add-on
//
class DatePickerTable: public QTableView
{
    Q_OBJECT

public:
	DatePickerTable( QWidget *parent=0 );
	void updateTable();
	void updateCurRowCol( int, int );
private:
	// The mandatory 'paint cell'
	void paintCell( QPainter *, int, int );

	// A table cell selection handler
	void mousePressEvent( QMouseEvent * );

	// Row & column
	int nCurRow, nCurCol;

private slots:
};
//
//   Main dialog object for the Qt 'Date Picker' add-on
//
class QDatePicker: public QDialog
{
    Q_OBJECT

public:
	QDatePicker( QDate *date );
	~QDatePicker();

	// Update the table
	void datePickerUpdateTable( );

	// Table
	DatePickerTable *calendarTable;
private:
	// Month & year displays
	QComboBox *monthBox;
	QSpinBox *yearBox;

	// Table headers (days of the week)
	QHeader *tableHeader;

	// 'OK' & 'Cancel' buttons
	QButton *okButton;
	QButton *cancelButton;

	// 'Prev' & 'Next' month buttons
	QButton *nextMonthButton;
	QButton *prevMonthButton;

	// Message box
	QMessageBox *mbox;

private slots:
	// 'OK' & 'Cancel' button handlers
	void datePickerOk();
	void datePickerCancel();

	// Populate the month array
	void datePickerMonthArray( QDate *date );

	// 'Prev' & 'Next' month button handlers
	void datePickerNextMonth();
	void datePickerPrevMonth();

        // month combobox changed handler
        void datePickerMonthChanged(int selection);

	// year spinbox changed handler
	void datePickerYearChanged(int year);

};


#endif // QTADATEPICKER_H
