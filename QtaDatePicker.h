//
//   Include file for the Qt Date Picker add-on
//
#ifndef QTADATEPICKER_H
#define QTADATEPICKER_H

#include <qtableview.h>
#include <qdialog.h>

class QComboBox;
class QDate;
class QPainter;
class QSpinBox;

//
//   Table object for the Qt 'Date Picker' add-on
//
class DatePickerTable: public QTableView
{
    Q_OBJECT

public:
    DatePickerTable(int w, int h, QWidget *parent=0);
    void updateTable();
    void updateCurRowCol(int, int);
    void updateTodayRowCol(int, int);

private:
    // The mandatory 'paint cell'
    void paintCell(QPainter *, int, int);

    // A table cell selection handler
    void mousePressEvent(QMouseEvent *);

    // Selection row & column
    int nCurRow, nCurCol;

    // Today row & column
    int todayRow, todayCol;
};
//
//   Main dialog object for the Qt 'Date Picker' add-on
//
class QDatePicker: public QDialog
{
    Q_OBJECT

public:
    QDatePicker(QDate *date);
    ~QDatePicker();

    // Update the table
    void datePickerUpdateTable();

private:
    // Month & year displays
    QComboBox *monthBox;
    QSpinBox *yearBox;

    // Table
    DatePickerTable *calendarTable;

    // Week starts on Monday?
    bool startMonday;

private slots:
    // Populate the month array
    void datePickerMonthArray(QDate *date);

    // 'Prev' & 'Next' month button handlers
    void datePickerNextMonth();
    void datePickerPrevMonth();

    // month combobox changed handler
    void datePickerMonthChanged(int selection);

    // year spinbox changed handler
    void datePickerYearChanged(int year);

    // 'Today' button handler
    void gotoToday();
};


#endif // QTADATEPICKER_H
