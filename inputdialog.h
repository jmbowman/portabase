/*
 * inputdialog.h
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

#ifndef QT_H
#include <qdialog.h>
#include <qstring.h>
#include <qlineedit.h>
#endif // QT_H

class InputDialogPrivate;
class QComboBox;

class InputDialog : public QDialog
{
    Q_OBJECT

private:
    enum Type { LineEdit, ComboBox, EditableComboBox };

    InputDialog(const QString &label, QWidget* parent = 0,
                const char* name = 0, bool modal = TRUE,
                Type type = LineEdit );
    ~InputDialog();

    QLineEdit *lineEdit() const;
    QComboBox *comboBox() const;
    QComboBox *editableComboBox() const;

    void setType( Type t );
    Type type() const;

public:
    static QString getText(const QString &caption, const QString &label,
                           QLineEdit::EchoMode echo,
                           const QString &text = QString::null,
                           bool *ok = 0, QWidget *parent = 0,
                           const char *name = 0);
    static QString getItem(const QString &caption, const QString &label,
                           const QStringList &list, int current = 0,
                           bool editable = TRUE, bool *ok = 0,
                           QWidget *parent = 0, const char *name = 0 );

private slots:
    void tryAccept();

private:
    InputDialogPrivate *d;
    // just to avoid warnings...
    friend class InputDialogPrivate;

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    InputDialog(const InputDialog &);
    InputDialog &operator=(const InputDialog &);
#endif
};

#endif // INPUTDIALOG_H
