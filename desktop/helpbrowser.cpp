/*
 * helpbrowser.cpp
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtextbrowser.h>
#include <qtoolbutton.h>
#include "helpbrowser.h"
#include "qpeapplication.h"
#include "resource.h"

HelpBrowser::HelpBrowser(QWidget *parent, const char *name, WFlags f)
  : QDialog(parent, name, TRUE, f)
{
    setCaption(tr("Help") + " - " + tr("PortaBase"));
    QVBoxLayout *vbox = new QVBoxLayout(this);
#if defined(Q_WS_WIN)
    setSizeGripEnabled(TRUE);
    vbox->setMargin(8);
    vbox->addWidget(new QLabel("<center><b>" + tr("Help") + "</b></center>",
                               this));
#endif
    QHBox *hbox = new QHBox(this);
    vbox->addWidget(hbox);

    QToolButton *backButton = new QToolButton(hbox);
    backButton->setIconSet(Resource::loadIconSet("back"));
    backButton->setTextLabel(tr("Back"), FALSE);
    backButton->setUsesTextLabel(TRUE);
    connect(backButton, SIGNAL(clicked()), this, SLOT(back()));

    QToolButton *forwardButton = new QToolButton(hbox);
    forwardButton->setIconSet(Resource::loadIconSet("forward"));
    forwardButton->setTextLabel(tr("Forward"), FALSE);
    forwardButton->setUsesTextLabel(TRUE);
    connect(forwardButton, SIGNAL(clicked()), this, SLOT(forward()));

    QWidget *filler = new QWidget(hbox);
    hbox->setStretchFactor(filler, 1);
    content = new QTextBrowser(this);
    vbox->addWidget(content);
    content->setSource(QPEApplication::helpDir() + "portabase.html");

    hbox = new QHBox(this);
    vbox->addWidget(hbox);
    new QWidget(hbox);
    QPushButton *okButton = new QPushButton(tr("OK"), hbox);
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    new QWidget(hbox);
    vbox->setResizeMode(QLayout::FreeResize);
    setMinimumWidth(600);
    setMinimumHeight(400);
}

HelpBrowser::~HelpBrowser()
{

}

void HelpBrowser::back()
{
    content->backward();
}

void HelpBrowser::forward()
{
    content->forward();
}
