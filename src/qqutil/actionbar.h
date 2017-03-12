/*
 * actionbar.h
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

/** @file qqutil/actionbar.h
 * Header file for ActionBar
 */

#ifndef ACTIONBAR_H
#define ACTIONBAR_H

#include <QWidget>
#include <QList>
#include <QAction>
#include <QMenu>
#include <QHBoxLayout>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QToolButton>
#include <QProxyStyle>
#include <QStyleOption>

/**
 * Toolbar similar to Android's Action Bar, can also be used on Desktop OS.
 * The action bar shows an icon, a title and any number of action buttons.
 * Also the title can have a menu of navigation actions.
 * <p>
 * If the buttons do not fit into the window, then they are displayed
 * as an "overflow" menu on the right side.
 * <p>
 * See http://developer.android.com/design/patterns/actionbar.html
 *
 * To be used within a vertical box layout this way:
 * <pre><code>
 * MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
 *     QVBoxLayout* layout=new QVBoxLayout(this);
 *     layout->setMargin(0);
 *     layout->setSizeConstraint(QLayout::SetNoConstraint);
 *
 *     // Action bar
 *     ActionBar* actionBar=new ActionBar(this);
 *     actionBar->setTitle("My App",false);
 *     actionBar->addNavigation(new QAction("News",this));
 *     actionBar->addNavigation(new QAction("Weather",this));
 *     actionBar->addButton(new QAction(QIcon(":icons/search"),"Search",this));
 *     actionBar->addButton(new QAction(QIcon(":icons/call"),"Call",this));
 *     actionBar->addButton(new QAction(QIcon(":icons/settings"),"Settings",this));
 *     layout->addWidget(actionBar);
 *
 *     // Content of main window below the action bar
 *     layout->addWidget(new QLabel("Hello",this));
 *     layout->addStretch();
 * }
 * </code></pre>
 * The layout of the main window must use the option QLayout::SetNoConstraint,
 * to support screen rotation on mpbile devices.
 * <p>
 * The action bar needs two icons in the resource file:
 * <ul>
 *     <li>QIcon(":icons/app") is used for the initial display.
 *     <li>QIcon(":icons/app_up") is used when you enable "up" navigation by calling setTitle().
 * </ul>
 */
class ActionBar : public QWidget {
    Q_OBJECT

public:
    /**
     * Constructor, creates an Activity bar with icon and title but no action buttons.
     * The icon is loaded from the resource file via QIcon(":icons/app").
     * The title is taken from QApplication::applicationDisplayName().
     *
     * @param parent Points to the parent window.
     */
    explicit ActionBar(QWidget *parent = 0);

    /**
     * Destructor.
     */
    ~ActionBar();

    /**
     * Adds an item to the navigation menu of the action bar.
     * @param action The action, containing at least a text and optionally an icon. The action emits signal triggered() when clicked.
     */
    void addMenuItem(QAction* action);

    /**
     * Adds many items to the navigation menu of the action bar.
     * @param actions List of actions.
     * @see addAction()
     */
    void addMenuItems(QList<QAction*> actions);

    /**
     * Removes an item from the navigation menu of the action bar.
     * @param action The action that had been added before.
     */
    void removeMenuItem(QAction* action);

    /**
     * Set title of the action bar.
     * @param title Either the name of the application or title of the current view within the application.
     * @param showUpButton Enables "up" navigation. Then the action bar emits signal up() on click on the icon.
     */
    void setTitle(const QString& title, bool showUpButton);

    /** Get the current title of the action bar. */
    QString getTitle();

    /**
     * Adds a view navigation link to the title of the action bar.
     * @param action The action, containing at least a text and optionally an icon. The action emits signal triggered() when clicked.
     */
    void addView(QAction* action);

    /**
     * Adds many view navigation links to the title of the action bar.
     * @param actions List of actions.
     * @see addAction()
     */
    void addViews(QList<QAction*> actions);

    /**
     * Removes a view navigation link from the title of the action bar.
     * @param action The action that had been added before.
     */
    void removeView(QAction* action);

    /**
     * Adds an action button (or overflow menu item) to the action bar.
     * @param action The action, containing at least a text and optinally an icon. The action emits signal triggered() when clicked.
     * @param position Insert before this position. 0=before first button, 1=second button. Default is -1=append to the end.
     */
    void addButton(QAction* action, int position=-1);

    /**
     * Removes an action button (or overflow menu item) from the action bar.
     * @param action The action that had been added before.
     */
    void removeButton(QAction* action);

    /**
     * Adjust the number of buttons in the action bar. Buttons that don't fit go into the overflow menu.
     * You need to call this method after adding, removing or changing the visibility of an action.
     */
    void adjustContent();

signals:
    /** Emitted when the user clicks on the app icon (for "up" navigation) */
    void up();

public slots:
    /** Can be used to open the overflow menu */
    void openOverflowMenu();

protected:
    /**
     * Overrides the paintEvent method to draw background color properly.
     */
    void paintEvent(QPaintEvent* event);

    /**
     * Overrides the resizeEvent to adjust the content depending on the new size.
     */
    void resizeEvent(QResizeEvent* event);

private:
    /** Horizontal layout of the navigation bar */
    QHBoxLayout* layout;

    /** The Button that contains the applications icon, used for "up" navigation. */
    QToolButton* appIcon;

    /** The menu that appears when the user clicks on the navigation menu button */
    QMenu* navigationMenu;

    /** The button that contains the title, used for view navigation. */
    QToolButton* viewControl;

    /** The menu that appears when the user clicks on the title. */
    QMenu* viewMenu;

    /** List of actions for the action buttons and overflow menu. */
    QList<QAction*> buttonActions;

    /** List of action buttons, same order as buttonActions. */
    QList<QToolButton*> actionButtons;

    /** Overflow button, is only visible when there are more buttons than available space. */
    QToolButton* overflowButton;

    /** The menu that appears when the user clicks on the overflow button. */
    QMenu* overflowMenu;

    /** Style sheet template for this widget, taking its height in pixels as a parameter */
    QString styleSheetTemplate;

private slots:
    /** Listener for changes in action enabled/disabled status */
    void actionChanged();

    /** Internally used to forward events from the appIcon button. */
    void appIconClicked();

    /** Listener for navigation menu open signals */
    void aboutToShowNavigationMenu();

    /** Listener for navigation menu close signals */
    void aboutToHideNavigationMenu();

    /** Listener for screen rotation events */
    void screenGeometryChanged(const QRect &geometry);
};

#endif // ACTIONBAR_H
