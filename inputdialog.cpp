/*
 * inputdialog.cpp
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "inputdialog.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qapplication.h>

class InputDialogPrivate
{
public:
    friend class InputDialog;
    QLineEdit *lineEdit;
};

InputDialog::InputDialog(const QString &label, QWidget* parent,
                         const char* name, bool modal)
    : QDialog( parent, name, modal )
{
    d = new InputDialogPrivate;
    d->lineEdit = 0;

    QVBoxLayout *vbox = new QVBoxLayout( this, 6, 6 );

    QLabel* l = new QLabel( label, this );
    vbox->addWidget( l );

    d->lineEdit = new QLineEdit( this );
    vbox->addWidget( d->lineEdit );

    connect( d->lineEdit, SIGNAL( returnPressed() ),
	     this, SLOT( tryAccept() ) );

    resize( QMAX( sizeHint().width(), 240 ), sizeHint().height() );
}

/*!
  Returns the line edit, which is used in the LineEdit mode
*/

QLineEdit *InputDialog::lineEdit() const
{
    return d->lineEdit;
}

/*!
  Destructor.
*/

InputDialog::~InputDialog()
{
    delete d;
}

/*!
  Static convenience function to get a textual input from the user. \a caption is the text
  which is displayed in the title bar of the dialog. \a label is the text which
  is shown to the user (it should mention to the user what he/she should input), \a text
  the default text which will be initially set to the line edit, \a ok a pointer to
  a bool which will be (if not 0!) set to TRUE if the user pressed ok or to FALSE if the
  user pressed cancel, \a parent the parent widget of the dialog and \a name
  the name of it. The dialogs pops up modally!

  This method returns the text which has been entered in the line edit.

  You will use this static method like this:

  \code
  bool ok = FALSE;
  QString text = QInputDialog::getText( tr( "Please enter your name" ), QString::null, &ok, this );
  if ( ok && !text.isEmpty() )
      ;// user entered something and pressed ok
  else
      ;// user entered nothing or pressed cancel
  \endcode
*/

QString InputDialog::getText(const QString &caption, const QString &label,
			     const QString &text, bool *ok, QWidget *parent,
                             const char *name )
{
    InputDialog *dlg = new InputDialog(label, parent, name, TRUE);
    dlg->setCaption(caption);
    dlg->lineEdit()->setText(text);
    dlg->lineEdit()->setEchoMode(QLineEdit::Normal);
    if (!text.isEmpty())
	dlg->lineEdit()->selectAll();

    bool ok_ = FALSE;
    QString result;
    ok_ = dlg->exec() == QDialog::Accepted;
    if (ok)
	*ok = ok_;
    if (ok_)
	result = dlg->lineEdit()->text();

    delete dlg;
    return result;
}

/*!
  \internal
*/

void InputDialog::tryAccept()
{
    if (!d->lineEdit->text().isEmpty())
	accept();
}
