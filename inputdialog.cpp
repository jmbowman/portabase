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

#include <qcombobox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qwidgetstack.h>
#include <qapplication.h>

class InputDialogPrivate
{
public:
    friend class InputDialog;
    QLineEdit *lineEdit;
    QComboBox *comboBox, *editComboBox;
    QWidgetStack *stack;
    InputDialog::Type type;
};

InputDialog::InputDialog(const QString &label, QWidget* parent,
                         const char* name, bool modal, Type type)
    : QDialog(parent, name, modal)
{
    d = new InputDialogPrivate;
    d->lineEdit = 0;
    d->comboBox = 0;

    QVBoxLayout *vbox = new QVBoxLayout(this, 6, 6);

    QLabel* l = new QLabel(label, this);
    vbox->addWidget(l);

    d->stack = new QWidgetStack(this);
    vbox->addWidget(d->stack);
    d->lineEdit = new QLineEdit(d->stack);
    d->comboBox = new QComboBox(FALSE, d->stack);
    d->editComboBox = new QComboBox(TRUE, d->stack);

    connect(d->lineEdit, SIGNAL(returnPressed()), this, SLOT(tryAccept()));

    resize(QMAX(sizeHint().width(), 240), sizeHint().height());
    setType(type);
}

/*!
  Returns the line edit, which is used in the LineEdit mode
*/

QLineEdit *InputDialog::lineEdit() const
{
    return d->lineEdit;
}

/*!
  Returns the combobox, which is used in the ComboBox mode
*/

QComboBox *InputDialog::comboBox() const
{
    return d->comboBox;
}

/*!
  Returns the combobox, which is used in the EditableComboBox mode
*/

QComboBox *InputDialog::editableComboBox() const
{
    return d->editComboBox;
}

/*!
  Sets the input type of the dialog to \a t.
*/

void InputDialog::setType(Type t)
{
    switch (t) {
    case LineEdit:
	d->stack->raiseWidget(d->lineEdit);
	d->lineEdit->setFocus();
	break;
    case ComboBox:
	d->stack->raiseWidget(d->comboBox);
	d->comboBox->setFocus();
	break;
    case EditableComboBox:
	d->stack->raiseWidget(d->editComboBox);
	d->editComboBox->setFocus();
	break;
    }
    d->type = t;
}

/*!
  Returns the input type of the dialog.

  \sa setType()
*/

InputDialog::Type InputDialog::type() const
{
    return d->type;
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
  QString text = InputDialog::getText( tr( "Please enter your name" ), QString::null, &ok, this );
  if ( ok && !text.isEmpty() )
      ;// user entered something and pressed ok
  else
      ;// user entered nothing or pressed cancel
  \endcode
*/

QString InputDialog::getText(const QString &caption, const QString &label,
                             QLineEdit::EchoMode, const QString &text,
                             bool *ok, QWidget *parent, const char *name)
{
    InputDialog *dlg = new InputDialog(label, parent, name, TRUE, LineEdit);
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
  Static convenience function to let the user select an item from a string list. \a caption is the text
  which is displayed in the title bar of the dialog. \a label is the text which
  is shown to the user (it should mention to the user what he/she should input), \a list the
  string list which is inserted into the combobox, \a current the number of the item which should
  be initially the current item, \a editable specifies if the combobox should be editable (if it is TRUE)
  or read-only (if \a editable is FALSE), \a ok a pointer to
  a bool which will be (if not 0!) set to TRUE if the user pressed ok or to FALSE if the
  user pressed cancel, \a parent the parent widget of the dialog and \a name
  the name of it. The dialogs pops up modally!

  This method returns the text of the current item, or if \a editable was TRUE, the current
  text of the combobox.

  You will use this static method like this:

  \code
  QStringList lst;
  lst << "First" << "Second" << "Third" << "Fourth" << "Fifth";
  bool ok = FALSE;
  QString res = QInputDialog::getItem( tr( "Please select an item" ), lst, 1, TRUE, &ok, this );
  if ( ok )
      ;// user selected an item and pressed ok
  else
      ;// user pressed cancel
  \endcode
*/

QString InputDialog::getItem(const QString &caption, const QString &label,
                             const QStringList &list, int current,
                             bool editable, bool *ok, QWidget *parent,
                             const char *name )
{
    InputDialog *dlg = new InputDialog(label, parent, name, TRUE,
                                       editable ? EditableComboBox : ComboBox);
    dlg->setCaption(caption);
    if (editable) {
        dlg->editableComboBox()->insertStringList(list);
        dlg->editableComboBox()->setCurrentItem(current);
    }
    else {
        dlg->comboBox()->insertStringList(list);
        dlg->comboBox()->setCurrentItem(current);
    }

    bool ok_ = FALSE;
    QString result;
    ok_ = dlg->exec() == QDialog::Accepted;
    if (ok)
	*ok = ok_;
    if (editable)
	result = dlg->editableComboBox()->currentText();
    else
	result = dlg->comboBox()->currentText();

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
