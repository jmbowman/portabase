/*
 * preferences.cpp
 *
 * (c) 2002-2004,2009-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file preferences.cpp
 * Source file for Preferences
 */

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QFont>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QTabWidget>
#include <QtColorPicker>
#include "factory.h"
#include "menuactions.h"
#include "pbdialog.h"
#include "portabase.h"
#include "preferences.h"

/**
 * Constructor.
 *
 * @param parent This dialog's parent widget
 */
Preferences::Preferences(QWidget *parent)
    : PBDialog(tr("Preferences"), parent)
{
    QTabWidget *tabs = new QTabWidget(this);
    vbox->addWidget(tabs);
    QSettings settings;
    addGeneralTab(tabs, &settings);
    addDateTimeTab(tabs, &settings);
    addAppearanceTab(tabs);
    finishLayout(true, true, 400, 200);
}

/**
 * Add the general options tab to the dialog.
 *
 * @param tabs The main tabbed widget stack
 * @param settings PortaBase's application settings
 */
void Preferences::addGeneralTab(QTabWidget *tabs, QSettings *settings)
{
    QWidget *generalTab = new QWidget(tabs);
    QVBoxLayout *layout = Factory::vBoxLayout(generalTab, true);
    settings->beginGroup("General");

    QHBoxLayout *hbox = Factory::hBoxLayout(layout);
    confirmDeletions = new QCheckBox(tr("Confirm deletions"), generalTab);
    confirmDeletions->setChecked(settings->value("ConfirmDeletions",
                                                true).toBool());
    hbox->addWidget(confirmDeletions);
    hbox->addStretch(1);

    hbox = Factory::hBoxLayout(layout);
    booleanToggle = new QCheckBox(tr("Allow checkbox edit in data viewer"),
                                  generalTab);
    booleanToggle->setChecked(settings->value("BooleanToggle",
                                             false).toBool());
    hbox->addWidget(booleanToggle);
    hbox->addStretch(1);

    hbox = Factory::hBoxLayout(layout);
    pagedDisplay = new QCheckBox(tr("Use pages in data viewer"), generalTab);
    pagedDisplay->setChecked(settings->value("PagedDisplay", true).toBool());
    hbox->addWidget(pagedDisplay);
    hbox->addStretch(1);

    hbox = Factory::hBoxLayout(layout);
    singleClickShow = new QCheckBox(tr("View rows with a single click"), generalTab);
    singleClickShow->setChecked(settings->value("SingleClickShow", true).toBool());
    hbox->addWidget(singleClickShow);
    hbox->addStretch(1);

    hbox = Factory::hBoxLayout(layout);
    noteWrap = new QCheckBox(tr("Wrap Notes"), generalTab);
    noteWrap->setChecked(settings->value("NoteWrap", true).toBool());
    hbox->addWidget(noteWrap);
    wrapType = new QComboBox(generalTab);
    wrapType->addItem(tr("at whitespace"));
    wrapType->addItem(tr("anywhere"));
    if (settings->value("WrapAnywhere", false).toBool()) {
        wrapType->setCurrentIndex(1);
    }
    wrapType->setEnabled(noteWrap->isChecked());
    connect(noteWrap, SIGNAL(toggled(bool)),
            wrapType, SLOT(setEnabled(bool)));
    hbox->addWidget(wrapType);

    hbox = Factory::hBoxLayout(layout);
    hbox->addWidget(new QLabel(tr("Default rows per page"), generalTab));
    rowsPerPage = new QSpinBox(generalTab);
    rowsPerPage->setRange(1, 9999);
    rowsPerPage->setValue(settings->value("RowsPerPage", 25).toInt());
    hbox->addWidget(rowsPerPage);

    hbox = Factory::hBoxLayout(layout);
    smallScreen = new QCheckBox(tr("Use small-screen settings on this device"),
                                generalTab);
#if defined(Q_WS_HILDON) || defined(Q_WS_MAEMO_5)
    bool smallDefault = true;
#else
    bool smallDefault = false;
#endif
    smallScreen->setChecked(settings->value("SmallScreen", smallDefault).toBool());
    hbox->addWidget(smallScreen);
    hbox->addStretch(1);

    layout->addStretch(1);
    tabs->addTab(generalTab, tr("General"));
    settings->endGroup();
}

/**
 * Add the date and time options tab to the dialog.
 *
 * @param tabs The main tabbed widget stack
 * @param settings PortaBase's application settings
 */
void Preferences::addDateTimeTab(QTabWidget *tabs, QSettings *settings)
{
    QWidget *dateTimeTab = new QWidget(tabs);
    QGridLayout *grid = Factory::gridLayout(dateTimeTab, true);
    settings->beginGroup("DateTime");

    grid->addWidget(new QLabel(tr("Date format"), dateTimeTab), 0, 0);
    QString df = settings->value("ShortDateFormat", "yyyy-MM-dd").toString();
    dateFormatCombo = new QComboBox(dateTimeTab);
    int currentdf = 0;
    dateFormats << "M/d/yyyy" << "d.M.yyyy" << "yyyy-MM-dd" << "d/M/yyyy";
    for (int i = 0; i < 4; i++) {
        QString format = QString(dateFormats[i]).replace("MM", "M");
        format = format.replace("dd", "d").replace("d", tr("D"));
        format = format.replace("M", tr("M")).replace("yyyy", tr("Y"));
        dateFormatCombo->addItem(format);
        if (df == dateFormats[i]) {
            currentdf = i;
        }
    }
    dateFormatCombo->setCurrentIndex(currentdf);
    grid->addWidget(dateFormatCombo, 0, 1);

    QString timeFormat = settings->value("TimeFormat", "hh:mm AP").toString();
    grid->addWidget(new QLabel(tr("Time format"), dateTimeTab), 1, 0);
    ampmCombo = new QComboBox(dateTimeTab);
    ampmCombo->addItem(tr("24 hour"));
    ampmCombo->addItem(tr("12 hour"));
    int show12hr = timeFormat.contains("AP") ? 1 : 0;
    ampmCombo->setCurrentIndex(show12hr);
    grid->addWidget(ampmCombo, 1, 1);

    grid->addWidget(new QLabel(tr("Weeks start on"), dateTimeTab), 2, 0);
    weekStartCombo = new QComboBox(dateTimeTab);
    weekStartCombo->addItem(tr("Sunday"));
    weekStartCombo->addItem(tr("Monday"));
    int startMonday =  settings->value("MONDAY", false).toBool() ? 1 : 0;
    weekStartCombo->setCurrentIndex(startMonday);
    grid->addWidget(weekStartCombo, 2, 1);

    showSeconds = new QCheckBox(tr("Show seconds for times"), dateTimeTab);
    showSeconds->setChecked(timeFormat.contains("ss"));
    grid->addWidget(showSeconds, 3, 0, 1, 2);

    grid->addWidget(new QWidget(dateTimeTab), 4, 0, 1, 2);
    grid->setRowStretch(4, 1);
    tabs->addTab(dateTimeTab, tr("Date and Time"));
    settings->endGroup();
}

/**
 * Add the appearance options tab to the dialog.
 *
 * @param tabs The main tabbed widget stack
 */
void Preferences::addAppearanceTab(QTabWidget *tabs)
{
    QWidget *appearanceTab = new QWidget(tabs);
    QVBoxLayout *layout = Factory::vBoxLayout(appearanceTab, true);
#if !defined(Q_WS_MAC)
    QGroupBox *fontGroup = new QGroupBox(tr("Font"), appearanceTab);
    QGridLayout *fontGrid = Factory::gridLayout(fontGroup, true);

    fontGrid->addWidget(new QLabel(tr("Name"), fontGroup), 0, 0);
    fontName = new QComboBox(fontGroup);
    QStringList fonts = fontdb.families();
    fontName->addItems(fonts);
    fontGrid->addWidget(fontName, 0, 1);

    fontGrid->addWidget(new QLabel(tr("Size"), fontGroup), 1, 0);
    fontSize = new QComboBox(fontGroup);
    connect(fontName, SIGNAL(activated(int)), this, SLOT(updateSizes(int)));
    QFont currentFont = qApp->font();
    int fontCount = fonts.count();
    QString family = currentFont.family().toLower();
    int index = -1;
    for (int i = 0; i < fontCount; i++) {
        if (family == fonts[i].toLower()) {
            index = i;
        }
    }
    // Windows defaults to a font not in QFontDatabase ???
    if (index == -1) {
        fontName->addItem(currentFont.family());
        index = fontName->count() - 1;
    }
    fontName->setCurrentIndex(index);
    updateSizes(index);
    int size = currentFont.pointSize();
    index = sizes.indexOf(size);
    if (index > -1) {
        fontSize->setCurrentIndex(index);
    }
    connect(fontSize, SIGNAL(activated(int)), this, SLOT(updateSample(int)));
    fontGrid->addWidget(fontSize, 1, 1);

    fontGrid->addWidget(new QLabel(tr("Sample"), fontGroup), 2, 0);
    sample = new QLabel(tr("Sample text"), fontGroup);
    fontGrid->addWidget(sample, 2, 1);
    layout->addWidget(fontGroup);
#endif

    QGroupBox *colorGroup = new QGroupBox(tr("Row Colors"), appearanceTab);
    layout->addWidget(colorGroup);
    QHBoxLayout *hbox = new QHBoxLayout(colorGroup);
    colorGroup->setLayout(hbox);
    evenButton = new QtColorPicker(colorGroup);
    configureColorPicker(evenButton);
    evenButton->setCurrentColor(Factory::evenRowColor);
    //evenButton->setSizePolicy(QSizePolicy::MinimumExpanding,
    //                          QSizePolicy::Fixed);
    hbox->addWidget(evenButton);
    oddButton = new QtColorPicker(colorGroup);
    configureColorPicker(oddButton);
    oddButton->setCurrentColor(Factory::oddRowColor);
    //oddButton->setSizePolicy(QSizePolicy::MinimumExpanding,
    //                         QSizePolicy::Fixed);
    hbox->addWidget(oddButton);

    layout->addStretch(1);
    tabs->addTab(appearanceTab, tr("Appearance"));
}

/**
 * Configure one of the row color selection buttons with appropriate settings
 * and color options.
 *
 * @param picker The color picker to be configured
 */
void Preferences::configureColorPicker(QtColorPicker *picker)
{
    picker->insertColor(Qt::white, tr("White"));
    picker->insertColor(QColor("aquamarine"), tr("Aquamarine"));
    picker->insertColor(QColor("lightblue"), tr("Light blue"));
    picker->insertColor(QColor("cyan"), tr("Cyan"));
    picker->insertColor(QColor("lightgray"), tr("Light gray"));
    picker->insertColor(QColor("lightgreen"), tr("Light green"));
    picker->insertColor(QColor("lavender"), tr("Lavender"));
    picker->insertColor(QColor("silver"), tr("Silver"));
    picker->insertColor(QColor("tan"), tr("Tan"));
    picker->insertColor(QColor("thistle"), tr("Thistle"));
    picker->setColorDialogEnabled(true);
}

/**
 * Update the list of available font sizes based on the currently selected
 * font.
 *
 * @param selection The index of the currently selected font
 */
void Preferences::updateSizes(int selection)
{
    int currentSize = 0;
    if (fontSize->count() > 0) {
        currentSize = sizes[fontSize->currentIndex()];
    }
    sizes = fontdb.pointSizes(fontName->itemText(selection));
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
        fontSize->addItem(QString::number(sizes[i]));
        if (sizes[i] <= currentSize) {
            newIndex = i;
        }
    }
    fontSize->setCurrentIndex(newIndex);
    if (currentSize > 0) {
        updateSample(newIndex);
    }
}

/**
 * Update the sample text which demonstrates the currently selected text
 * font and size.
 *
 * @param sizeSelection The index of the currently selected font size
 */
void Preferences::updateSample(int sizeSelection)
{
    QString name = fontName->currentText();
    int size = sizes[sizeSelection];
    QFont font(name, size);
    sample->setFont(font);
}

/**
 * Set the preferences currently selected in this dialog to be the new
 * application settings.  Returns the selected font in case the GUI needs to
 * be updated accordingly.
 *
 * @return The currently selected font
 */
QFont Preferences::applyChanges()
{
    QSettings settings;
    settings.beginGroup("DateTime");
    QString df = dateFormats[dateFormatCombo->currentIndex()];
    settings.setValue("ShortDateFormat", df);
    QString timeFormat("hh:mm");
    if (showSeconds->isChecked()) {
        timeFormat += ":ss";
    }
    if (ampmCombo->currentIndex() == 1) {
        timeFormat += " AP";
    }
    settings.setValue("TimeFormat", timeFormat);
    settings.setValue("MONDAY", weekStartCombo->currentIndex());
    settings.endGroup();

    settings.beginGroup("General");
    settings.setValue("ConfirmDeletions", confirmDeletions->isChecked());
    settings.setValue("BooleanToggle", booleanToggle->isChecked());
    settings.setValue("PagedDisplay", pagedDisplay->isChecked());
    settings.setValue("SingleClickShow", singleClickShow->isChecked());
    settings.setValue("NoteWrap", noteWrap->isChecked());
    settings.setValue("WrapAnywhere", wrapType->currentIndex() == 1);
    settings.setValue("RowsPerPage", rowsPerPage->value());
    settings.setValue("SmallScreen", smallScreen->isChecked());
    settings.endGroup();

    settings.beginGroup("Colors");
    const QColor evenColor = evenButton->currentColor();
    Factory::evenRowColor = QColor(evenColor);
    settings.setValue("EvenRows", evenColor.name());
    const QColor oddColor = oddButton->currentColor();
    Factory::oddRowColor = QColor(oddColor);
    settings.setValue("OddRows", oddColor.name());
    settings.endGroup();

#if !defined(Q_WS_MAC)
    settings.beginGroup("Font");
    QString name = fontName->currentText();
    int size = sizes[fontSize->currentIndex()];
    settings.setValue("Name", name);
    settings.setValue("Size", size);
    QFont font(name, size);
    qApp->setFont(font);
    settings.endGroup();
    return font;
#else
    return QFont();
#endif
}
