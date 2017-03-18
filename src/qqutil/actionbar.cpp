/*
 * actionbar.cpp
 *
 * (c) 2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 * (c) 2015 by Muhammad Bashir Al-Noimi
 * (c) 2014 by Stefan Frings
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file qqutil/actionbar.cpp
 * Source file for ActionBar
 */

#include <QIcon>
#include <QFont>
#include <QApplication>
#include <QScreen>
#include <QStyleOption>
#include <QPainter>
#include "actionbar.h"
#include "qqfactory.h"

ActionBar::ActionBar(QWidget *parent) : QWidget(parent) {
    int paddingPixels = QQFactory::dpToPixels(16);
    int minWidth = QQFactory::dpToPixels(48);
    styleSheetTemplate = QString(
            "* {background:lightGray}"
            "QToolButton {height: %3px; min-width: %2px}"
            "QToolButton QMenu::item {padding: %1px %1px %1px %1px; border: 1px solid transparent}"
            "QToolButton QMenu::indicator {image: none}"
            "QToolButton QMenu::item::selected {border-color: black}"
            "QToolButton#viewControl {font:bold}"
            "QToolButton::menu-indicator {image: none}").arg(paddingPixels).arg(minWidth);
    QScreen *screen = qApp->primaryScreen();
    QSizeF physicalSize = screen->physicalSize();
    if (qMax(physicalSize.width(), physicalSize.height()) > 145) {
        // Over 6.5" screen, this is a tablet
        setStyleSheet(styleSheetTemplate.arg(QQFactory::dpToPixels(56)));
    }
    else {
        // Phone, height changes when the screen is rotated
        screenGeometryChanged(screen->geometry());
        connect(screen, &QScreen::geometryChanged, this, &ActionBar::screenGeometryChanged);
    }

    // Create layout
    layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->setContentsMargins(0,0,0,0);
    layout->setSizeConstraint(QLayout::SetNoConstraint);

    // App Icon, Up, and Navigation Button
    appIcon=new QToolButton();
    appIcon->setIcon(QIcon(":/icons/app_icon.svg"));
    appIcon->setAutoRaise(true);
    appIcon->setFocusPolicy(Qt::NoFocus);
    appIcon->setPopupMode(QToolButton::InstantPopup);
    navigationMenu = new QMenu(appIcon);
    connect(navigationMenu, &QMenu::aboutToHide, this, &ActionBar::aboutToHideNavigationMenu);
    connect(navigationMenu, &QMenu::aboutToShow, this, &ActionBar::aboutToShowNavigationMenu);
    layout->addWidget(appIcon);

    // View Control Button
    viewControl=new QToolButton();
    viewControl->setObjectName("viewControl");
    viewControl->setText(QApplication::applicationDisplayName());
    viewControl->setAutoRaise(true);
    viewControl->setFocusPolicy(Qt::NoFocus);
    viewControl->setPopupMode(QToolButton::InstantPopup);
    viewMenu=new QMenu(viewControl);
    viewControl->setMenu(viewMenu);
    layout->addWidget(viewControl);

    // Spacer
    layout->addStretch(1);

    // Action Overflow Button
    overflowButton = new QToolButton();
    overflowButton->setIcon(QIcon(":/icons/overflow.svg"));
    overflowButton->setAutoRaise(true);
    overflowButton->setFocusPolicy(Qt::NoFocus);
    overflowButton->setPopupMode(QToolButton::InstantPopup);
    overflowMenu = new QMenu(overflowButton);
    overflowButton->setMenu(overflowMenu);
    layout->addWidget(overflowButton);
}

ActionBar::~ActionBar() {}

void ActionBar::resizeEvent(QResizeEvent* event) {
    int oldWidth=event->oldSize().width();
    int newWidth=event->size().width();
    if (oldWidth!=newWidth) {
        adjustContent();
    }
}

void ActionBar::paintEvent(QPaintEvent*) {
     QStyleOption opt;
     opt.init(this);
     QPainter p(this);
     style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ActionBar::screenGeometryChanged(const QRect &geometry)
{
    if (geometry.height() > geometry.width()) {
        setStyleSheet(styleSheetTemplate.arg(QQFactory::dpToPixels(48)));
    }
    else {
        setStyleSheet(styleSheetTemplate.arg(QQFactory::dpToPixels(40)));
    }
}

void ActionBar::setTitle(const QString& title, bool showUpButton) {
    viewControl->setText(title);
    if (!navigationMenu->isEmpty()) {
        appIcon->setIcon(QIcon(":/icons/menu.svg"));
        disconnect(appIcon, &QToolButton::clicked, this, &ActionBar::appIconClicked);
        appIcon->setMenu(navigationMenu);
    }
    else if (showUpButton) {
        (qApp->layoutDirection() == Qt::LeftToRight) ? appIcon->setIcon(QIcon(":/icons/app_up")) : appIcon->setIcon(QIcon(":/icons/app_up_rtl"));
        connect(appIcon, &QToolButton::clicked, this, &ActionBar::appIconClicked);
        appIcon->setMenu(0);
    }
    else {
        appIcon->setIcon(QIcon(":/icons/app_icon.svg"));
        disconnect(appIcon, &QToolButton::clicked, this, &ActionBar::appIconClicked);
        appIcon->setMenu(0);
    }
    adjustContent();
}

QString ActionBar::getTitle() {
    return viewControl->text();
}

void ActionBar::appIconClicked() {
    emit up();
}

void ActionBar::adjustContent() {
    int screenWidth = qApp->primaryScreen()->availableSize().width();

    if (!navigationMenu->isEmpty()) {
        appIcon->setIcon(QIcon(":/icons/menu.svg"));
        disconnect(appIcon, &QToolButton::clicked, this, &ActionBar::appIconClicked);
        appIcon->setMenu(navigationMenu);
    }

    viewMenu->repaint();
    overflowButton->hide();

    // Check if all action buttons fit into the available space.
    for (int i = 0; i < buttonActions.size(); i++) {
        QAction *action = buttonActions.at(i);
        QToolButton *button = actionButtons.at(i);
        if (action->isVisible()) {
            button->show();
        }
    }
    if (sizeHint().width() > screenWidth) {
        // The buttons don't fit, we need an overflow menu.
        overflowButton->show();
        overflowMenu->clear();
    }
    else {
        overflowButton->hide();
    }

    // Show/Hide action buttons and overflow menu entries
    QAction *lastAction = 0;
    for (int i = buttonActions.size() - 1; i >= 0; i--) {
        QAction *action = buttonActions.at(i);
        QToolButton *button = actionButtons.at(i);
        if (action->isVisible()) {
            if (sizeHint().width() <= screenWidth) {
                // show as button
                button->show();
            }
            else {
                // show as overflow menu entry
                button->hide();
                overflowMenu->insertAction(lastAction, action);
                lastAction = action;
            }
        }
    }
}

void ActionBar::addMenuItem(QAction* action) {
    QWidget::addAction(action);
    navigationMenu->addAction(action);
    if (!navigationMenu->isEmpty()) {
        appIcon->setMenu(navigationMenu);
    }
}

void ActionBar::addMenuItems(QList<QAction*> actions) {
    QWidget::addActions(actions);
    for (int i = 0; i < actions.size(); i++) {
        addAction(actions.at(i));
    }
}

void ActionBar::removeMenuItem(QAction* action) {
    QWidget::removeAction(action);
    navigationMenu->removeAction(action);
    if (navigationMenu->isEmpty()) {
        appIcon->setMenu(0);
    }
}

void ActionBar::addView(QAction* action) {
    QWidget::addAction(action);
    viewMenu->addAction(action);
    if (!viewMenu->isEmpty()) {
        viewControl->setMenu(viewMenu);
    }
}

void ActionBar::addViews(QList<QAction*> actions) {
    QWidget::addActions(actions);
    for (int i=0; i<actions.size(); i++) {
        addAction(actions.at(i));
    }
}

void ActionBar::removeView(QAction* action) {
    QWidget::removeAction(action);
    viewMenu->removeAction(action);
    if (viewMenu->isEmpty()) {
        viewControl->setMenu(NULL);
    }
}

void ActionBar::addButton(QAction* action, int position) {
    if (position == -1) {
        position = buttonActions.size();
    }
    buttonActions.insert(position, action);
    QToolButton* button = new QToolButton();
    button->setText(action->text());
    QIcon icon = action->icon();
    if (!icon.isNull()) {
        button->setIcon(action->icon());
        button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    }
    button->setEnabled(action->isEnabled());
    button->setFocusPolicy(Qt::NoFocus);
    button->setAutoRaise(true);
    connect(button, &QToolButton::clicked, action, &QAction::trigger);
    connect(action, &QAction::changed, this, &ActionBar::actionChanged);
    actionButtons.insert(position, button);
    layout->insertWidget(position + 3, button);
}

void ActionBar::removeButton(QAction* action) {
    QToolButton* button=NULL;
    for (int i = 0; i < buttonActions.size(); i++) {
        if (buttonActions.at(i)==action) {
            button=actionButtons.at(i);
            break;
        }
    }
    if (button) {
        layout->removeWidget(button);
        actionButtons.removeOne(button);
        delete button;
        buttonActions.removeOne(action);
    }
}

void ActionBar::openOverflowMenu() {
    if (overflowButton->isVisible()) {
        overflowButton->click();
    }
}

void ActionBar::actionChanged()
{
    QAction *action = qobject_cast<QAction *>(sender());
    int index = buttonActions.indexOf(action);
    actionButtons[index]->setEnabled(action->isEnabled());
    actionButtons[index]->setIcon(action->icon());
}

void ActionBar::aboutToHideNavigationMenu()
{
    appIcon->setIcon(QIcon(":/icons/menu.svg"));
}

void ActionBar::aboutToShowNavigationMenu()
{
    (qApp->layoutDirection() == Qt::LeftToRight) ? appIcon->setIcon(QIcon(":/icons/app_up")) : appIcon->setIcon(QIcon(":/icons/app_up_rtl"));
}
