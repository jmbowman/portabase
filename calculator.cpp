/***************************************************************************
                          kmymoneycalculator.cpp  -  description
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

#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qsignalmapper.h>
#include <qregexp.h>

#include "calculator.h"

Calculator::Calculator(QWidget* parent, const char *name)
  : QQDialog("", parent, name, TRUE)
{
    m_decimal = '.';
    m_result = "";

#if defined(Q_WS_QWS)
    int rows = 5;
#else
    int rows = 6;
#endif
    QGridLayout* grid = new QGridLayout(this, rows, 5, 1, 2);

    display = new QLabel(this);
    display->setBackgroundColor(QColor("#BDFFB4"));

    display->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    display->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    grid->addMultiCellWidget(display, 0, 0, 0, 4);

    buttons[0] = new QPushButton("0", this);
    buttons[1] = new QPushButton("1", this);
    buttons[2] = new QPushButton("2", this);
    buttons[3] = new QPushButton("3", this);
    buttons[4] = new QPushButton("4", this);
    buttons[5] = new QPushButton("5", this);
    buttons[6] = new QPushButton("6", this);
    buttons[7] = new QPushButton("7", this);
    buttons[8] = new QPushButton("8", this);
    buttons[9] = new QPushButton("9", this);
    buttons[PLUS] = new QPushButton("+", this);
    buttons[MINUS] = new QPushButton("-", this);
    buttons[STAR] = new QPushButton("X", this);
    buttons[DECIMAL] = new QPushButton(m_decimal, this);
    buttons[EQUAL] = new QPushButton("=", this);
    buttons[SLASH] = new QPushButton("/", this);
    buttons[CLEAR] = new QPushButton("C", this);
    buttons[CLEARALL] = new QPushButton("AC", this);
    buttons[PLUSMINUS] = new QPushButton("+-", this);
    buttons[PERCENT] = new QPushButton("%", this);

    int buttonWidth = 30;
    int i;
    for (i = 0; i < MAX_BUTTONS; i++) {
        buttons[i]->setMinimumWidth(buttonWidth);
        buttons[i]->setMaximumWidth(buttonWidth);
    }

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

#if !defined(Q_WS_QWS)
    QHBox *hbox = new QHBox(this);
    new QWidget(hbox);
    QPushButton *okButton = new QPushButton(tr("OK"), hbox);
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    new QWidget(hbox);
    QPushButton *cancelButton = new QPushButton(tr("Cancel"), hbox);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    new QWidget(hbox);
    grid->addMultiCellWidget(hbox, 5, 5, 0, 4);
    grid->setResizeMode(QLayout::FreeResize);
#endif
    finishConstruction(FALSE);
}

Calculator::~Calculator()
{

}

void Calculator::digitClicked(int button)
{
    operand += QChar(button + 0x30);
    if (operand.length() > 16) {
        operand = operand.left(16);
    }
    changeDisplay(operand);
}

void Calculator::decimalClicked()
{
    if (operand.length() == 0) {
        operand = "0";
    }
    if (operand.contains('.', FALSE) == 0) {
        operand += '.';
    }
    if (operand.length() > 16) {
        operand = operand.left(16);
    }
    changeDisplay(operand);
}

void Calculator::plusminusClicked()
{
    if (operand.length() == 0 && m_result.length() > 0) {
        operand = m_result;
    }
    if (operand.length() > 0) {
        if (operand[0] == '-') {
            operand = operand.mid(1);
        }
        else {
            operand = "-" + operand;
        }
        changeDisplay(operand);
    }
}

void Calculator::calculationClicked(int button)
{
    if (operand.length() == 0 && op != 0 && button == EQUAL) {
        op = 0;
        m_result.setNum(op1);
        changeDisplay(m_result);
    }
    else if (operand.length() > 0 && op != 0) {
        // perform operation
        double op2 = operand.toDouble();
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
            m_result.setNum(op1);
            changeDisplay(m_result);
        }
    }
    else if(operand.length() > 0 && op == 0) {
        op1 = operand.toDouble();
    }

    if (button != EQUAL) {
        op = button;
    }
    else {
        op = 0;
    }
    operand = "";
}

void Calculator::clearClicked()
{
    if (operand.length() > 0) {
        operand = operand.left(operand.length() - 1);
    }
    if (operand.length() == 0) {
        changeDisplay("0");
    }
    else {
        changeDisplay(operand);
    }
}

void Calculator::clearAllClicked()
{
    operand = "";
    op = 0;
    changeDisplay("0");
}

void Calculator::percentClicked()
{
    if (op != 0) {
        double op2 = operand.toDouble() / 100;
        if (op == PLUS || op == MINUS) {
            op2 = op1 * op2;
        }
        operand.setNum(op2);
        calculationClicked(EQUAL);
    }
}

const QString Calculator::result() const
{
    QString txt = operand;
    if (operand == "") {
        txt = m_result;
    }
    txt.replace(QRegExp("\\."), m_decimal);
    return txt;
}

void Calculator::changeDisplay(const QString& str)
{
    QString txt = str;
    txt.replace(QRegExp("\\."), m_decimal);
    display->setText("<b>" + txt + "</b>");
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

void Calculator::setInitialValue(const QString& value)
{
    // setup operand
    operand = value;
    operand.replace(QRegExp(QString("\\") + m_decimal), ".");
    op1 = value.toDouble();
    m_result.setNum(op1);
    changeDisplay(m_result);
    operand = "";
}
