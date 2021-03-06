/*
 * calculator.h (based on kmymoneycalculator.h, by Thomas Baumgart)
 *
 * (c) 2002-2004,2008-2010,2015 by Jeremy Bowman <jmbowman@alum.mit.edu>
 * (c) 2000-2002 by Michael Edwardes <mte@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file calculator.h
 * Header file for Calculator
 */

#ifndef CALCULATOR_H
#define CALCULATOR_H

#include "pbdialog.h"

class QLabel;
class QAbstractButton;

/**
  * This class implements a simple electronic calculator with the
  * ability of addition, subtraction, multiplication and division
  * and percentage calculation. Memory locations are not available.
  *
  * The first operand and operation can be loaded from an external
  * source to switch from an edit-widget to the calculator widget
  * without the user having to re-type the data. See setInitialValues()
  * for details.
  */
class Calculator : public PBDialog  {
    Q_OBJECT
public:
    explicit Calculator(QWidget* parent = 0);

    const QString result() const;
    void setInitialValue(const QString& value);

protected:
    void keyPressEvent(QKeyEvent* ev);

protected slots:
    void digitClicked(int button);
    void calculationClicked(int button);
    void decimalClicked();
    void plusminusClicked();
    void clearClicked();
    void clearAllClicked();
    void percentClicked();
    void changeDisplay(const QString& str);

private:
    QString operand; /**< The current (second) operand */
    QString m_result; /**< The last result */
    double op1; /**< The numeric representation of the first operand */
    int op; /**< The operation to be performed between the first and the second operand */
    QLabel *display; /**< A pointer to the display area */
    QAbstractButton *buttons[20]; /**< Pointers to the various buttons of the calculator */

    /**
     * The values used for the various keys internally
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
