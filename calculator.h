/***************************************************************************
                          kmymoneycalculator.h  -  description
                             -------------------
    begin                : Sat Oct 19 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CALCULATOR_H
#define CALCULATOR_H

#include "qqdialog.h"

class QLabel;

/**
  *@author Thomas Baumgart
  */

/**
  * This class implements a simple electronic calculator with the
  * ability of addition, subtraction, multiplication and division
  * and percentage calculation. Memory locations are not available.
  *
  * The first operand and operation can be loaded from an external
  * source to switch from an edit-widget to the calculator widget
  * without having the user to re-type the data. See setInitialValues()
  * for details.
  */
class Calculator : public QQDialog  {
    Q_OBJECT
public:
    Calculator(QWidget* parent = 0, const char *name = 0);
    ~Calculator();

    /**
     * This methods is used to extract the result of the last
     * calculation. The fractional part is separated from the
     * integral part by the character setup using setDecimal().
     *
     * @return QString representing the result of the
     *         last operation
     */
    const QString result() const;

    /**
     * This method is used to set the character to be used
     * as the separator between the integer and fractional part
     * of an operand.
     *
     * @param ch QChar representing the character to be used
     */
    void setDecimal(const QChar ch) { m_decimal = ch; };

    /**
     * This method is used to preset the first operand
     *
     * @param value reference to QString representing the operands value
     */
    void setInitialValue(const QString& value);

protected:
    void keyPressEvent(QKeyEvent* ev);

protected slots:
    /**
     * This method appends the digit represented by the parameter
     * to the current operand
     *
     * @param button integer value of the digit to be added in the
     *               range [0..9]
     */
    void digitClicked(int button);

    /**
     * This methods starts the operation contained in the parameter
     *
     * @param button The Qt::Keycode for the button pressed or clicked
     */
    void calculationClicked(int button);

    /**
     * This method appends a period (comma) to initialize the fractional
     * part of an operand. The period is only appended once.
     */
    void decimalClicked();

    /**
     * This method reverses the sign of the current operand
     */
    void plusminusClicked();

    /**
     * This method clears the current operand
     */
    void clearClicked();

    /**
     * This method clears all registers
     */
    void clearAllClicked();

    /**
     * This method executes the percent operation
     */
    void percentClicked();

    /**
     * This method updates the display of the calculator with
     * the text passed as argument
     *
     * @param str reference to QString containing the new display contents
     */
    void changeDisplay(const QString& str);

private:
    /**
     * This member variable stores the current (second) operand
     */
    QString operand;

    /**
     * This member variable stores the last result
     */
    QString m_result;

    /**
     * This member variable stores the representation of the
     * character to be used to separate the integer and fractional
     * part of numbers. The internal representation is always a period.
     */
    QChar m_decimal;

    /**
     * The numeric representation of the first operand
     */
    double op1;

    /**
     * This member stores the operation to be performed between
     * the first and the second operand.
     */
    int op;

    /**
     * This member stores a pointer to the display area
     */
    QLabel *display;

    /**
     * This member array stores the pointers to the various
     * buttons of the calculator. It is setup during the
     * constructor of this object
     */
    QPushButton *buttons[20];

    /**
     * This enumeration type stores the values used for the
     * various keys internally
     */
    enum {
        /* 0-9 are used by digits */
        DECIMAL = 10,
        /*
         * make sure, that PLUS through EQUAL remain in
         * the order they are. Otherwise, check the calculation
         * signal mapper
         */
        PLUS,
        MINUS,
        SLASH,
        STAR,
        EQUAL,
        PLUSMINUS,
        PERCENT,
        CLEAR,
        CLEARALL,
        /* insert new buttons before this line */
        MAX_BUTTONS
    };
};

#endif
