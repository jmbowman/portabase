/*
 * preferences.cpp
 *
 * (c) 2002-2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#if defined(DESKTOP)
#include "desktop/config.h"
#else
#include <qpe/config.h>
#endif

#include <qapplication.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qfont.h>
#include <qfontdatabase.h>
#include <qgroupbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include "colorbutton.h"
#include "portabase.h"
#include "preferences.h"

Preferences::Preferences(QWidget *parent, const char *name, WFlags f)
    : PBDialog(tr("Preferences"), parent, name, f)
{
#if defined(DESKTOP)
    sizeFactor = 1;
#else
    sizeFactor = 10;
#endif
    QGroupBox *fontGroup = new QGroupBox(2, Qt::Horizontal, tr("Font"), this);
    vbox->addWidget(fontGroup);
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
    int i;
    for (i = 0; i < fontCount; i++) {
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

    QGroupBox *generalGroup = new QGroupBox(1, Qt::Horizontal, tr("General"),
                                            this);
    vbox->addWidget(generalGroup);
    Config conf("portabase");
    conf.setGroup("General");
    QHBox *hbox = new QHBox(generalGroup);
    confirmDeletions = new QCheckBox(tr("Confirm deletions"), hbox);
    confirmDeletions->setChecked(conf.readBoolEntry("ConfirmDeletions"));
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

    QGroupBox *colorGroup = new QGroupBox(2, Qt::Horizontal, tr("Row Colors"),
                                          this);
    vbox->addWidget(colorGroup);
    evenButton = new ColorButton(*PortaBase::evenRowColor, colorGroup);
    oddButton = new ColorButton(*PortaBase::oddRowColor, colorGroup);

#if defined(DESKTOP)
    QGroupBox *dateGroup = new QGroupBox(2, Qt::Horizontal,
                                         tr("Date and Time"), this);
    vbox->addWidget(dateGroup);
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
    for (i = 0; i < 4; i++) {
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
#else
    vbox->addWidget(new QWidget(this));
#endif
    finishLayout();
}

Preferences::~Preferences()
{

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
#if defined(DESKTOP)
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

    Config conf("portabase");
    conf.setGroup("General");
    conf.writeEntry("ConfirmDeletions", confirmDeletions->isChecked());
    conf.writeEntry("BooleanToggle", booleanToggle->isChecked());
    conf.writeEntry("ShowSeconds", showSeconds->isChecked());
    conf.writeEntry("NoteWrap", noteWrap->isChecked());
    conf.writeEntry("WrapAnywhere", wrapType->currentItem() == 1);

    conf.setGroup("Colors");
    const QColor evenColor = evenButton->getColor();
    PortaBase::evenRowColor = new QColor(evenColor);
    conf.writeEntry("EvenRows", evenColor.name());
    const QColor oddColor = oddButton->getColor();
    PortaBase::oddRowColor = new QColor(oddColor);
    conf.writeEntry("OddRows", oddColor.name());

    conf.setGroup("Font");
    QString name = fontName->currentText();
    int size = sizes[fontSize->currentItem()] / sizeFactor;
    conf.writeEntry("Name", name);
    conf.writeEntry("Size", size);
    QFont font(name, size);
    qApp->setFont(font);
    return font;
}
