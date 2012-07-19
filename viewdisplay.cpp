/*
 * viewdisplay.cpp
 *
 * (c) 2002-2004,2008-2011 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file viewdisplay.cpp
 * Source file for ViewDisplay
 */

#include <QApplication>
#include <QButtonGroup>
#include <QHeaderView>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QSpinBox>
#include <QStackedWidget>
#include <QStringList>
#include <QTextDocument>
#include <QTimer>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include "columninfo.h"
#include "database.h"
#include "datamodel.h"
#include "datatypes.h"
#include "factory.h"
#include "noteeditor.h"
#include "pbmaemo5style.h"
#include "portabase.h"
#include "roweditor.h"
#include "rowviewer.h"
#include "view.h"
#include "viewdisplay.h"
#include "image/imageviewer.h"
#include "image/slideshowdialog.h"

/**
 * Constructor.
 *
 * @param pbase The application's main window
 * @param parent This widget's parent widget
 */
ViewDisplay::ViewDisplay(PortaBase *pbase, QWidget *parent) : QWidget(parent),
    portabase(pbase), propogateColWidths(false), pressedHeader(-1),
    booleanToggle(false), paged(true), singleClickShow(true)
{
    timer.start();
    QVBoxLayout *vbox = Factory::vBoxLayout(this, true);
    stack = new QStackedWidget(this);
    vbox->addWidget(stack, 1);
    noResults = new QLabel("<center>" + tr("No results") + "</center>", stack);
    stack->addWidget(noResults);

    table = new QTreeView(stack);
    table->setUniformRowHeights(true);
    table->setSortingEnabled(false);
    table->setAllColumnsShowFocus(true);
    table->setRootIsDecorated(false);
#if defined(Q_WS_HILDON)
    table->setIconSize(QSize(24, 24));
#endif
#if defined(Q_WS_MAEMO_5)
    table->setIconSize(QSize(32, 32));
    table->setStyle(new PBMaemo5Style());
#endif
    Factory::updateRowColors(table);
    model = new DataModel(this);
    connect(model, SIGNAL(viewLoaded(View *)),
            this, SLOT(matchNewView(View *)));
    connect(model, SIGNAL(paginationChanged(int, int)),
            this, SLOT(updateButtons(int, int)));
    connect(model, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
            this, SLOT(tableChanged()));
    connect(model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
            this, SLOT(tableChanged()));
    connect(model, SIGNAL(modelReset()),
            this, SLOT(tableChanged()));
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setModel(model);
    stack->addWidget(table);

    connect(table->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            this, SLOT(rowSelected()));
    connect(table, SIGNAL(pressed(const QModelIndex &)),
            this, SLOT(cellPressed(const QModelIndex &)));
    connect(table, SIGNAL(clicked(const QModelIndex &)),
            this, SLOT(cellReleased(const QModelIndex &)));

    QHeaderView *header = table->header();
    header->setClickable(true);
    header->setMovable(false);
    header->setResizeMode(QHeaderView::Interactive);
    connect(header, SIGNAL(sectionPressed(int)), this, SLOT(headerPressed(int)));
    connect(header, SIGNAL(sectionClicked(int)), this, SLOT(headerReleased(int)));
    connect(header, SIGNAL(sectionResized(int, int, int)),
            this, SLOT(columnResized(int, int, int)));
    stack->setCurrentWidget(noResults);

    buttonRow = new QWidget(this);
    QHBoxLayout *hbox = Factory::hBoxLayout(buttonRow, true);
    rowsPerPage = new QSpinBox(buttonRow);
    rowsPerPage->setRange(1, 9999);
    rowsPerPage->setValue(13);
    connect(rowsPerPage, SIGNAL(valueChanged(int)), this,
            SLOT(updateRowsPerPage(int)));
    hbox->addWidget(rowsPerPage);
    vbox->addWidget(buttonRow);

    prevButton = new QToolButton(buttonRow);
    prevButton->setArrowType(Qt::LeftArrow);
    prevButton->setSizePolicy(QSizePolicy::MinimumExpanding,
                              QSizePolicy::Fixed);
    connect(prevButton, SIGNAL(clicked()), this, SLOT(previousPages()));
    hbox->addWidget(prevButton, 1);

    buttonGroup = new QButtonGroup(buttonRow);
    buttonGroup->setExclusive(true);
    for (int i=0; i < PAGE_BUTTON_COUNT; i++) {
        pageButtons[i] = new QToolButton(buttonRow);
        pageButtons[i]->setCheckable(true);
        pageButtons[i]->setToolButtonStyle(Qt::ToolButtonTextOnly);
        pageButtons[i]->setSizePolicy(QSizePolicy::MinimumExpanding,
                                      QSizePolicy::Fixed);
        buttonGroup->addButton(pageButtons[i], i);
        hbox->addWidget(pageButtons[i], 1);
    }
    connect(buttonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(changePage(int)));

    nextButton = new QToolButton(buttonRow);
    nextButton->setArrowType(Qt::RightArrow);
    nextButton->setSizePolicy(QSizePolicy::MinimumExpanding,
                              QSizePolicy::Fixed);
    connect(nextButton, SIGNAL(clicked()), this, SLOT(nextPages()));
    hbox->addWidget(nextButton, 1);

    updateButtonSizes();
}

/**
 * Set whether or not the data records should be split between multiple pages.
 * If set to false, all records will be shown in a scrollable list and the
 * page navigation widget row will be hidden.
 *
 * @param flag True if using paged data navigation, false otherwise
 */
void ViewDisplay::usePages(bool flag)
{
    if (paged == flag) {
        return;
    }
    paged = flag;
    if (paged) {
        model->setPage(1);
        buttonRow->show();
    }
    else {
        buttonRow->hide();
        model->setPage(0);
    }
}

/**
 * Make sure all the widgets in the page navigation row have the same height.
 */
void ViewDisplay::updateButtonSizes()
{
    int buttonHeight = rowsPerPage->sizeHint().height();
    prevButton->setFixedHeight(buttonHeight);
    for (int i=0; i < PAGE_BUTTON_COUNT; i++) {
        pageButtons[i]->setFixedHeight(buttonHeight);
    }
    nextButton->setFixedHeight(buttonHeight);
}

/**
 * Set whether or not actions taken in the data view have made
 * not-yet-committed changes to the database (for example, a change in the
 * width of a column made by moving the edge of its header).  Just passes the
 * provided value along to the main window (a PortaBase instance).
 *
 * @param y True if uncommitted changes have been made, false otherwise.
 */
void ViewDisplay::setEdited(bool y)
{
    portabase->setEdited(y);
}

/**
 * Update the maximum number of records which can be shown on a single page
 * of data.  Called when the value in the spin box at the lower left corner
 * of the widget is changed.
 *
 * @param rpp The new maximum number of records per page
 */
void ViewDisplay::updateRowsPerPage(int rpp)
{
    if (rpp < 1) {
        rowsPerPage->setValue(1);
    }
    model->setRowsPerPage(rowsPerPage->value());
    setEdited(true);
}

/**
 * Move to the data page whose number is displayed on the page navigation
 * button with the specified ID.  Called when that button is clicked.
 *
 * @param id The ID of the page navigation button which was pressed
 */
void ViewDisplay::changePage(int id)
{
    QAbstractButton *button = buttonGroup->button(id);
    int newPage = button->text().toInt();
    if (newPage == model->page()) {
        return;
    }
    model->setPage(newPage);
}

/**
 * Update the page navigation buttons to display the next set of pages,
 * and display the data on the first page in that set.  Called when the
 * right arrow navigation button is pressed
 */
void ViewDisplay::nextPages()
{
    int newPage = pageButtons[0]->text().toInt() + PAGE_BUTTON_COUNT;
    model->setPage(newPage);
}

/**
 * Update the page navigation buttons to display the previous set of pages,
 * and display the data on the first page in that set.  Called when the
 * left arrow navigation button is pressed
 */
void ViewDisplay::previousPages()
{
    int newFirstPage = pageButtons[0]->text().toInt() - PAGE_BUTTON_COUNT;
    newFirstPage = qMax(newFirstPage, 1);
    if (model->page() >= newFirstPage + PAGE_BUTTON_COUNT) {
        model->setPage(newFirstPage + PAGE_BUTTON_COUNT - 1);
    }
}

/**
 * Respond to any significant changes in the display table, and see if it
 * still contains data and a selected row.
 */
void ViewDisplay::tableChanged()
{
    if (model->rowCount() > 0) {
        stack->setCurrentWidget(table);
    }
    else {
        stack->setCurrentWidget(noResults);
    }
    portabase->setRowSelected(table->currentIndex().isValid());
}

/**
 * Update the display table to match the current application settings and
 * recent data changes. Row colors, screen size category, date & time settings,
 * newly imported data, etc.
 */
void ViewDisplay::resetTable()
{
    model->refresh();
}

/**
 * Get the number of rows passing the current filter.
 *
 * @return The row count
 */
int ViewDisplay::rowCount()
{
    return model->view()->totalRowCount();
}

/**
 * Get the number of data columns in the current view.
 *
 * @return The column count
 */
int ViewDisplay::columnCount()
{
    return model->columnCount() - 1;
}

/**
 * Generate an HTML string representing the content matching the current
 * view, filter, and sorting settings.  Used for printing, so it's limited
 * to the subset of HTML supported by QTextDocument.
 *
 * @return The generated HTML string
 */
QString ViewDisplay::toPrintHTML()
{
    View *view = model->view();
    QString result = "<html><head></head><body><table width=\"100%\"><thead><tr>";
    QStringList colNames = view->getColNames();
    int colCount = colNames.count();
    int i, j;
    QString cellPattern("<th align=\"left\">%1</th>");
    for (i = 0; i < colCount; i++) {
        result += cellPattern.arg(Qt::escape(colNames[i]));
    }
    result += "</tr></thead><tbody>";
    int rowCount = view->totalRowCount();
    int *types = view->getColTypes();
    QStringList data;
    int type;
    QString align;
    QStringList rowStarts;
    rowStarts << QString("<tr bgcolor=\"%1\">").arg(Factory::evenRowColor.name());
    rowStarts << QString("<tr bgcolor=\"%1\">").arg(Factory::oddRowColor.name());
    QString rowEnd("</tr>");
    cellPattern = "<td align=\"%1\">%2</td>";
    QStringList imgTags;
    imgTags << "<img src=\"icon://unchecked.png\">";
    imgTags << "<img src=\"icon://checked.png\">";
    imgTags << "<img src=\"icon://image.png\" width=\"16\" height=\"16\">";
    QString newline("\n");
    QString br("<br>");
    QString left("left");
    QString right("right");
    QString value;
    for (i = 0; i < rowCount; i++) {
        result += rowStarts[i % 2];
        data = view->getRow(i);
        for (j = 0; j < colCount; j++) {
            type = types[j];
            value = data[j];
            align = left;
            if (type == INTEGER || type == FLOAT || type == CALC || type == SEQUENCE) {
                align = right;
            }
            if (type == BOOLEAN) {
                value = imgTags[value.toInt()];
            }
            else if (type == IMAGE && !value.isEmpty()) {
                value = imgTags[2];
            }
            else if (type == NOTE || type == STRING) {
                value = Qt::escape(value).replace(newline, br);
            }
            result += cellPattern.arg(align).arg(value);
        }
        result += rowEnd;
    }
    result += "</tbody></table></body>";
    return result;
}

/**
 * Update the page navigation buttons to reflect the current data and
 * navigation status; show the correct page numbers, disable buttons for
 * pages that don't exist, etc.
 *
 * @param currentPage The new current page number
 * @param totalPages The new total number of pages available
 */
void ViewDisplay::updateButtons(int currentPage, int totalPages)
{
    if (!paged) {
        return;
    }
    int newFirstPage = (((currentPage - 1)/PAGE_BUTTON_COUNT) * PAGE_BUTTON_COUNT) + 1;
    prevButton->setEnabled(newFirstPage > 1);
    nextButton->setEnabled(totalPages >= newFirstPage + PAGE_BUTTON_COUNT);
    int page = newFirstPage;
    for (int i = 0; i < PAGE_BUTTON_COUNT; i++) {
        pageButtons[i]->setChecked(page == currentPage);
        pageButtons[i]->setEnabled(totalPages >= page);
        pageButtons[i]->setText(QString::number(page));
        page++;
    }
}

/**
 * Set the database from which data is to be shown.
 *
 * @param dbase The database now in use.
 */
void ViewDisplay::setDatabase(Database *dbase)
{
    propogateColWidths = false;
    model->setDatabase(dbase);
    setEdited(false);
}

/**
 * Set the view to be used for displaying data.
 *
 * @param name The name of the view to use
 * @param applyDefaults True if the view's default filter and sorting are to
 *                      be used, false to retain the current settings
 */
void ViewDisplay::setView(const QString &name, bool applyDefaults)
{
    model->setView(name, applyDefaults);
}

/**
 * Update the column widths and rows per page spinbox to match a newly loaded
 * or edited view.
 *
 * @param view The new view to use
 */
void ViewDisplay::matchNewView(View *view)
{
    int count = view->columnCount();
    for (int i = 0; i < count; i++) {
        table->setColumnWidth(i, view->getColWidth(i));
    }
    table->setColumnWidth(count, 0);
    rowsPerPage->setValue(view->getRowsPerPage());
}

/**
 * Manually force a refresh of the column widths.  Used the first time the
 * data grid is shown for a file, since the column widths set earlier may not
 * have stuck as the widget wasn't shown yet.
 */
void ViewDisplay::updateColWidths()
{
    View *view = model->view();
    if (view) {
        bool edited = portabase->isWindowModified();
        matchNewView(view);
        if (!edited) {
            // these values were already in the file, so it isn't a change
            setEdited(false);
        }
        propogateColWidths = true;
    }
}

/**
 * Set the sorting to use and update the data display accordingly.
 *
 * @param name The name of the sorting to use
 */
void ViewDisplay::setSorting(const QString &name)
{
    model->setSorting(name);
}

/**
 * Set the filter to use and update the data display accordingly.
 *
 * @param name The name of the filter to use
 */
void ViewDisplay::setFilter(const QString &name)
{
    model->setFilter(name);
}

/**
 * Save any settings made to the current view, then remove this object's
 * reference to it.
 */
void ViewDisplay::closeView()
{
    model->closeView();
}

/**
 * Store in the database any changes made to column widths or the maximum
 * number of records per page.
 */
void ViewDisplay::saveViewSettings()
{
    model->saveViewSettings();
}

/**
 * Launch the RowEditor dialog to add a new record to the database, and
 * update the display if a record was actually added.
 */
void ViewDisplay::addRow()
{
    RowEditor rowEditor(this);
    int id = rowEditor.edit(model->database(), -1);
    if (id != -1) {
        model->addRow(id);
        setEdited(true);
    }
}

/**
 * Launch the RowEditor dialog to edit a record, and update the display if
 * the changes were accepted.
 *
 * @param id The ID of the record to edit; if -1, the record selected in the
 *           display table is used
 * @param copy True if the specified record is to be copied rather than edited
 * @param parent The parent widget of the new dialog (this widget if 0)
 * @return True if any changes were made, false otherwise
 */
bool ViewDisplay::editRow(int id, bool copy, QWidget *parent)
{
    int rowId = id;
    if (rowId == -1) {
        rowId = selectedRowId();
    }
    if (rowId != -1) {
        int rowIndex = model->view()->getIndex(rowId);
        RowEditor rowEditor(parent ? parent : this);
        rowId = rowEditor.edit(model->database(), rowId, copy);
        if (rowId != -1) {
            if (copy) {
                model->addRow(rowId);
            }
            else {
                model->editRow(rowId, rowIndex);
            }
            setEdited(true);
            return true;
        }
    }
    return false;
}

/**
 * View in the RowViewer dialog the record currently selected in the display
 * table (if any).
 */
void ViewDisplay::viewRow()
{
    int rowIndex = selectedRowIndex();
    if (rowIndex != -1) {
        RowViewer rowViewer(model->database(), this);
        rowViewer.viewRow(model->view(), rowIndex);
    }
}

/**
 * Delete the record currently selected in the display table (if any).
 */
void ViewDisplay::deleteRow()
{
    int rowId = selectedRowId();
    if (rowId != -1) {
        model->deleteRow(rowId);
        setEdited(true);
    }
}

/**
 * Delete all records matching the current filter.
 */
void ViewDisplay::deleteAllRows()
{
    model->deleteAllRows();
    setEdited(true);
}

/**
 * Export the records which match the currently applied filter to the
 * specified file.  All fields are exported (not just the ones in the current
 * view), in the order in which they appear in the database format definition.
 * The records are listed in the current sorting order.
 *
 * @param filename The CSV file to create or overwrite
 */
void ViewDisplay::exportToCSV(const QString &filename)
{
    model->view()->exportToCSV(filename);
}

/**
 * Export the information in the current view and filter to an HTML file at
 * the specified path.  The records are listed in the current sorting order.
 * Unlike the HTML generated for printing, this method's output is intended
 * to be a standalone page viewable in normal web browsers, with easily
 * overridden display properties.
 *
 * @param filename The HTML file to create or overwrite
 */
void ViewDisplay::exportToHTML(const QString &filename)
{
    model->view()->exportToHTML(filename);
}

/**
 * Export the full PortaBase file content to an XML file.  The records are
 * listed in the current sorting order, and the ones which do not match the
 * current filter are marked with an 'h="y"' attribute (an abbreviation for
 * 'hidden="yes"').
 *
 * @param filename The XML file to create or overwrite
 */
void ViewDisplay::exportToXML(const QString &filename)
{
    model->view()->exportToXML(filename);
}

/**
 * Get the position index in the underlying Metakit data view of the record
 * currently selected in the display table (if any).
 *
 * @return The selected record's position index within the view, -1 if none
 */
int ViewDisplay::selectedRowIndex()
{
    QModelIndex selected = table->currentIndex();
    if (!selected.isValid()) {
        return -1;
    }
    int index = selected.row();
    if (!paged) {
        return index;
    }
    int rpp = rowsPerPage->value();
    int startIndex = (qMax(model->page(), 1) - 1) * rpp;
    return startIndex + index;
}

/**
 * Get the ID of the record currently selected in the display table (if any).
 *
 * @return The selected record's ID, -1 if no selection has been made
 */
int ViewDisplay::selectedRowId()
{
    int index = selectedRowIndex();
    if (index == -1) {
        return -1;
    }
    return model->view()->getId(index);
}

/**
 * Lets the main window know that a record has been selected.  Called
 * automatically when a record is selected.
 */
void ViewDisplay::rowSelected()
{
    portabase->setRowSelected(true);
}

/**
 * Handler for mouse presses on displayed fields.  Used to determine how
 * long the mouse button was held down before releasing.
 *
 * @param index The cell which was pressed
 */
void ViewDisplay::cellPressed(const QModelIndex &index)
{
    pressedIndex = index.column();
    timer.restart();
}

/**
 * Mouse click (pressed and released) handler for displayed fields.  Used to
 * display note content dialogs, toggle boolean values, display images, and
 * launch the RowEditor dialog for the selected row (depending on exactly
 * where and for how long the mouse was clicked).  If the "SingleClickShow"
 * preference is set to true, then a single click launches the row viewer for
 * the clicked row.
 *
 * @param index The cell which was clicked
 */
void ViewDisplay::cellReleased(const QModelIndex &index)
{
    int column = index.column();
    if (column != pressedIndex) {
        return;
    }
    int *types = model->view()->getColTypes();
    int type = types[column];
    if (type == BOOLEAN && booleanToggle) {
        model->toggleBoolean(index);
        setEdited(true);
    }
    else if (timer.elapsed() > 500) {
        if (type == NOTE) {
            QString colName = model->headerData(column, Qt::Horizontal).toString();
            NoteEditor viewer(colName, true, this);
            viewer.setContent(model->data(index, Qt::EditRole).toString());
            viewer.exec();
        }
        else if (type == IMAGE) {
            int rowId = selectedRowId();
            if (rowId == -1) {
                return;
            }
            View *view = model->view();
            QString format = view->getImageFormat(rowId, column);
            if (!format.isEmpty()) {
                QImage image = view->getImage(rowId, column);
                ImageViewer viewer(true, this);
                viewer.setView(view, selectedRowIndex(), column);
                viewer.setImage(image);
                viewer.exec();
            }
        }
        else {
            editRow();
        }
    }
    else if (singleClickShow) {
        viewRow();
    }
}

/**
 * Handler for mouse press events on the row of column headers.  Used to
 * determine how long the mouse button was held down before releasing.
 *
 * @param column The position index of the column whose header was pressed on
 */
void ViewDisplay::headerPressed(int column)
{
    pressedHeader = column;
    QTimer::singleShot(500, this, SLOT(showStatistics()));
}

/**
 * Mouse click (pressed and released) handler for the row of column headers.
 * Used to change the current sort column with a short click, or display a
 * column statistics dialog with a long click.
 *
 * @param column The position index of the column in which the click occurred
 */
void ViewDisplay::headerReleased(int column)
{
    if (column != pressedHeader) {
        pressedHeader = -1;
        return;
    }
    else {
        pressedHeader = -1;
        sort(column);
    }
}

/**
 * Handler for column resize events sent by the row of column headers.
 * Propogates the new width setting to the view and marks the database as
 * having been modified.
 *
 * @param column The position index of the column whose width was altered
 * @param oldWidth The previous width in pixels of the column in question
 * @param newWidth The new width in pixels of the column in question
 */
void ViewDisplay::columnResized(int column, int, int newWidth)
{
    // The last column always resizes to fill available space; ignore it
    if (propogateColWidths && column != model->columnCount() - 1) {
        model->view()->setColWidth(column, newWidth);
        setEdited(true);
    }
}

/**
 * Update the sorting in use to reflect a short click on a column header.
 * Sorts only on the clicked column, first in ascending order and then in
 * descending order if clicked again.  Updates the display table and marks
 * the database as having been modified.
 *
 * @param column The position index of the column being sorted on
 */
void ViewDisplay::sort(int column)
{
    model->toggleSort(column);
    portabase->updateSortMenu();
    setEdited(true);
}

/**
 * Show a dialog containing summary statistics for a particular column of
 * data.  Only values in records matching the current filter are factored
 * into the statistics.
 *
 * @param allColumns True if the dialog should allow selection of any column
 *                   in the current view, false for just the one clicked
 */
void ViewDisplay::showStatistics(bool allColumns)
{
    QString colName = QString::null;
    if (pressedHeader != -1) {
        colName = model->headerData(pressedHeader, Qt::Horizontal).toString();
        pressedHeader = -1;
    }
    else if (!allColumns) {
        return;
    }
    ColumnInfoDialog info(this);
    if (info.launch(model->view(), colName)) {
        matchNewView(model->view());
        setEdited(true);
    }
}

/**
 * Launch a dialog which will allow the user to pick a column of images to
 * view in a slideshow.  If there are now image columns in the current view
 * or records matching the current filter, a message dialog to that effect is
 * displayed instead.
 */
void ViewDisplay::slideshow()
{
    View *view = model->view();
    QStringList colNames = view->getColNames();
    int *types = view->getColTypes();
    int count = colNames.count();
    QStringList imageCols;
    for (int i = 0; i < count; i++) {
        if (types[i] == IMAGE) {
            imageCols.append(colNames[i]);
        }
    }
    if (imageCols.count() == 0) {
        QMessageBox::warning(this, qApp->applicationName(),
                             tr("No image columns in this view"));
        return;
    }
    if (view->totalRowCount() == 0) {
        QMessageBox::warning(this, qApp->applicationName(),
                             tr("No rows in this filter"));
        return;
    }
    SlideshowDialog dialog(imageCols, view, this);
    dialog.exec();
}

/**
 * Update display options to match the user's current preferences.
 *
 * @param settings The current set of application preferences
 */
void ViewDisplay::updatePreferences(QSettings *settings)
{
    settings->beginGroup("General");
    booleanToggle = settings->value("BooleanToggle", false).toBool();
    singleClickShow = settings->value("SingleClickShow", true).toBool();
    usePages(settings->value("PagedDisplay", false).toBool());
    settings->endGroup();
    Factory::updateRowColors(table);
#if defined(Q_WS_MAEMO_5)
    bool vhr = settings->value("Font/VariableHeightRows", false).toBool();
    (static_cast<PBMaemo5Style*>(table->style()))->setVariableHeightRows(vhr);
#endif
}

/**
 * Handler for keyboard key press events.  Ensures that pressing the
 * enter/return key while a row is selected will show that row in the
 * RowViewer dialog.
 *
 * @param e The keyboard event which occurred
 */
void ViewDisplay::keyPressEvent(QKeyEvent *e)
{
    int key = e->key();
    if (key == Qt::Key_Enter || key == Qt::Key_Return) {
        viewRow();
    }
    else {
        e->ignore();
    }
}

/**
 * Handler for keyboard key release events.  Ensures that releasing the space
 * bar while a row is selected will show that row in the RowViewer dialog.
 *
 * @param e The keyboard event which occurred
 */
void ViewDisplay::keyReleaseEvent(QKeyEvent *e)
{
    int key = e->key();
    if (key == Qt::Key_Space) {
        viewRow();
    }
    else {
        e->ignore();
    }
}
