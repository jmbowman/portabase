/*
 * roweditor.cpp
 *
 * (c) 2002-2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#if defined(DESKTOP)
#include <qhbox.h>
#include <qpushbutton.h>
#include "desktop/resource.h"
#endif

#if QT_VERSION >= 300
#include "desktop/dynamicedit.h"
#else
#include "dynamicedit.h"
#endif

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdialog.h>
#include <qgrid.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qscrollview.h>
#include "calc/calcwidget.h"
#include "database.h"
#include "datatypes.h"
#include "datewidget.h"
#include "notebutton.h"
#include "numberwidget.h"
#include "roweditor.h"
#include "timewidget.h"

RowEditor::RowEditor(QWidget *parent, const char *name, WFlags f)
  : QDialog(parent, name, TRUE, f), db(0), colTypes(0)
{
    setCaption(tr("Row Editor") + " - " + tr("PortaBase"));
}

RowEditor::~RowEditor()
{

}

bool RowEditor::edit(Database *subject, int rowId, bool copy)
{
    db = subject;
    addContent(rowId);
    bool finished = FALSE;
    bool aborted = FALSE;
    while (!finished) {
        if (!exec()) {
            finished = TRUE;
            aborted = TRUE;
        }
        else {
            finished = isValid();
        }
    }
    if (!aborted) {
        QStringList values = getRow();
        if (rowId == -1 || copy) {
            db->addRow(values);
        }
        else {
            db->updateRow(rowId, values);
        }
        return TRUE;
    }
    else {
        return FALSE;
    }
}

bool RowEditor::isValid()
{
    int count = colNames.count();
    int numberWidgetIndex = 0;
    int timeWidgetIndex = 0;
    for (int i = 0; i < count; i++) {
        int type = colTypes[i];
        if (type == INTEGER || type == FLOAT) {
            QString value = numberWidgets[numberWidgetIndex]->getValue();
            QString error = db->isValidValue(type, value);
            if (error != "") {
                QString message = colNames[i] + " " + error;
                QMessageBox::warning(this, tr("PortaBase"), message);
                return FALSE;
            }
            numberWidgetIndex++;
        }
        else if (type == TIME) {
            QString value = timeWidgets[timeWidgetIndex]->getTime();
            QString error = db->isValidValue(type, value);
            if (error != "") {
                QString message = colNames[i] + " " + error;
                QMessageBox::warning(this, tr("PortaBase"), message);
                return FALSE;
            }
            timeWidgetIndex++;
        }
    }
    return TRUE;
}

QStringList RowEditor::getRow(bool doCalcs)
{
    QStringList values;
    int checkBoxIndex = 0;
    int noteButtonIndex = 0;
    int dateWidgetIndex = 0;
    int timeWidgetIndex = 0;
    int calcWidgetIndex = 0;
    int numberWidgetIndex = 0;
    int comboBoxIndex = 0;
    int dynamicEditIndex = 0;
    int labelIndex = 0;
    int count = colNames.count();
    for (int i = 0; i < count; i++) {
        int type = colTypes[i];
        if (type == BOOLEAN) {
            int value = checkBoxes[checkBoxIndex]->isChecked() ? 1 : 0;
            values.append(QString::number(value));
            checkBoxIndex++;
        }
        else if (type == INTEGER || type == FLOAT) {
            values.append(numberWidgets[numberWidgetIndex]->getValue());
            numberWidgetIndex++;
        }
        else if (type == NOTE) {
            values.append(noteButtons[noteButtonIndex]->content());
            noteButtonIndex++;
        }
        else if (type == DATE) {
            int dateInt = dateWidgets[dateWidgetIndex]->getDate();
            values.append(QString::number(dateInt));
            dateWidgetIndex++;
        }
        else if (type == TIME) {
            values.append(timeWidgets[timeWidgetIndex]->getTime());
            timeWidgetIndex++;
        }
        else if (type == CALC) {
            if (doCalcs) {
                calcWidgets[calcWidgetIndex]->calculate();
            }
            values.append(calcWidgets[calcWidgetIndex]->getValue());
            calcWidgetIndex++;
        }
        else if (type == SEQUENCE) {
            values.append(sequenceLabels[labelIndex]->text());
            labelIndex++;
        }
        else if (type >= FIRST_ENUM) {
            values.append(comboBoxes[comboBoxIndex]->currentText());
            comboBoxIndex++;
        }
        else {
            values.append(dynamicEdits[dynamicEditIndex]->text());
            dynamicEditIndex++;
        }
    }
    return values;
}

void RowEditor::addContent(int rowId)
{
    QVBoxLayout *vbox = new QVBoxLayout(this);
#if defined(Q_WS_WIN)
    setSizeGripEnabled(TRUE);
    vbox->addWidget(new QLabel("<center><b>" + tr("Row Editor")
                               + "</b></center>", this));
#endif
    QScrollView *sv = new QScrollView(this);
    vbox->addWidget(sv);
    QWidget *grid = new QWidget(sv->viewport());
    sv->addChild(grid);
    sv->setResizePolicy(QScrollView::AutoOneFit);
    grid->resize(sv->visibleWidth(), sv->visibleHeight());
    colNames = db->listColumns();
    int count = colNames.count();
    QGridLayout *layout = new QGridLayout(grid, count + 1, 2);
    QStringList values;
    if (rowId != -1) {
        values = db->getRow(rowId);
    }
    else {
        for (int i = 0; i < count; i++) {
            values.append(db->getDefault(colNames[i]));
        }
    }
    if (colTypes) {
        delete[] colTypes;
    }
    colTypes = db->listTypes();
    for (int i = 0; i < count; i++) {
        QString name = colNames[i];
        int type = colTypes[i];
        layout->addWidget(new QLabel(name, grid), i, 0);
        if (type == BOOLEAN) {
            QCheckBox *box = new QCheckBox(grid);
            layout->addWidget(box, i, 1);
            if (values[i].toInt()) {
                box->setChecked(TRUE);
            }
            checkBoxes.append(box);
        }
        else if (type == INTEGER || type == FLOAT) {
            NumberWidget *widget = new NumberWidget(type, grid);
            layout->addWidget(widget, i, 1);
            widget->setValue(values[i]);
            numberWidgets.append(widget);
        }
        else if (type == NOTE) {
            NoteButton *button = new NoteButton(name, grid);
            layout->addWidget(button, i, 1);
            button->setContent(values[i]);
            noteButtons.append(button);
        }
        else if (type == DATE) {
            DateWidget *widget = new DateWidget(grid);
            layout->addWidget(widget, i, 1);
            widget->setDate(values[i].toInt());
            dateWidgets.append(widget);
        }
        else if (type == TIME) {
            TimeWidget *widget = new TimeWidget(grid);
            layout->addWidget(widget, i, 1);
            int defaultTime = values[i].toInt();
            if (defaultTime == 0) {
                defaultTime = -2;
            }
            widget->setTime(defaultTime);
            timeWidgets.append(widget);
        }
        else if (type == CALC) {
            CalcWidget *widget = new CalcWidget(db, name, colNames, this, grid);
            layout->addWidget(widget, i, 1);
            widget->setValue(values[i]);
            calcWidgets.append(widget);
        }
        else if (type == SEQUENCE) {
            QLabel *label = new QLabel(values[i], grid);
            layout->addWidget(label, i, 1);
            sequenceLabels.append(label);
        }
        else if (type >= FIRST_ENUM) {
            QComboBox *combo = new QComboBox(FALSE, grid);
            layout->addWidget(combo, i, 1);
            QStringList options = db->listEnumOptions(type);
            combo->insertStringList(options);
            int index = options.findIndex(values[i]);
            combo->setCurrentItem(index);
            comboBoxes.append(combo);
        }
        else {
            DynamicEdit *edit = new DynamicEdit(grid);
            layout->addWidget(edit, i, 1);
            edit->setText(values[i]);
            dynamicEdits.append(edit);
        }
    }
    layout->addWidget(new QWidget(grid), count, 0);
    layout->addWidget(new QWidget(grid), count, 1);
    layout->setRowStretch(count, 1);

#if defined(DESKTOP)
    QHBox *hbox = new QHBox(this);
    vbox->addWidget(hbox);
    new QWidget(hbox);
    QPushButton *okButton = new QPushButton(tr("OK"), hbox);
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    new QWidget(hbox);
    QPushButton *cancelButton = new QPushButton(tr("Cancel"), hbox);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    new QWidget(hbox);
    vbox->setResizeMode(QLayout::FreeResize);
    QWidget *parent = parentWidget();
    setMinimumWidth(parent->width() / 2);
    setMinimumHeight(parent->height());
    setIcon(Resource::loadPixmap("portabase"));
#else
    showMaximized();
#endif
}
