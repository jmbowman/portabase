/*
 * datedialog.cpp
 *
 * (c) 2009-2010,2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file datedialog.cpp
 * Source file for DateDialog
 */

#include <QAction>
#include <QCalendarWidget>
#include <QDialogButtonBox>
#include <QLayout>
#include <QPushButton>
#include <QSettings>
#include <QTextCharFormat>

#include "datedialog.h"
#include "factory.h"
#include "qqutil/qqspinbox.h"

/**
 * Constructor.
 *
 * @param date The date to initially select in the calendar
 * @param parent This dialog's parent widget
 */
DateDialog::DateDialog(const QDate &date, QWidget *parent)
#if defined(Q_WS_HILDON)
  : PBDialog(tr("Select a date"), parent, true), noneSelected(false)
#else
  : PBDialog(tr("Select a date"), parent), noneSelected(false)
#endif
{
    if (!date.isValid()) {
        reject();
    }
    calendar = new QCalendarWidget(this);
#if defined(Q_OS_ANDROID)
    calendar->setStyleSheet(QStringLiteral("QAbstractItemView:disabled {color: rgb(160, 160, 160);}"));
    QWidget *navBarBackground = calendar->findChild<QWidget *>(QStringLiteral("qt_calendar_navigationbar"));
    int buttonPixels = Factory::dpToPixels(48);
    int paddingPixels = Factory::dpToPixels(16);
    navBarBackground->setStyleSheet(QStringLiteral(
            "QToolButton {height: %1px; min-width: %1px}"
            "QToolButton QMenu::item {padding: %2px %2px %2px %2px; border: 1px solid transparent}"
            "QToolButton QMenu::item::selected {border-color: black}"
            "QToolButton::menu-indicator {image: none}").arg(buttonPixels).arg(paddingPixels));
    QSpinBox *yearEdit = calendar->findChild<QSpinBox *>(QStringLiteral("qt_calendar_yearedit"));
    QQSpinBox::updateStyleSheet(yearEdit);
#elif defined(Q_WS_MAEMO_5)
    calendar->setStyleSheet("alternate-background-color: darkblue");
#endif
    // date limits are for backwards compatibility
    calendar->setMinimumDate(QDate(1752, 9, 15));
    calendar->setMaximumDate(QDate(7000, 12, 31));
    calendar->setGridVisible(true);
    calendar->setSelectedDate(date);
    QDate today = QDate::currentDate();
    QTextCharFormat format = calendar->dateTextFormat(today);
    format.setBackground(Qt::cyan);
    calendar->setDateTextFormat(QDate::currentDate(), format);
    QSettings settings;
    if (settings.value("DateTime/MONDAY", false).toBool()) {
        calendar->setFirstDayOfWeek(Qt::Monday);
    }
    connect(calendar, SIGNAL(activated(QDate)), this, SLOT(accept()));
    connect(calendar, SIGNAL(clicked(QDate)), this, SLOT(accept()));
    vbox->addWidget(calendar);

    QDialogButtonBox *buttonBox = finishLayout();
#if defined(Q_OS_ANDROID)
    Q_UNUSED(buttonBox)
    QAction *todayAction = new QAction(tr("Today"), this);
    connect(todayAction, SIGNAL(triggered()), this, SLOT(gotoToday()));
    addButton(todayAction, 0);
    QAction *noneAction = new QAction(tr("None"), this);
    connect(noneAction, SIGNAL(triggered()), this, SLOT(selectNone()));
    addButton(noneAction, 0);
#else
    QPushButton *todayButton = new QPushButton(tr("Today"));
    buttonBox->addButton(todayButton, QDialogButtonBox::ActionRole);
    connect(todayButton, SIGNAL(clicked()), this, SLOT(gotoToday()));
    QPushButton *noneButton = new QPushButton(tr("None"));
    buttonBox->addButton(noneButton, QDialogButtonBox::ActionRole);
    connect(noneButton, SIGNAL(clicked()), this, SLOT(selectNone()));
#endif
}

/**
 * Get the currently selected date.
 *
 * @return The selected date
 */
QDate DateDialog::selectedDate()
{
    if (noneSelected) {
        return QDate(1752, 9, 14);
    }
    else {
        return calendar->selectedDate();
    }
}

/**
 * Select today's date on the calendar.  Called when the "Today" button is
 * clicked.
 */
void DateDialog::gotoToday()
{
    calendar->setSelectedDate(QDate::currentDate());
    accept();
}

/**
 * Select the "None" date and close the dialog.  Called when the "None" button
 * is clicked.
 */
void DateDialog::selectNone()
{
    noneSelected = true;
    accept();
}
