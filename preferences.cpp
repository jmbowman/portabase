/*
 * preferences.cpp
 *
 * (c) 2002-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qapplication.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qfont.h>
#include <qfontdatabase.h>
#include <qgroupbox.h>
#include <qhbox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qspinbox.h>
#include <qtabwidget.h>
#include "colorbutton.h"
#include "menuactions.h"
#include "pbdialog.h"
#include "portabase.h"
#include "preferences.h"

#if defined(Q_WS_QWS)
#include <qpe/config.h>
#include <qpe/resource.h>
#else
#include "desktop/config.h"
#include "desktop/resource.h"
#endif

Preferences::Preferences(QWidget *parent, const char *name)
    : PBDialog(tr("Preferences"), parent, name)
{
    QTabWidget *tabs = new QTabWidget(this);
    vbox->addWidget(tabs);
    addOptionsTab(tabs);
    addAppearanceTab(tabs);
    addMenusTab(tabs);
    addButtonsTab(tabs);
    finishLayout(TRUE, TRUE, TRUE, 400, 0);
}

Preferences::~Preferences()
{

}

void Preferences::addOptionsTab(QTabWidget *tabs)
{
    QWidget *optionsTab = new QWidget(tabs);
    QVBoxLayout *layout = new QVBoxLayout(optionsTab);
    QGroupBox *generalGroup = new QGroupBox(1, Qt::Horizontal, tr("General"),
                                            optionsTab);
    layout->addWidget(generalGroup);
    Config conf("portabase");
    conf.setGroup("General");
    QHBox *hbox = new QHBox(generalGroup);
    confirmDeletions = new QCheckBox(tr("Confirm deletions"), hbox);
    confirmDeletions->setChecked(conf.readBoolEntry("ConfirmDeletions", TRUE));
    QWidget *filler = new QWidget(hbox);
    hbox->setStretchFactor(filler, 1);
    hbox = new QHBox(generalGroup);
    booleanToggle = new QCheckBox(tr("Allow checkbox edit in data viewer"),
                                  hbox);
    booleanToggle->setChecked(conf.readBoolEntry("BooleanToggle"));
    filler = new QWidget(hbox);
    hbox->setStretchFactor(filler, 1);
    hbox = new QHBox(generalGroup);
    showSeconds = new QCheckBox(tr("Show seconds for times"), hbox);
    showSeconds->setChecked(conf.readBoolEntry("ShowSeconds"));
    filler = new QWidget(hbox);
    hbox->setStretchFactor(filler, 1);
    hbox = new QHBox(generalGroup);
    pagedDisplay = new QCheckBox(tr("Use pages in data viewer"), hbox);
    pagedDisplay->setChecked(conf.readBoolEntry("PagedDisplay", TRUE));
    filler = new QWidget(hbox);
    hbox->setStretchFactor(filler, 1);
    hbox = new QHBox(generalGroup);
    noteWrap = new QCheckBox(tr("Wrap Notes"), hbox);
    noteWrap->setChecked(conf.readBoolEntry("NoteWrap", TRUE));
    wrapType = new QComboBox(hbox);
    wrapType->insertItem(tr("at whitespace"));
    wrapType->insertItem(tr("anywhere"));
    if (conf.readBoolEntry("WrapAnywhere")) {
        wrapType->setCurrentItem(1);
    }
    wrapType->setEnabled(noteWrap->isChecked());
    connect(noteWrap, SIGNAL(toggled(bool)), wrapType, SLOT(setEnabled(bool)));
    hbox = new QHBox(generalGroup);
    new QLabel(tr("Default rows per page"), hbox);
    rowsPerPage = new QSpinBox(hbox);
    rowsPerPage->setRange(1, 9999);
#if defined(Q_WS_QWS)
    int defaultRows = 13;
#else
    int defaultRows = 25;
#endif
    rowsPerPage->setValue(conf.readNumEntry("RowsPerPage", defaultRows));

#if !defined(Q_WS_QWS)
    QGroupBox *dateGroup = new QGroupBox(2, Qt::Horizontal,
                                         tr("Date and Time"), optionsTab);
    layout->addWidget(dateGroup);
    Config config("qpe");
    config.setGroup("Date");
    new QLabel(tr("Date format"), dateGroup);
    PBDateFormat df(QChar(config.readEntry("Separator", "/")[0]),
	    (PBDateFormat::Order)config.readNumEntry("ShortOrder",
                                                  PBDateFormat::DayMonthYear),
	    (PBDateFormat::Order)config.readNumEntry("LongOrder",
                                                  PBDateFormat::DayMonthYear));
    dateFormatCombo = new QComboBox(dateGroup);
    int currentdf = 0;
    date_formats[0] = PBDateFormat('/', PBDateFormat::MonthDayYear);
    date_formats[1] = PBDateFormat('.', PBDateFormat::DayMonthYear);
    date_formats[2] = PBDateFormat('-', PBDateFormat::YearMonthDay,
                                   PBDateFormat::DayMonthYear);
    date_formats[3] = PBDateFormat('/', PBDateFormat::DayMonthYear);
    for (int i = 0; i < 4; i++) {
        dateFormatCombo->insertItem(date_formats[i].toNumberString());
        if (df == date_formats[i]) {
            currentdf = i;
        }
    }
    dateFormatCombo->setCurrentItem(currentdf);

    config.setGroup("Time");
    new QLabel(tr("Time format"), dateGroup);
    ampmCombo = new QComboBox(dateGroup);
    ampmCombo->insertItem(tr("24 hour"), 0);
    ampmCombo->insertItem(tr("12 hour"), 1);
    int show12hr = config.readBoolEntry("AMPM", TRUE) ? 1 : 0;
    ampmCombo->setCurrentItem(show12hr);

    new QLabel(tr("Weeks start on"), dateGroup);
    weekStartCombo = new QComboBox(dateGroup);
    weekStartCombo->insertItem(tr("Sunday"), 0);
    weekStartCombo->insertItem(tr("Monday"), 1);
    int startMonday =  config.readBoolEntry("MONDAY") ? 1 : 0;
    weekStartCombo->setCurrentItem( startMonday );
#endif
    layout->addWidget(new QWidget(optionsTab), 1);
    tabs->addTab(optionsTab, tr("Options"));
}

void Preferences::addAppearanceTab(QTabWidget *tabs)
{
    QWidget *appearanceTab = new QWidget(tabs);
    QVBoxLayout *layout = new QVBoxLayout(appearanceTab);
#if defined(Q_WS_QWS)
    sizeFactor = 10;
#else
    sizeFactor = 1;
#endif
    QGroupBox *fontGroup = new QGroupBox(2, Qt::Horizontal, tr("Font"),
                                         appearanceTab);
    layout->addWidget(fontGroup);
    new QLabel(tr("Name"), fontGroup);
    fontName = new QComboBox(FALSE, fontGroup);
    fonts = fontdb.families();
    fontName->insertStringList(fonts);
    new QLabel(tr("Size"), fontGroup);
    fontSize = new QComboBox(FALSE, fontGroup);
    connect(fontName, SIGNAL(activated(int)), this, SLOT(updateSizes(int)));
    QFont currentFont = qApp->font();
    int fontCount = fonts.count();
    QString family = currentFont.family().lower();
    int index = -1;
    for (int i = 0; i < fontCount; i++) {
        if (family == fonts[i].lower()) {
            index = i;
        }
    }
    // Windows defaults to a font not in QFontDatabase ???
    if (index == -1) {
        fontName->insertItem(currentFont.family());
        index = fontName->count() - 1;
    }
    fontName->setCurrentItem(index);
    updateSizes(index);
    int size = currentFont.pointSize();
    index = sizes.findIndex(size * sizeFactor);
    if (index > -1) {
        fontSize->setCurrentItem(index);
    }
    connect(fontSize, SIGNAL(activated(int)), this, SLOT(updateSample(int)));
    new QLabel(tr("Sample"), fontGroup);
    sample = new QLabel(tr("Sample text"), fontGroup);

    QGroupBox *colorGroup = new QGroupBox(2, Qt::Horizontal, tr("Row Colors"),
                                          appearanceTab);
    layout->addWidget(colorGroup);
    evenButton = new ColorButton(*PortaBase::evenRowColor, colorGroup);
    oddButton = new ColorButton(*PortaBase::oddRowColor, colorGroup);
    layout->addWidget(new QWidget(appearanceTab), 1);
    tabs->addTab(appearanceTab, tr("Appearance"));
}

void Preferences::addMenusTab(QTabWidget *tabs)
{
    QWidget *menusTab = new QWidget(tabs);
    QVBoxLayout *layout = new QVBoxLayout(menusTab);

    menuTable = new QListView(menusTab);
    layout->addWidget(menuTable);
    menuTable->setAllColumnsShowFocus(TRUE);
    menuTable->setSorting(-1);
    menuTable->header()->setMovingEnabled(FALSE);
    menuTable->addColumn(tr("Top-level"));
    menuTable->addColumn(tr("Menu"));

    menuList.append("Row");
    menuLabelList.append(MenuActions::tr("Row"));
    menuList.append("View");
    menuLabelList.append(MenuActions::tr("View"));
    menuList.append("Sort");
    menuLabelList.append(MenuActions::tr("Sort"));
    menuList.append("Filter");
    menuLabelList.append(MenuActions::tr("Filter"));

    QStringList topLevel;
    QStringList underFile;
    menuConfiguration(topLevel, underFile);
    // need to use this instead of menuList to get the sequence correct
    QStringList allMenus = topLevel + underFile;

    int count = allMenus.count();
    QCheckListItem *item = 0;
    QCheckListItem *lastItem = 0;
    int index;
    for (int i = 0; i < count; i++) {
        item = new QCheckListItem(menuTable, "", QCheckListItem::CheckBox);
        QString menu = allMenus[i];
        index = menuList.findIndex(menu);
        item->setText(1, menuLabelList[index]);
        if (topLevel.findIndex(menu) != -1) {
            item->setOn(TRUE);
        }
        if (lastItem != 0) {
            item->moveItem(lastItem);
        }
        lastItem = item;
    }

    QHBox *hbox = new QHBox(menusTab);
    layout->addWidget(hbox);
    QPushButton *upButton = new QPushButton(PBDialog::tr("Up"), hbox);
    connect(upButton, SIGNAL(clicked()), this, SLOT(menuUp()));
    QPushButton *downButton = new QPushButton(PBDialog::tr("Down"), hbox);
    connect(downButton, SIGNAL(clicked()), this, SLOT(menuDown()));

    tabs->addTab(menusTab, tr("Menus"));
}

void Preferences::addButtonsTab(QTabWidget *tabs)
{
    QWidget *buttonsTab = new QWidget(tabs);
    QVBoxLayout *layout = new QVBoxLayout(buttonsTab);

    buttonTable = new QListView(buttonsTab);
    layout->addWidget(buttonTable);
    buttonTable->setAllColumnsShowFocus(TRUE);
    buttonTable->setSorting(-1);
    buttonTable->header()->setMovingEnabled(FALSE);
    buttonTable->addColumn(tr("Show"));
    buttonTable->addColumn(tr("Toolbar Button"));

    buttonList.append("Save");
    buttonLabelList.append(MenuActions::tr("Save"));
    buttonResourceList.append("portabase/save");

    buttonList.append("Add");
    buttonLabelList.append(MenuActions::tr("Add"));
    buttonResourceList.append("new");

    buttonList.append("Edit");
    buttonLabelList.append(MenuActions::tr("Edit"));
    buttonResourceList.append("edit");

    buttonList.append("Copy");
    buttonLabelList.append(MenuActions::tr("Copy"));
    buttonResourceList.append("copy");

    buttonList.append("Delete");
    buttonLabelList.append(MenuActions::tr("Delete"));
    buttonResourceList.append("trash");

    buttonList.append("QuickFilter");
    buttonLabelList.append(MenuActions::tr("Quick Filter"));
    buttonResourceList.append("find");

    QStringList shown;
    QStringList hidden;
    buttonConfiguration(shown, hidden);
    // need to use this instead of buttonList to get the sequence correct
    QStringList allButtons = shown + hidden;

    int count = allButtons.count();
    QCheckListItem *item = 0;
    QCheckListItem *lastItem = 0;
    int index;
    for (int i = 0; i < count; i++) {
        item = new QCheckListItem(buttonTable, "", QCheckListItem::CheckBox);
        QString button = allButtons[i];
        index = buttonList.findIndex(button);
        item->setText(1, buttonLabelList[index]);
        QPixmap pixmap = Resource::loadPixmap(buttonResourceList[index]);
        item->setPixmap(1, pixmap);
        if (shown.findIndex(button) != -1) {
            item->setOn(TRUE);
        }
        if (lastItem != 0) {
            item->moveItem(lastItem);
        }
        lastItem = item;
    }

    QHBox *hbox = new QHBox(buttonsTab);
    layout->addWidget(hbox);
    QPushButton *upButton = new QPushButton(PBDialog::tr("Up"), hbox);
    connect(upButton, SIGNAL(clicked()), this, SLOT(buttonUp()));
    QPushButton *downButton = new QPushButton(PBDialog::tr("Down"), hbox);
    connect(downButton, SIGNAL(clicked()), this, SLOT(buttonDown()));

    tabs->addTab(buttonsTab, tr("Buttons"));
}

void Preferences::menuUp()
{
    moveSelectionUp(menuTable);
}

void Preferences::menuDown()
{
    moveSelectionDown(menuTable);
}

void Preferences::menuConfiguration(QStringList &top, QStringList &file)
{
    Config conf("portabase");
    conf.setGroup("Layout");
    top += conf.readListEntry("TopLevelMenus", ',');
    file += conf.readListEntry("UnderFileMenus", ',');
    // if menu preferences aren't set yet, use the original configuration
    if (top.count() == 0 && file.count() == 0) {
        top.append("View");
        top.append("Sort");
        top.append("Filter");
        file.append("Row");
    }
}

void Preferences::buttonUp()
{
    moveSelectionUp(buttonTable);
}

void Preferences::buttonDown()
{
    moveSelectionDown(buttonTable);
}

void Preferences::buttonConfiguration(QStringList &shown, QStringList &hidden)
{
    Config conf("portabase");
    conf.setGroup("Layout");
    if (conf.hasKey("ToolbarButtons")) {
        shown += conf.readListEntry("ToolbarButtons", ',');
    }
    else {
        // button preferences aren't set yet, use the original configuration
        shown.append("Save");
        shown.append("Add");
        shown.append("Edit");
        shown.append("Delete");
    }
    QStringList allButtons;
    allButtons.append("Save");
    allButtons.append("Add");
    allButtons.append("Edit");
    allButtons.append("Copy");
    allButtons.append("Delete");
    allButtons.append("QuickFilter");
    int count = allButtons.count();
    for (int i = 0; i < count; i++) {
        QString button = allButtons[i];
        if (shown.findIndex(button) == -1) {
            hidden.append(button);
        }
    }
}

void Preferences::moveSelectionUp(QListView *table)
{
    QListViewItem *item = table->selectedItem();
    if (item == 0) {
        return;
    }
    QListViewItem *sibling = table->firstChild();
    QListViewItem *before = 0;
    while (sibling != item && sibling != 0) {
        before = sibling;
        sibling = sibling->nextSibling();
    }
    if (before != 0) {
        before->moveItem(item);
    }
}

void Preferences::moveSelectionDown(QListView *table)
{
    QListViewItem *item = table->selectedItem();
    if (item == 0) {
        return;
    }
    QListViewItem *after = item->nextSibling();
    if (after != 0) {
        item->moveItem(after);
    }
}

void Preferences::updateSizes(int selection)
{
    int currentSize = 0;
    if (fontSize->count() > 0) {
        currentSize = sizes[fontSize->currentItem()];
    }
    sizes = fontdb.pointSizes(fontName->text(selection));
    int count = sizes.count();
    if (count == 0) {
        QFont currentFont = qApp->font();
        int size = currentFont.pointSize();
        sizes.append(size);
        count = 1;
    }
    fontSize->clear();
    int newIndex = 0;
    for (int i = 0; i < count; i++) {
        fontSize->insertItem(QString::number(sizes[i] / sizeFactor));
        if (sizes[i] <= currentSize) {
            newIndex = i;
        }
    }
    fontSize->setCurrentItem(newIndex);
    if (currentSize > 0) {
        updateSample(newIndex);
    }
}

void Preferences::updateSample(int sizeSelection)
{
    QString name = fontName->currentText();
    int size = sizes[sizeSelection] / sizeFactor;
    QFont font(name, size);
    sample->setFont(font);
}

QFont Preferences::applyChanges()
{
    applyDateTimeChanges();
    Config conf("portabase");
    conf.setGroup("General");
    conf.writeEntry("ConfirmDeletions", confirmDeletions->isChecked());
    conf.writeEntry("BooleanToggle", booleanToggle->isChecked());
    conf.writeEntry("ShowSeconds", showSeconds->isChecked());
    conf.writeEntry("PagedDisplay", pagedDisplay->isChecked());
    conf.writeEntry("NoteWrap", noteWrap->isChecked());
    conf.writeEntry("WrapAnywhere", wrapType->currentItem() == 1);
    conf.writeEntry("RowsPerPage", rowsPerPage->value());

    conf.setGroup("Colors");
    const QColor evenColor = evenButton->getColor();
    PortaBase::evenRowColor = new QColor(evenColor);
    conf.writeEntry("EvenRows", evenColor.name());
    const QColor oddColor = oddButton->getColor();
    PortaBase::oddRowColor = new QColor(oddColor);
    conf.writeEntry("OddRows", oddColor.name());

    conf.setGroup("Layout");
    QStringList topLevel;
    QStringList underFile;
    QCheckListItem *item = (QCheckListItem*)menuTable->firstChild();
    int index;
    while (item != 0) {
        QString label = item->text(1);
        index = menuLabelList.findIndex(label);
        QString menu = menuList[index];
        if (item->isOn()) {
            topLevel.append(menu);
        }
        else {
            underFile.append(menu);
        }
        item = (QCheckListItem*)item->nextSibling();
    }
    conf.writeEntry("TopLevelMenus", topLevel, ',');
    conf.writeEntry("UnderFileMenus", underFile, ',');

    QStringList buttons;
    item = (QCheckListItem*)buttonTable->firstChild();
    while (item != 0) {
        QString label = item->text(1);
        index = buttonLabelList.findIndex(label);
        QString button = buttonList[index];
        if (item->isOn()) {
            buttons.append(button);
        }
        item = (QCheckListItem*)item->nextSibling();
    }
    conf.writeEntry("ToolbarButtons", buttons, ',');

    conf.setGroup("Font");
    QString name = fontName->currentText();
    int size = sizes[fontSize->currentItem()] / sizeFactor;
    conf.writeEntry("Name", name);
    conf.writeEntry("Size", size);
    QFont font(name, size);
    qApp->setFont(font);
    return font;
}

void Preferences::applyDateTimeChanges()
{
#if !defined(Q_WS_QWS)
    Config config("qpe");
    config.setGroup("Date");
    PBDateFormat df = date_formats[dateFormatCombo->currentItem()];
    config.writeEntry("Separator", QString(df.separator()));
    config.writeEntry("ShortOrder", df.shortOrder());
    config.writeEntry("LongOrder", df.longOrder());
    config.setGroup("Time");
    config.writeEntry("AMPM", ampmCombo->currentItem());
    config.writeEntry("MONDAY", weekStartCombo->currentItem());
#endif
}
