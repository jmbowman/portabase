/*
 * preferences.cpp
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qpe/config.h>
#include <qapplication.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qfont.h>
#include <qfontdatabase.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qvbox.h>
#include "preferences.h"

Preferences::Preferences(QWidget *parent, const char *name, WFlags f)
    : QDialog(parent, name, TRUE, f)
{
    setCaption(tr("Preferences") + " - " + tr("PortaBase"));
    vbox = new QVBox(this);
    vbox->resize(size());

    QGroupBox *fontGroup = new QGroupBox(2, Qt::Horizontal, tr("Font"), vbox);
    new QLabel(tr("Name"), fontGroup);
    fontName = new QComboBox(FALSE, fontGroup);
    fonts = fontdb.families();
    fontName->insertStringList(fonts);
    new QLabel(tr("Size"), fontGroup);
    fontSize = new QComboBox(FALSE, fontGroup);
    connect(fontName, SIGNAL(activated(int)), this, SLOT(updateSizes(int)));
    QFont currentFont = qApp->font();
    QString family = currentFont.family().lower();
    int index = fonts.findIndex(family);
    fontName->setCurrentItem(index);
    updateSizes(index);
    int size = currentFont.pointSize();
    index = sizes.findIndex(size * 10);
    if (index > -1) {
        fontSize->setCurrentItem(index);
    }
    connect(fontSize, SIGNAL(activated(int)), this, SLOT(updateSample(int)));
    new QLabel(tr("Sample"), fontGroup);
    sample = new QLabel(tr("Sample text"), fontGroup);

    QGroupBox *generalGroup = new QGroupBox(1, Qt::Horizontal, tr("General"),
                                            vbox);
    confirmDeletions = new QCheckBox(tr("Confirm deletions"), generalGroup);
    Config conf("portabase");
    conf.setGroup("General");
    if (conf.readNumEntry("ConfirmDeletions", 0)) {
        confirmDeletions->setChecked(TRUE);
    }
    else {
        confirmDeletions->setChecked(FALSE);
    }
    booleanToggle = new QCheckBox(tr("Allow checkbox edit in data viewer"),
                                  generalGroup);
    if (conf.readNumEntry("BooleanToggle", 0)) {
        booleanToggle->setChecked(TRUE);
    }
    else {
        booleanToggle->setChecked(FALSE);
    }
    new QWidget(vbox);

    showMaximized();
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
    int newIndex = 0;
    int count = sizes.count();
    fontSize->clear();
    for (int i = 0; i < count; i++) {
        fontSize->insertItem(QString::number(sizes[i] / 10));
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
    int size = sizes[sizeSelection] / 10;
    QFont font(name, size);
    sample->setFont(font);
}

QFont Preferences::applyChanges()
{
    Config conf("portabase");
    conf.setGroup("General");
    int confirm = 0;
    if (confirmDeletions->isChecked()) {
        confirm = 1;
    }
    conf.writeEntry("ConfirmDeletions", confirm);
    int toggle = 0;
    if (booleanToggle->isChecked()) {
        toggle = 1;
    }
    conf.writeEntry("BooleanToggle", toggle);
    conf.setGroup("Font");
    QString name = fontName->currentText();
    int size = sizes[fontSize->currentItem()] / 10;
    conf.writeEntry("Name", name);
    conf.writeEntry("Size", size);
    QFont font(name, size);
    qApp->setFont(font);
    return font;
}

void Preferences::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    vbox->resize(size());
}
