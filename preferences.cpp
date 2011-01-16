/*
 * preferences.cpp
 *
 * (c) 2002-2004,2009-2011 by Jeremy Bowman <jmbowman@alum.mit.edu>
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
#include <QScrollArea>
#include <QSettings>
#include <QSpinBox>
#include <QTabWidget>
#include "color_picker/qtcolorpicker.h"
#include "factory.h"
#include "menuactions.h"
#include "pbdialog.h"
#include "portabase.h"
#include "preferences.h"
#include "qqutil/qqmenuhelper.h"

/**
 * Constructor.
 *
 * @param menuHelper Menu management helper object
 * @param parent This dialog's parent widget
 */
Preferences::Preferences(QQMenuHelper *menuHelper, QWidget *parent)
    : PBDialog(tr("Preferences"), parent), mh(menuHelper), tabs(0), panel(0),
      variableHeightRows(0), fontName(0), fontSize(0), sample(0),
      autoRotate(0), useAlternating(0)
{
#if defined(Q_WS_MAEMO_5)
    QScrollArea *sa = new QScrollArea(this);
    panel = new QWidget();
    Factory::vBoxLayout(panel, true);
    sa->setWidgetResizable(true);
    sa->setWidget(panel);
    vbox->addWidget(sa);
#else
    tabs = new QTabWidget(this);
    vbox->addWidget(tabs);
#endif
    QSettings settings;
    addGeneralTab(&settings);
    addDateTimeTab(&settings);
    addAppearanceTab(&settings);
    finishLayout(true, true, 400, 200);
}

/**
 * Add the general options tab to the dialog.
 *
 * @param settings PortaBase's application settings
 */
void Preferences::addGeneralTab(QSettings *settings)
{
    QWidget *generalTab;
    QVBoxLayout *layout;
    if (tabs) {
        generalTab = new QWidget(tabs);
        layout = Factory::vBoxLayout(generalTab, true);
    }
    else {
        generalTab = panel;
        layout = static_cast<QVBoxLayout *>(panel->layout());
    }
    settings->beginGroup("General");

    confirmDeletions = new QCheckBox(tr("Confirm deletions"), generalTab);
    confirmDeletions->setChecked(settings->value("ConfirmDeletions",
                                                true).toBool());
    layout->addWidget(confirmDeletions);

    booleanToggle = new QCheckBox(tr("Allow checkbox edit in data viewer"),
                                  generalTab);
    booleanToggle->setChecked(settings->value("BooleanToggle",
                                             false).toBool());
    layout->addWidget(booleanToggle);

    singleClickShow = new QCheckBox(tr("View rows with a single click"), generalTab);
    singleClickShow->setChecked(settings->value("SingleClickShow", true).toBool());
    layout->addWidget(singleClickShow);

    smallScreen = new QCheckBox(tr("Use small-screen settings on this device"),
                                generalTab);
#if defined(Q_WS_HILDON) || defined(Q_WS_MAEMO_5)
    bool smallDefault = true;
#else
    bool smallDefault = false;
#endif
    smallScreen->setChecked(settings->value("SmallScreen", smallDefault).toBool());
    layout->addWidget(smallScreen);

    pagedDisplay = new QCheckBox(tr("Use pages in data viewer"), generalTab);
    pagedDisplay->setChecked(settings->value("PagedDisplay", false).toBool());
    layout->addWidget(pagedDisplay);

#if defined(Q_WS_MAEMO_5)
    autoRotate = new QCheckBox(tr("Auto-rotate to match device orientation"),
                               generalTab);
    autoRotate->setChecked(settings->value("AutoRotate", false).toBool());
    layout->addWidget(autoRotate);
#endif

    QHBoxLayout *hbox = Factory::hBoxLayout(layout);
    hbox->addWidget(new QLabel(tr("Default rows per page"), generalTab));
    rowsPerPage = new QSpinBox(generalTab);
    rowsPerPage->setRange(1, 9999);
    rowsPerPage->setValue(settings->value("RowsPerPage", 25).toInt());
    hbox->addWidget(rowsPerPage);

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

    if (tabs) {
        layout->addStretch(1);
        tabs->addTab(generalTab, tr("General"));
    }
    settings->endGroup();
}

/**
 * Add the date and time options tab to the dialog.
 *
 * @param settings PortaBase's application settings
 */
void Preferences::addDateTimeTab(QSettings *settings)
{
    QWidget *dateTimeTab;
    QGridLayout *grid;
    if (tabs) {
        dateTimeTab = new QWidget(tabs);
        grid = Factory::gridLayout(dateTimeTab, true);
    }
    else {
        dateTimeTab = panel;
        QVBoxLayout *layout = static_cast<QVBoxLayout *>(panel->layout());
        grid = Factory::gridLayout(layout);
    }
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
    if (tabs) {
        grid->setRowStretch(4, 1);
        tabs->addTab(dateTimeTab, tr("Date and Time"));
    }
    settings->endGroup();
}

/**
 * Add the appearance options tab to the dialog.
 */
void Preferences::addAppearanceTab(QSettings *settings)
{
    QWidget *appearanceTab;
    QVBoxLayout *layout;
    if (tabs) {
        appearanceTab = new QWidget(tabs);
        layout = Factory::vBoxLayout(appearanceTab, true);
    }
    else {
        appearanceTab = panel;
        layout = static_cast<QVBoxLayout *>(panel->layout());
    }
#if !defined(Q_WS_MAC)
    QGroupBox *fontGroup = new QGroupBox(tr("Font"), appearanceTab);
    layout->addWidget(fontGroup);
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
#endif

#if defined(Q_WS_MAEMO_5)
    variableHeightRows = new QCheckBox(tr("Adjust row height to match font"),
                                       fontGroup);
    variableHeightRows->setChecked(settings->value("Font/VariableHeightRows",
                                                   false).toBool());
    fontGrid->addWidget(variableHeightRows, 3, 0, 1, 2);

    useAlternating = new QCheckBox(
                         tr("Use alternating row colors if theme has them"),
                         appearanceTab);
    useAlternating->setChecked(settings->value("Colors/UseAlternating",
                                                     true).toBool());
    layout->addWidget(useAlternating);
#else
    Q_UNUSED(settings);
    QGroupBox *colorGroup = new QGroupBox(tr("Row Colors"), appearanceTab);
    layout->addWidget(colorGroup);
    QHBoxLayout *hbox = Factory::hBoxLayout(colorGroup, true);
    evenButton = new QtColorPicker(colorGroup);
    configureColorPicker(evenButton);
    evenButton->setCurrentColor(Factory::evenRowColor);
    hbox->addWidget(evenButton);
    hbox->addSpacing(10);
    oddButton = new QtColorPicker(colorGroup);
    configureColorPicker(oddButton);
    oddButton->setCurrentColor(Factory::oddRowColor);
    hbox->addWidget(oddButton);
#endif

    QPushButton *button = new QPushButton(tr("Clear the recent files list"),
                                          appearanceTab);
    connect(button, SIGNAL(clicked()), mh, SLOT(clearRecentMenu()));
    layout->addWidget(button);

    if (tabs) {
        layout->addStretch(1);
        tabs->addTab(appearanceTab, tr("Appearance"));
    }
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

#if defined(Q_WS_MAEMO_5)
    settings.setValue("General/AutoRotate", autoRotate->isChecked());
    settings.setValue("Font/VariableHeightRows",
                      variableHeightRows->isChecked());
    settings.setValue("Colors/UseAlternating", useAlternating->isChecked());
#else
    settings.beginGroup("Colors");
    const QColor evenColor = evenButton->currentColor();
    settings.setValue("EvenRows", evenColor.name());
    const QColor oddColor = oddButton->currentColor();
    settings.setValue("OddRows", oddColor.name());
    settings.endGroup();
#endif

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
