/*
 * qqutil/qqhelpbrowser.cpp
 *
 * (c) 2003-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file qqutil/qqhelpbrowser.cpp
 * Source file for QQHelpBrowser
 */

#include <QDialogButtonBox>
#include <QIcon>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QTextBrowser>
#include <QToolButton>
#include "qqhelpbrowser.h"

/**
 * Constructor.
 * @param resource Resource path of the help file to be opened.
 * @param parent The dialog's parent widget.
 */
QQHelpBrowser::QQHelpBrowser(const QString &resource, QWidget *parent)
  : QQDialog(tr("Help"), parent)
{
    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);
    setLayout(vbox);

    QTextBrowser *content = new QTextBrowser(this);
    vbox->addWidget(content);
    content->setSource(resource);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    vbox->addWidget(buttonBox);

#if defined(Q_WS_MAC)
    QAbstractButton *backButton = new QToolButton(buttonBox);
    QAbstractButton *forwardButton = new QToolButton(buttonBox);
#else
    QAbstractButton *backButton = new QPushButton(buttonBox);
    QAbstractButton *forwardButton = new QPushButton(buttonBox);
#endif
    backButton->setIcon(QIcon(":/icons/back.png"));
    backButton->setToolTip(tr("Back"));
    connect(backButton, SIGNAL(clicked()), content, SLOT(backward()));
    buttonBox->addButton(backButton, QDialogButtonBox::ActionRole);

    forwardButton->setIcon(QIcon(":/icons/forward.png"));
    forwardButton->setToolTip(tr("Forward"));
    connect(forwardButton, SIGNAL(clicked()), content, SLOT(forward()));
    buttonBox->addButton(forwardButton, QDialogButtonBox::ActionRole);

    finishConstruction(600, 400);
}
