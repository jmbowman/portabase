/*
 * noteeditor.cpp
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
#include <qlabel.h>
#include <qpushbutton.h>
#include "desktop/config.h"
#include "desktop/resource.h"
#else
#include <qpe/config.h>
#endif

#include <qlayout.h>
#include <qmultilineedit.h>
#include "noteeditor.h"

NoteEditor::NoteEditor(QString colName, bool readOnly, QWidget *parent,
    const char *name, WFlags f) : QDialog(parent, name, TRUE, f)
{
    setCaption(colName + " - " + tr("PortaBase"));
    QVBoxLayout *vbox = new QVBoxLayout(this);
#if defined(Q_WS_WIN)
    setSizeGripEnabled(TRUE);
    vbox->addWidget(new QLabel("<center><b>" + colName + "</b></center>",
                               this));
#endif
    textBox = new QMultiLineEdit(this);
    vbox->addWidget(textBox);
    textBox->setReadOnly(readOnly);
    Config conf("portabase");
    conf.setGroup("General");
    if (conf.readBoolEntry("NoteWrap", TRUE)) {
        textBox->setWordWrap(QMultiLineEdit::WidgetWidth);
        if (conf.readBoolEntry("WrapAnywhere")) {
            textBox->setWrapPolicy(QMultiLineEdit::Anywhere);
        }
    }

#if defined(DESKTOP)
    QHBox *hbox = new QHBox(this);
    vbox->addWidget(hbox);
    new QWidget(hbox);
    QPushButton *okButton = new QPushButton(tr("OK"), hbox);
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    new QWidget(hbox);
    if (!readOnly) {
        QPushButton *cancelButton = new QPushButton(tr("Cancel"), hbox);
        connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
        new QWidget(hbox);
    }
    vbox->setResizeMode(QLayout::FreeResize);
    setMinimumWidth(600);
    setMinimumHeight(400);
    setIcon(Resource::loadPixmap("portabase"));
#else
    showMaximized();
#endif
    textBox->setFocus();
}

NoteEditor::~NoteEditor()
{

}

QString NoteEditor::content()
{
    return textBox->text();
}

void NoteEditor::setContent(QString text)
{
    textBox->setText(text);
}
