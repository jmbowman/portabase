/*
 * enumeditor.cpp
 *
 * (c) 2002-2004,2008-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file enumeditor.cpp
 * Source file for EnumEditor
 */

#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QStackedWidget>
#include <QTextStream>
#include "database.h"
#include "enumeditor.h"
#include "factory.h"
#include "importdialog.h"
#include "metakitfuncs.h"
#include "qqutil/qqmenuhelper.h"

/**
 * Constructor.
 *
 * @param parent This dialog's parent widget, if any
 */
EnumEditor::EnumEditor(QWidget *parent)
  : PBDialog(tr("Enum Editor"), parent), db(0), eeiName("_eeiname"),
  eeiIndex("_eeiindex"), eecIndex("_eecindex"), eecType("_eectype"),
  eecOldName("_eecoldname"), eecNewName("_eecnewname"), sorting(NotSorted)
{
    QHBoxLayout *hbox = Factory::hBoxLayout(vbox);
    hbox->addWidget(new QLabel(tr("Enum Name") + " ", this));
    nameBox = new QLineEdit(this);
    hbox->addWidget(nameBox);

    hbox = Factory::hBoxLayout(vbox);
    QPushButton *sortButton = new QPushButton(tr("Sort"), this);
    hbox->addWidget(sortButton);
    connect(sortButton, SIGNAL(clicked()), this, SLOT(sortOptions()));
    QPushButton *importButton = new QPushButton(tr("Import"), this);
    hbox->addWidget(importButton);
    connect(importButton, SIGNAL(clicked()), this, SLOT(importOptions()));
    QPushButton *exportButton = new QPushButton(tr("Export"), this);
    hbox->addWidget(exportButton);
    connect(exportButton, SIGNAL(clicked()), this, SLOT(exportOptions()));

    stack = new QStackedWidget(this);
    vbox->addWidget(stack, 1);
    QString text("<center>%1<br><br>%2</center>");
    text = text.arg(tr("No options defined for this enumeration type"));
    QString extraText = tr("Press the \"Add\" button to create one, or press\nthe \"Import\" button to load lines from a text file");
    text = text.arg(extraText.replace("\n", "<br>"));
    noOptions = new QLabel(text, stack);
    stack->addWidget(noOptions);

    listWidget = Factory::listWidget(stack);
    stack->addWidget(listWidget);
    stack->setCurrentWidget(noOptions);

    addEditButtons();
    connect(addButton, SIGNAL(clicked()), this, SLOT(addOption()));
    connect(editButton, SIGNAL(clicked()), this, SLOT(editOption()));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteOption()));
    connect(upButton, SIGNAL(clicked()), this, SLOT(moveUp()));
    connect(downButton, SIGNAL(clicked()), this, SLOT(moveDown()));

    finishLayout();
    nameBox->setFocus();
}

/**
 * Launch this dialog to edit the named enumeration in the specified database
 * (it may be a new enumeration with a blank name).
 *
 * @param subject The database being edited
 * @param enumName The initial name of the enumeration to create or edit
 * @return 1 if the dialog's changes are to be committed, 0 otherwise
 */
int EnumEditor::edit(Database *subject, const QString &enumName)
{
    db = subject;
    originalName = enumName;
    nameBox->setText(enumName);
    if (!enumName.isEmpty()) {
        int enumId = db->getEnumId(enumName);
        QStringList options = db->listEnumOptions(enumId);
        int optionCount = options.count();
        for (int i = 0; i < optionCount; i++) {
            info.Add(eeiName [options[i].toUtf8()] + eeiIndex [i]);
        }
        updateList();
    }
    int result = exec();
    while (result) {
        if (hasValidName()) {
            if (info.GetSize() > 0) {
                break;
            }
            else {
                QMessageBox::warning(this, qApp->applicationName(),
                                     tr("Must have at least one option"));
                result = exec();
            }
        }
        else {
            result = exec();
        }
    }
    return result;
}

/**
 * Get the latest specified name for the enumeration.
 *
 * @return The enumeration's name
 */
QString EnumEditor::getName()
{
    return nameBox->text();
}

/**
 * Sort the listed options; triggered by the "Sort" button.  Sorts first
 * in ascending order, then descending if the button is clicked again before
 * any other changes are made.
 */
void EnumEditor::sortOptions()
{
    c4_View sorted;
    c4_View::stringCompareFunc = compareUsingLocale;
    if (sorting == Ascending) {
        sorting = Descending;
        sorted = info.SortOnReverse(eeiName, eeiName);
    }
    else {
        sorting = Ascending;
        sorted = info.SortOn(eeiName);
    }
    c4_View::stringCompareFunc = strcmp;
    int count = sorted.GetSize();
    for (int i = 0; i < count; i++) {
        int index = info.Find(eeiName [eeiName (sorted[i])]
                              + eeiIndex [eeiIndex (sorted[i])]);
        eeiIndex (info[index]) = i;
    }
    updateList();
}

/**
 * Import a list of options from the lines of a text file.  Triggered by the
 * "Import" button.
 */
void EnumEditor::importOptions()
{
    ImportDialog dialog(ImportDialog::OptionList, db, this);
    if (!dialog.exec()) {
        return;
    }
    QStringList lines = dialog.getOptions();
    int count = lines.count();
    int i;
    int additions = 0;
    for (i = 0; i < count; i++) {
        QByteArray option = lines[i].toUtf8();
        if (info.Find(eeiName [option]) != -1) {
            continue;
        }
        changes.Add(eecIndex [changes.GetSize()] + eecType [ADD_OPTION]
                 + eecOldName [""] + eecNewName [option]);
        info.Add(eeiName [option] + eeiIndex [info.GetSize()]);
        additions++;
    }
    if (additions > 0) {
        updateList();
        sorting = NotSorted;
    }
}

/**
 * Export the current list of options to a text file.  Triggered by the
 * "Export" button.
 */
void EnumEditor::exportOptions()
{
    QSettings settings;
    QString lastDir = QQMenuHelper::getLastDir(&settings);
    QString filename = QFileDialog::getSaveFileName(this,
        QQMenuHelper::tr("Choose a filename to save under"), lastDir);
    if (filename.isEmpty()) {
        return;
    }
    QStringList options = listCurrentOptions();
    QFile file(filename);
    if (file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text)) {
        QTextStream stream(&file);
        foreach (QString option, options) {
            stream << option << endl;
        }
        file.close();
    }
    else {
        QMessageBox::warning(this, qApp->applicationName(),
                             QObject::tr("Unable to open file"));
    }
    settings.setValue("Files/LastDir", QFileInfo(filename).absolutePath());
}

/**
 * Add a new option to the end of the list.  Triggered by the "Add" button.
 */
void EnumEditor::addOption()
{
    bool ok = true;
    QString text;
    while (ok) {
        text = QInputDialog::getText(this, PBDialog::tr("Add"),
                                     tr("Option text"), QLineEdit::Normal,
                                     QString::null, &ok);
        if (ok) {
            if (isValidOption(text)) {
                break;
            }
        }
    }
    if (ok) {
        QByteArray utf8Text = text.toUtf8();
        changes.Add(eecIndex [changes.GetSize()] + eecType [ADD_OPTION]
                 + eecOldName [""] + eecNewName [utf8Text]);
        info.Add(eeiName [utf8Text] + eeiIndex [info.GetSize()]);
        updateList();
        sorting = NotSorted;
    }
}

/**
 * Edit the currently selected option.  Triggered by the "Edit" button.
 */
void EnumEditor::editOption()
{
    int selected = listWidget->currentRow();
    if (selected == -1) {
        return;
    }
    QString originalText = listWidget->currentItem()->text();
    bool ok = true;
    QString newText;
    while (ok) {
        newText = QInputDialog::getText(this, PBDialog::tr("Edit"),
                                        tr("Option text"), QLineEdit::Normal,
                                        originalText, &ok);
        if (ok) {
            if (newText == originalText) {
                // clicked ok but left unchanged
                ok = false;
            }
            else if (isValidOption(newText)) {
                break;
            }
        }
    }
    if (ok) {
        QByteArray utf8NewText = newText.toUtf8();
        int rowIndex = info.Find(eeiIndex [selected]);
        eeiName (info[rowIndex]) = utf8NewText;
        changes.Add(eecIndex [changes.GetSize()] + eecType [RENAME_OPTION]
                   + eecOldName [originalText.toUtf8()]
                   + eecNewName [utf8NewText]);
        updateList();
        listWidget->setCurrentRow(selected);
        sorting = NotSorted;
    }
}

/**
 * Delete the currently selected option.  Triggered by the "Delete" button.
 */
void EnumEditor::deleteOption()
{
    int selected = listWidget->currentRow();
    if (selected == -1) {
        return;
    }
    if (!originalName.isEmpty() && info.GetSize() == 1) {
        QMessageBox::warning(this, qApp->applicationName(),
                             tr("Must have at least one option"));
        return;
    }
    QString name = listWidget->currentItem()->text();
    bool ok = true;
    QString replace("");
    if (!originalName.isEmpty()) {
        QStringList options = listCurrentOptions();
        options.removeOne(name);
        replace = QInputDialog::getItem(this, PBDialog::tr("Delete"),
                                        tr("Replace where used with:"),
                                        options, 0, false, &ok);
    }
    if (ok) {
        int rowIndex = info.Find(eeiIndex [selected]);
        info.RemoveAt(rowIndex);
        changes.Add(eecIndex [changes.GetSize()] + eecType [DELETE_OPTION]
                    + eecOldName [name.toUtf8()] + eecNewName [replace.toUtf8()]);
        // update position indices
        int count = info.GetSize();
        for (int i = 0; i < count; i++) {
            int index = eeiIndex (info[i]);
            if (index > selected) {
                eeiIndex (info[i]) = index - 1;
            }
        }
        updateList();
    }
}

/**
 * Move the selected option up by one in the list.  Triggered by the "Up"
 * button.
 */
void EnumEditor::moveUp()
{
    int selected = listWidget->currentRow();
    if (selected < 1) {
        return;
    }
    int row1 = info.Find(eeiIndex [selected]);
    int row2 = info.Find(eeiIndex [selected - 1]);
    eeiIndex (info[row1]) = selected - 1;
    eeiIndex (info[row2]) = selected;
    updateList();
    listWidget->setCurrentRow(selected - 1);
    sorting = NotSorted;
}

/**
 * Move the selected option down by one in the list.  Triggered by the "Down"
 * button.
 */
void EnumEditor::moveDown()
{
    int selected = listWidget->currentRow();
    int optionCount = info.GetSize();
    if (selected == -1 || selected == optionCount - 1) {
        return;
    }
    int row1 = info.Find(eeiIndex [selected]);
    int row2 = info.Find(eeiIndex [selected + 1]);
    eeiIndex (info[row1]) = selected + 1;
    eeiIndex (info[row2]) = selected;
    updateList();
    listWidget->setCurrentRow(selected + 1);
    sorting = NotSorted;
}

/**
 * Update the displayed list of options.
 */
void EnumEditor::updateList()
{
    listWidget->clear();
    listWidget->addItems(listCurrentOptions());
    if (listWidget->count() == 0) {
        stack->setCurrentWidget(noOptions);
    }
    else {
        stack->setCurrentWidget(listWidget);
    }
}

/**
 * Determine if the currently entered enumeration name is valid or not.
 * Prevents name clashes with other enumerations and names that begin with an
 * underscore.
 *
 * @return True if valid, false otherwise.
 */
bool EnumEditor::hasValidName()
{
    return validateName(nameBox->text(), originalName, db->listEnums());
}

/**
 * Determine if the specified text is a valid option or not.  Prohibits
 * duplicate entries and values that begin with an underscore.
 */
bool EnumEditor::isValidOption(const QString &option)
{
    if (!option.isEmpty()) {
        if (option[0] == '_') {
            QMessageBox::warning(this, qApp->applicationName(),
                                 PBDialog::tr("Name must not start with '_'"));
            return false;
        }
    }
    // check for other options with same text
    QStringList options = listCurrentOptions();
    if (options.contains(option)) {
        QMessageBox::warning(this, qApp->applicationName(),
                             PBDialog::tr("Duplicate name"));
        return false;
    }
    return true;
}

/**
 * Apply the changes that were made in the dialog to the database.
 */
void EnumEditor::applyChanges()
{
    QString enumName = nameBox->text();
    if (originalName.isEmpty()) {
        db->addEnum(enumName, listCurrentOptions());
        return;
    }
    if (enumName != originalName) {
        db->renameEnum(originalName, enumName);
    }
    c4_View sorted = changes.SortOn(eecIndex);
    int count = changes.GetSize();
    for (int i = 0; i < count; i++) {
        int changeType = eecType (sorted[i]);
        QString oldName = QString::fromUtf8(eecOldName (sorted[i]));
        QString newName = QString::fromUtf8(eecNewName (sorted[i]));
        if (changeType == ADD_OPTION) {
            db->addEnumOption(enumName, newName);
        }
        else if (changeType == RENAME_OPTION) {
            db->renameEnumOption(enumName, oldName, newName);
        }
        else if (changeType == DELETE_OPTION) {
            db->deleteEnumOption(enumName, oldName, newName);
        }
    }
    db->setEnumOptionSequence(enumName, listCurrentOptions());
}

/**
 * List the currently specified options in order.
 *
 * @return The list of options
 */
QStringList EnumEditor::listCurrentOptions()
{
    QStringList options;
    c4_View sorted = info.SortOn(eeiIndex);
    int optionCount = sorted.GetSize();
    for (int i = 0; i < optionCount; i++) {
        options.append(QString::fromUtf8(eeiName (sorted[i])));
    }
    return options;
}
