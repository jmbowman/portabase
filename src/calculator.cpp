/*
 * calculator.cpp (based on kmymoneycalculator.cpp, by Thomas Baumgart)
 *
 * (c) 2002-2004,2008-2010,2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 * (c) 2000-2002 by Michael Edwardes <mte@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file calculator.cpp
 * Source file for Calculator
 */

#include <QAbstractButton>
#include <QDialogButtonBox>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QLocale>
#include <QSignalMapper>
#include <QRegExp>

#include "calculator.h"
#include "factory.h"
#include "formatting.h"

/**
 * Constructor.
 * @param parent The dialog's parent widget, if any.
 */
Calculator::Calculator(QWidget* parent)
  : PBDialog(tr("Calculator"), parent, true)
{
    setSizeGripEnabled(false);
    m_result = "";

    QGridLayout* grid = new QGridLayout();
    vbox->addLayout(grid);
    grid->setMargin(1);
    grid->setSpacing(2);
    grid->setSizeConstraint(QLayout::SetFixedSize);

    display = new QLabel(this);
#if !defined(Q_WS_MAEMO_5)
    display->setAutoFillBackground(true);
    QPalette displayPalette(display->palette());
    displayPalette.setColor(QPalette::Window, Qt::white);
    displayPalette.setColor(QPalette::WindowText, Qt::black);
    display->setPalette(displayPalette);
#endif
#if defined(Q_OS_ANDROID)
    vbox->addStretch(1);
    int pixels = Factory::dpToPixels(15);
    display->setMargin(pixels);
#endif

    display->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    display->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    grid->addWidget(display, 0, 0, 1, 5);

    int i;
    for (i = 0; i < MAX_BUTTONS; i++) {
        buttons[i] = Factory::button(this);
        buttons[i]->setSizePolicy(QSizePolicy::MinimumExpanding,
                                  QSizePolicy::Fixed);
    }
    buttons[0]->setText("0");
    buttons[1]->setText("1");
    buttons[2]->setText("2");
    buttons[3]->setText("3");
    buttons[4]->setText("4");
    buttons[5]->setText("5");
    buttons[6]->setText("6");
    buttons[7]->setText("7");
    buttons[8]->setText("8");
    buttons[9]->setText("9");
    buttons[PLUS]->setText("+");
    buttons[MINUS]->setText("-");
    buttons[STAR]->setText("X");
    buttons[DECIMAL]->setText(QLocale::system().decimalPoint());
    buttons[EQUAL]->setText("=");
    buttons[SLASH]->setText("/");
    buttons[CLEAR]->setText("C");
    buttons[CLEARALL]->setText("AC");
    buttons[PLUSMINUS]->setText("+-");
    buttons[PERCENT]->setText("%");

    grid->addWidget(buttons[7], 1, 0);
    grid->addWidget(buttons[8], 1, 1);
    grid->addWidget(buttons[9], 1, 2);
    grid->addWidget(buttons[4], 2, 0);
    grid->addWidget(buttons[5], 2, 1);
    grid->addWidget(buttons[6], 2, 2);
    grid->addWidget(buttons[1], 3, 0);
    grid->addWidget(buttons[2], 3, 1);
    grid->addWidget(buttons[3], 3, 2);
    grid->addWidget(buttons[0], 4, 1);

    grid->addWidget(buttons[DECIMAL], 4, 0);
    grid->addWidget(buttons[PLUS], 3, 3);
    grid->addWidget(buttons[MINUS], 4, 3);
    grid->addWidget(buttons[STAR], 3, 4);
    grid->addWidget(buttons[SLASH], 4, 4);
    grid->addWidget(buttons[EQUAL], 4, 2);
    grid->addWidget(buttons[PLUSMINUS], 2, 3);
    grid->addWidget(buttons[PERCENT], 2, 4);
    grid->addWidget(buttons[CLEAR], 1, 3);
    grid->addWidget(buttons[CLEARALL], 1, 4);

    int colWidth = buttons[CLEARALL]->sizeHint().width();
    for (i = 0; i < 5; i++) {
        grid->setColumnMinimumWidth(i, colWidth);
    }

    buttons[EQUAL]->setFocus();

    op1 = 0.0;
    op = 0;
    operand = "";
    changeDisplay("0");

    // connect the digit signals through a signal mapper
    QSignalMapper* mapper = new QSignalMapper(this);
    for (i = 0; i < 10; i++) {
        mapper->setMapping(buttons[i], i);
        connect(buttons[i], SIGNAL(clicked()), mapper, SLOT(map()));
    }
    connect(mapper, SIGNAL(mapped(int)), this, SLOT(digitClicked(int)));

    // connect the calculation operations through another mapper
    mapper = new QSignalMapper(this);
    for (i = PLUS; i <= EQUAL; i++) {
        mapper->setMapping(buttons[i], i);
        connect(buttons[i], SIGNAL(clicked()), mapper, SLOT(map()));
    }
    connect(mapper, SIGNAL(mapped(int)), this, SLOT(calculationClicked(int)));

    // connect all remaining signals
    connect(buttons[DECIMAL], SIGNAL(clicked()), SLOT(decimalClicked()));
    connect(buttons[PLUSMINUS], SIGNAL(clicked()), SLOT(plusminusClicked()));
    connect(buttons[PERCENT], SIGNAL(clicked()), SLOT(percentClicked()));
    connect(buttons[CLEAR], SIGNAL(clicked()), SLOT(clearClicked()));
    connect(buttons[CLEARALL], SIGNAL(clicked()), SLOT(clearAllClicked()));

    finishLayout();
}

/**
 * Append the digit represented by the parameter to the current operand
 *
 * @param button Integer value of the digit to be added in the range [0..9]
 */
void Calculator::digitClicked(int button)
{
    if (operand.length() >= 20) {
        return;
    }
    QChar newDigit(button + 0x30);
    QLocale locale = QLocale::system();
    QChar decimalPoint = locale.decimalPoint();
    if (operand.length() > 0 && !operand.contains(decimalPoint)) {
        // may need to change separator placement
        qlonglong value = locale.toLongLong(operand);
        operand = QString::number(value) + newDigit;
        value = operand.toLongLong();
        operand = locale.toString(value);
    }
    else {
        operand += newDigit;
    }
    changeDisplay(operand);
}

/**
 * Appends a period (comma) to initialize the fractional
 * part of an operand. The period is only appended once.
 */
void Calculator::decimalClicked()
{
    if (operand.length() == 0) {
        operand = "0";
    }
    QChar decimalPoint = QLocale::system().decimalPoint();
    if (operand.contains(decimalPoint) == 0) {
        operand += decimalPoint;
    }
    if (operand.length() > 16) {
        operand = operand.left(16);
    }
    changeDisplay(operand);
}

/**
 * Reverse the sign of the current operand.
 */
void Calculator::plusminusClicked()
{
    if (operand.length() == 0 && m_result.length() > 0) {
        operand = m_result;
    }
    if (operand.length() > 0) {
        QChar negativeSign = QLocale::system().negativeSign();
        if (operand[0] == negativeSign) {
            operand = operand.mid(1);
        }
        else {
            operand = QString(negativeSign) + operand;
        }
        changeDisplay(operand);
    }
}

/**
 * Start the operation contained in the parameter.
 *
 * @param button The Qt::Keycode for the button pressed or clicked
 */
void Calculator::calculationClicked(int button)
{
    if (operand.length() == 0 && op != 0 && button == EQUAL) {
        op = 0;
        m_result = QLocale::system().toString(op1);
        changeDisplay(m_result);
    }
    else if (operand.length() > 0 && op != 0) {
        // perform operation
        double op2 = QLocale::system().toDouble(operand);
        bool error = false;
        switch(op) {
            case PLUS:
                op2 = op1 + op2;
                break;
            case MINUS:
                op2 = op1 - op2;
                break;
            case STAR:
                op2 = op1 * op2;
                break;
            case SLASH:
                if (op2 == 0.0) {
                    error = true;
                }
                else {
                    op2 = op1 / op2;
                }
                break;
        }
        if (error) {
            op = 0;
            changeDisplay("Error");
            operand = "";
        }
        else {
            op1 = op2;
            m_result = QLocale::system().toString(op1);
            changeDisplay(m_result);
        }
    }
    else if(operand.length() > 0 && op == 0) {
        op1 = QLocale::system().toDouble(operand);
    }

    if (button != EQUAL) {
        op = button;
    }
    else {
        op = 0;
    }
    operand = "";
}

/**
 * Clear the current operand.
 */
void Calculator::clearClicked()
{
    if (operand.length() > 0) {
        QLocale locale = QLocale::system();
        QChar decimalPoint = locale.decimalPoint();
        if (!operand.contains(decimalPoint)) {
            // may need to change separator placement
            qlonglong value = locale.toLongLong(operand);
            operand = QString::number(value);
            operand = operand.left(operand.length() - 1);
            value = operand.toLongLong();
            operand = locale.toString(value);
        }
        else {
            operand = operand.left(operand.length() - 1);
        }
    }
    if (operand.length() == 0) {
        changeDisplay("0");
    }
    else {
        changeDisplay(operand);
    }
}

/**
 * Clear all registers.
 */
void Calculator::clearAllClicked()
{
    operand = "";
    op = 0;
    changeDisplay("0");
}

/**
 * Execute the percent operation.
 */
void Calculator::percentClicked()
{
    if (op != 0) {
        double op2 = QLocale::system().toDouble(operand) / 100;
        if (op == PLUS || op == MINUS) {
            op2 = op1 * op2;
        }
        operand = QLocale::system().toString(op2);
        calculationClicked(EQUAL);
    }
}

/**
 * Extract the result of the last calculation.
 *
 * @return The C-locale string representation of the result of the last
 *         operation
 */
const QString Calculator::result() const
{
    QString txt = operand;
    if (operand.isEmpty()) {
        txt = m_result;
    }
    return Formatting::fromLocalDouble(txt);
}

/**
 * Update the display of the calculator.
 *
 * @param str The new display contents
 */
void Calculator::changeDisplay(const QString& str)
{
    display->setText("<b>" + str + "</b>");
}

void Calculator::keyPressEvent(QKeyEvent* ev)
{
    int button = -1;

    switch(ev->key()) {
        case Qt::Key_0:
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
        case Qt::Key_6:
        case Qt::Key_7:
        case Qt::Key_8:
        case Qt::Key_9:
            button = ev->key() - Qt::Key_0;
            break;
        case Qt::Key_Plus:
            button = PLUS;
            break;
        case Qt::Key_Comma:
            button = DECIMAL;
            break;
        case Qt::Key_Minus:
            button = MINUS;
            break;
        case Qt::Key_Period:
            button = DECIMAL;
            break;
        case Qt::Key_Slash:
            button = SLASH;
            break;
        case Qt::Key_Backspace:
            button = CLEAR;
            break;
        case Qt::Key_Asterisk:
            button = STAR;
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
        case Qt::Key_Equal:
            button = EQUAL;
            break;
        case Qt::Key_Escape:
            button = CLEARALL;
            break;
        case Qt::Key_Percent:
            button = PERCENT;
            break;
        default:
            ev->ignore();
            break;
    }
    if (button != -1) {
        buttons[button]->animateClick();
    }
}

/**
 * Preset the first operand.
 *
 * @param value The operand's value
 */
void Calculator::setInitialValue(const QString& value)
{
    // setup operand
    operand = value;
    op1 = Formatting::parseDouble(value);
    m_result = QLocale::system().toString(op1);
    changeDisplay(m_result);
    operand = "";
}
