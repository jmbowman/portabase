/*
 * qqmenuhelper.cpp
 *
 * (c) 2005-2011,2015-2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file qqutil/qqmenuhelper.cpp
 * Source file for QQMenuHelper
 */

#include <QAction>
#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFont>
#include <QLocale>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>
#include <QRegExp>
#include <QSettings>
#include <QUrl>
#include "qqfactory.h"
#include "qqfiledialog.h"
#include "qqmenuhelper.h"
#include "qqtoolbar.h"

#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#endif

QRegExp QQMenuHelper::menuRegExp("\\(&\\w\\)");

/**
 * Constructor.
 * @param window The main application window.
 * @param fileDescription The description of the document file type as it is
 *                        to appear in file dialogs
 * @param fileExtension The file extension of the document file type (do not
 *                      include the period)
 * @param newFileLaunchesDialog True if creating a new file launches a dialog
 *                              to set additional parameters, false otherwise
 */
QQMenuHelper::QQMenuHelper(QMainWindow *window, const QString &fileDescription,
                           const QString &fileExtension,
                           bool newFileLaunchesDialog)
 : QObject(window), mainWindow(window), description(fileDescription),
   extension(fileExtension), file(0), help(0)
{
    QChar ellipsis(8230);

    // Some phrases need to be handled differently on Mac OS X
    QString prefsText = menuText(tr("Pr&eferences"));
    QString quitText = menuText(tr("&Quit"));
    QString macQuitText = QMenuBar::tr("Quit %1");
    QString helpText = tr("Help Contents");
    QString macHelpText = tr("%1 Help").arg(qApp->applicationName());
    QString aboutText = menuText(tr("&About %1")).arg(qApp->applicationName());
    QString macAboutText = QMenuBar::tr("About %1");
    QString aboutQtText = menuText(tr("About &Qt"));
#ifdef Q_OS_MAC
    quitText = macQuitText;
    helpText = macHelpText;
    aboutText = macAboutText;
#endif

#ifndef Q_OS_ANDROID
    documentToolBar = new QQToolBar(window, "documentToolbar");
    fileSelectorToolBar = new QQToolBar(window, "fileSelectorToolbar");
#endif

    // File menu actions
    QString fileNewText(tr("&New"));
    if (newFileLaunchesDialog) {
      fileNewText += ellipsis;
    }
    fileNewAction = new QAction(QQFactory::icon("new"), menuText(fileNewText), window);
    fileNewAction->setObjectName("New File");
    fileNewAction->setStatusTip(tr("Create a new file"));
    fileNewAction->setToolTip(fileNewAction->statusTip());
    fileNewAction->setShortcut(QKeySequence::New);
    connect(fileNewAction, SIGNAL(triggered()), this, SLOT(emitNewFile()));

    fileOpenAction = new QAction(QQFactory::icon("open"), menuText(tr("&Open")) + ellipsis, window);
    fileOpenAction->setObjectName("Open File");
    fileOpenAction->setStatusTip(tr("Open an existing file"));
    fileOpenAction->setToolTip(fileOpenAction->statusTip());
    fileOpenAction->setShortcut(QKeySequence::Open);
    connect(fileOpenAction, SIGNAL(triggered()), this, SLOT(emitOpenFile()));

    quitAction = new QAction(QQFactory::icon("quit"), quitText, window);
    quitAction->setStatusTip(tr("Quit the application"));
#if !defined(Q_WS_HILDON)
    quitAction->setShortcut(QKeySequence::Quit);
#endif
    quitAction->setMenuRole(QAction::QuitRole);
    connect(quitAction, SIGNAL(triggered()), this, SIGNAL(quit()));

    fileSaveAction = new QAction(QQFactory::icon("save"), menuText(tr("&Save")), window);
    fileSaveAction->setObjectName("Save");
    fileSaveAction->setStatusTip(tr("Save the current file"));
    fileSaveAction->setToolTip(fileSaveAction->statusTip());
    fileSaveAction->setShortcut(QKeySequence::Save);
    connect(fileSaveAction, SIGNAL(triggered()), this, SIGNAL(saveFile()));

    for (int i = 0; i < MAX_RECENT_FILES; i++) {
        // we'll set the actual paths later; just need actions that stick
        // around after a file is opened
        recentActions[i] = new QAction("", window);
        connect(recentActions[i], SIGNAL(triggered()), this, SLOT(openRecent()));
    }

    fileSeparatorAction = new QAction(this);
    fileSeparatorAction->setSeparator(true);

    closeAction = new QAction(QQFactory::icon("close"), menuText(tr("&Close")), window);
    closeAction->setStatusTip(tr("Close the current file"));
#if !defined(Q_OS_ANDROID)
    closeAction->setShortcut(QKeySequence::Close);
#endif
    connect(closeAction, SIGNAL(triggered()), this, SIGNAL(closeFile()));

    prefsAction = new QAction(prefsText, window);
    prefsAction->setStatusTip(tr("Change the application settings"));
#if !defined(Q_WS_HILDON)
    prefsAction->setShortcut(QKeySequence::Preferences);
#endif
#ifdef Q_OS_ANDROID
    prefsAction->setIcon(QQFactory::icon("preferences"));
#endif
    prefsAction->setMenuRole(QAction::PreferencesRole);
    connect(prefsAction, SIGNAL(triggered()), this, SIGNAL(editPreferences()));

#if defined(Q_OS_MAC) || defined(MOBILE)
    fileNewAction->setIconVisibleInMenu(false);
    fileOpenAction->setIconVisibleInMenu(false);
    quitAction->setIconVisibleInMenu(false);
    fileSaveAction->setIconVisibleInMenu(false);
    closeAction->setIconVisibleInMenu(false);
    prefsAction->setIconVisibleInMenu(false);
#endif
#if defined(Q_OS_MAC)
    docIcon = QIcon(":/icons/document_small.png");
    modifiedDocIcon = QIcon(darkenPixmap(QPixmap(":/icons/document_small.png")));
#endif

    // File menu basic setup
    file = new QMenu(menuText(tr("&File")), window);
    recent = new QMenu(menuText(tr("Open &Recent")), window);
    file->addAction(fileNewAction);
    file->addAction(fileOpenAction);
    file->addMenu(recent);
    file->addAction(fileSaveAction);
    file->addAction(fileSeparatorAction);
    file->addAction(closeAction);
    file->addAction(prefsAction);
    insertionPoint = fileSeparatorAction;
#if !defined(Q_OS_MAC)
    file->addSeparator();
#endif
    file->addAction(quitAction);
#ifndef MOBILE
    window->menuBar()->addMenu(file);
#endif

    // Help menu actions
    helpAction = new QAction(helpText, window);
    helpAction->setStatusTip(helpText);
    helpAction->setShortcut(QKeySequence::HelpContents);
    connect(helpAction, SIGNAL(triggered()), this, SLOT(showHelp()));

    aboutAction = new QAction(aboutText, window);
    aboutAction->setStatusTip(aboutText);
    aboutAction->setMenuRole(QAction::AboutRole);
    connect(aboutAction, SIGNAL(triggered()), this, SIGNAL(aboutApplication()));

    aboutQtAction = new QAction(aboutQtText, window);
    aboutQtAction->setStatusTip(aboutQtText);
    aboutQtAction->setMenuRole(QAction::AboutQtRole);
    connect(aboutQtAction, SIGNAL(triggered()), this, SLOT(aboutQt()));

    // Help menu setup
    help = new QMenu(menuText(tr("&Help")), window);
#ifndef MOBILE
    help->addAction(helpAction);
#if !defined(Q_OS_MAC)
    // skip this on the mac, since both "About.." actions get moved elsewhere
    help->addSeparator();
#endif
    help->addAction(aboutAction);
    help->addAction(aboutQtAction);

    window->menuBar()->addMenu(help);
#endif

    // toolbar setup
    addToFileSelectorToolBar(fileNewAction);
    addToFileSelectorToolBar(fileOpenAction);
    addToDocumentToolBar(fileSaveAction);

    // build the actions hash
    actions[New] = fileNewAction;
    actions[Open] = fileOpenAction;
    actions[Save] = fileSaveAction;
    actions[Recent1] = recentActions[0];
    actions[Recent2] = recentActions[1];
    actions[Recent3] = recentActions[2];
    actions[Recent4] = recentActions[3];
    actions[Recent5] = recentActions[4];
    actions[Recent6] = recentActions[5];
    actions[Recent7] = recentActions[6];
    actions[Recent8] = recentActions[7];
    actions[Recent9] = recentActions[8];
    actions[Recent10] = recentActions[9];
    actions[Separator] = fileSeparatorAction;
    actions[Close] = closeAction;
    actions[Preferences] = prefsAction;
    actions[Quit] = quitAction;
    actions[Help] = helpAction;
    actions[About] = aboutAction;
    actions[AboutQt] = aboutQtAction;
}

/**
 * Process an already translated string so it is suitable for inclusion in a
 * menu.  Some languages, such as Japanese and Chinese, represent Alt-key
 * accelerators by putting the appropriate letter in parentheses after the actual
 * text (since none of their native characters appear literally on the
 * keyboard). This is a problem on systems like Mac OS X, where the Alt-key
 * accelerators aren't supported; it just adds a few extra meaningless
 * characters after each menu item, and clashes with the look of other
 * applications.  So this method strips out such suffixes when appropriate.
 *
 * @param text The already-translated text to be adjusted
 * @return The appropriate string for inclusion in the menu on the current
 *         platform
 */
QString QQMenuHelper::menuText(QString text)
{
#if defined(Q_OS_MAC) || defined(Q_OS_ANDROID)
    return text.replace(menuRegExp, "").replace("&", "");
#else
    return text;
#endif
}

/**
 * <p>Load an assortment of saved application settings.  Typically called
 * early in the QMainWindow constructor.</p>
 * <ul>
 * <li>Replaces the default application font with the one saved in the
 * application settings (if present).  Doesn't do this on Mac OS X,
 * because it doesn't seem to stick well (keeps getting replaced by the
 * default font again upon certain actions, like the window regaining
 * focus).  The font settings are stored in the "Font" group, in the "Name"
 * and "Size" entries.</li>
 * <li>Loads the list of most recently opened files.  These are stored as
 * "Recent0", "Recent1", etc. in the "Files" group of the application
 * settings.</li>
 * <li>Loads the last directory where the application either opened or saved
 * a document file.  Replaces it with a reasonable default if the directory no
 * longer exists.  Stored in the "Files" group under "LastDir" in the
 * application settings.</li>
 * </ul>
 * @param settings The QSettings object from which to load the settings.
 */
void QQMenuHelper::loadSettings(QSettings *settings)
{
#if !defined(Q_OS_MAC) && !defined(Q_OS_ANDROID)
    // Load font settings
    QFont currentFont = qApp->font();
    QString family = currentFont.family().toLower();
    int size = currentFont.pointSize();
    family = settings->value("Font/Name", family).toString();
    size = settings->value("Font/Size", size).toInt();
    QFont font(family, size);
    qApp->setFont(font);
    mainWindow->setFont(font);
#endif
#ifndef Q_OS_ANDROID
    documentToolBar->loadSettings(settings);
    fileSelectorToolBar->loadSettings(settings);
#endif
    for (int i = 0; i < MAX_RECENT_FILES; i++) {
        QString key = QString("Files/Recent%1").arg(i);
        QString path = settings->value(key, "").toString();
        if (!path.isEmpty() && !QFileInfo(path).isDir()) {
            recentFiles.append(path);
        }
    }
}

/**
 * <p>Save an assortment of application settings to the specified QSettings
 * object.</p>
 * <ul>
 * <li>Updates the list of most recently opened files.</li>
 * <li>Updates the last directory where the application either opened or saved
 * a document file</li>
 * </ul>
 * @param settings The QSettings object to be updated.
 */
void QQMenuHelper::saveSettings(QSettings *settings)
{
#ifndef Q_OS_ANDROID
    documentToolBar->saveSettings(settings);
    fileSelectorToolBar->saveSettings(settings);
#endif
    // Save the list of recently opened files
    QStringList files(recentFiles);
    // Insert blank entries if necessary
    while (files.count() < MAX_RECENT_FILES) {
        files.append("");
    }
    for (int i = 0; i < MAX_RECENT_FILES; i++) {
        settings->setValue(QString("Files/Recent%1").arg(i), files[i]);
    }
}

/**
 * Get the path to the directory last used to open or save a file.  If it
 * doesn't exist, get the default document directory instead (and make sure
 * this is registered with the application settings).
 *
 * @param settings The QSettings object to look the current value up in
 */
QString QQMenuHelper::getLastDir(QSettings *settings)
{
    QString lastDir = settings->value("Files/LastDir", "").toString();
    if (lastDir.isEmpty() || !QDir(lastDir).exists()) {
#if defined(Q_OS_ANDROID)
        lastDir = QQFileDialog::defaultStoragePath();
#elif QT_VERSION >= 0x050000
        lastDir = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)[0];
#else
        lastDir = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
#endif
        if (lastDir.isEmpty()) {
            lastDir = QDir::homePath();
        }
        settings->setValue("Files/LastDir", lastDir);
    }
    return lastDir;
}

/**
 * Update the menu of most recently opened files.
 */
void QQMenuHelper::updateRecentMenu()
{
    recent->clear();
    int count = recentFiles.count();
    for (int i = 0; i < MAX_RECENT_FILES; i++) {
        if (i < count) {
            recentActions[i]->setText(recentFiles[i]);
            recent->addAction(recentActions[i]);
        }
        else {
            recentActions[i]->setText("");
        }
    }
    emit recentFilesChanged();
}

/**
 * Clear the menu of most recently opened files so that it is empty again.
 */
void QQMenuHelper::clearRecentMenu()
{
    recent->clear();
    recentFiles.clear();
    for (int i = 0; i < MAX_RECENT_FILES; i++) {
        recentActions[i]->setText("");
    }
    emit recentFilesChanged();
}

/**
 * Indicate whether the current document has been edited or not since it was
 * last saved, so that menu items (particularly the "Save" action) and the
 * main window decorations can be updated accordingly.
 * @param y True if the document has been edited, false otherwise.
 */
void QQMenuHelper::setEdited(bool y)
{
    fileSaveAction->setEnabled(y);
    mainWindow->setWindowModified(y);
#if defined(Q_OS_MAC)
    if (y) {
        mainWindow->setWindowIcon(modifiedDocIcon);
    }
    else {
        mainWindow->setWindowIcon(docIcon);
    }
#endif
}

/**
 * Show the application's main help file.  This should be an "index.html"
 * file in one of the following locations:
 * <ul>
 * <li>Linux/UNIX: /usr/share/[app_name]/help/[locale]/index.html</li>
 * <li>Mac OS X: [app_bundle]/Contents/Resources/[locale].lproj/index.html</li>
 * <li>Windows: [app_directory]/help/[locale]/index.html</li>
 * </ul>
 * The "en" locale is used by default if no help files in a more suitable
 * locale are available.  To override the directory path with a custom value
 * (to test a translation, for example), set that path as the value of the
 * APPLICATIONNAME_HELP environment variable.
 */
void QQMenuHelper::showHelp()
{
    QStringList env = QProcess::systemEnvironment();
    QString var = QString("%1_HELP").arg(qApp->applicationName().toUpper());
    QString helpDir;
    int index = env.indexOf(QRegExp(QString("%1=.*").arg(var)));
    if (index != -1) {
        QString helpDir = env[index];
        helpDir = helpDir.right(helpDir.length() - var.length() - 1);
    }
    else {
#if QT_VERSION >= 0x040800
        // Use the correct list of UI languages from the system locale
        QStringList langs = QLocale::system().uiLanguages();
#else
        // Hope that Qt isn't returning a formatting locale different from the
        // UI language one
        QStringList langs = QStringList(QLocale::system().name());
#endif
        QStringList dirNames;
        int count = langs.count();
        int i;
        QString lang;
        for (i = 0; i < count; i++) {
            lang = langs[0].replace(QLatin1Char('-'), QLatin1Char('_'));
            dirNames.append(lang);
            int j  = lang.indexOf('.');
            if (j > 0) {
                lang = lang.left(j);
            }
            j = lang.indexOf('_');
            if (j > 0) {
                dirNames.append(lang.left(j));
            }
        }
        // if no translation is available, show the English help
        dirNames.append("en");
#if defined(Q_OS_WIN)
        QString path = qApp->applicationDirPath() + "/help/";
        QString suffix = "/";
#elif defined(Q_OS_MAC)
        QString path = qApp->applicationDirPath() + "/../Resources/";
        QString suffix = ".lproj/";
#else
        QString path = QString("/usr/share/%1/help/")
                       .arg(qApp->applicationName().toLower());
        QString suffix = "/";
#endif
        count = dirNames.count();
        for (i = 0; i < count; i++) {
            QDir dir(path + dirNames[i] + suffix);
            if (dir.exists()) {
                helpDir = path + dirNames[i] + suffix;
                break;
            }
        }
    }
    if (!helpDir.isEmpty()) {
        QDir dir(helpDir);
        QString path = dir.absoluteFilePath("index.html");
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    }
}

/**
 * Display a dialog window containing information about the Qt library.
 */
void QQMenuHelper::aboutQt()
{
    QMessageBox::aboutQt(mainWindow, qApp->applicationName());
}

/**
 * <p>Adjust the "File" menu and toolbar contents as appropriate for the file
 * selection screen.  Sets the "File" menu to have the following content:</p>
 * <ul>
 * <li>New</li>
 * <li>Open</li>
 * <li>Open Recent <i>(submenu)</i></li>
 * <li>(items added using addToFileMenu() go here)</li>
 * <li>Preferences</li>
 * <li><i>(separator)</i></li>
 * <li>Quit</li>
 * </ul>
 * <p>Also shows the "New" and "Open" buttons on the toolbar (and on the Mac,
 * makes sure no document icon is shown in the titlebar).  This method should
 * be called after the visibility of any application-specific actions has
 * been updated (due to a Maemo implementation detail).</p>
 */
void QQMenuHelper::updateForFileSelector()
{
    // update action visibility
    fileNewAction->setVisible(true);
    fileOpenAction->setVisible(true);
    updateRecentMenu();
    recent->menuAction()->setVisible(true);
    fileSaveAction->setVisible(false);
    fileSeparatorAction->setVisible(false);
    closeAction->setVisible(false);

    // enable and disable actions as appropriate for the current state
    fileNewAction->setEnabled(true);
    fileOpenAction->setEnabled(true);
    fileSaveAction->setEnabled(false);
    closeAction->setEnabled(false);

#if defined(Q_WS_HILDON) || defined(Q_WS_MAEMO_5)
    QMenuBar *mb = mainWindow->menuBar();
    mb->clear();
    mb->addAction(fileNewAction);
    mb->addAction(fileOpenAction);
    int count = extraFileActions.count();
    for (int i = 0; i < count; i++) {
        if (extraFileActions[i]->isVisible()) {
            mb->addAction(extraFileActions[i]);
        }
    }
    mb->addAction(prefsAction);
    mb->addAction(helpAction);
    mb->addAction(aboutAction);
#endif

#ifndef Q_OS_ANDROID
    fileSelectorToolBar->show();
#endif

#if defined(Q_OS_MAC)
    mainWindow->setWindowIcon(QIcon());
#endif
}

/**
 * <p>Adjust the "File" menu and toolbar contents as appropriate for the
 * document screen.  Sets the "File" menu to have the following content:</p>
 * <ul>
 * <li>Save</li>
 * <li>(items added using addToFileMenu() go here)</li>
 * <li><i>(separator)</i></li>
 * <li>Close</li>
 * <li>Preferences</li>
 * <li><i>(separator)</i></li>
 * <li>Quit</li>
 * </ul>
 * <p>Also shows the "Save" button on the toolbar.  If creating
 * a new file which isn't automatically saved, follow this with a call
 * to setEdited(true).  On the Mac, also shows a document icon in the
 * titlebar.  This method should be called after the visibility of any
 * application-specific actions has been updated (due to a Maemo
 * implementation detail).</p>
 */
void QQMenuHelper::updateForDocument()
{
    // update action visibility
    fileSaveAction->setVisible(true);
    fileSeparatorAction->setVisible(true);
    closeAction->setVisible(true);
    fileNewAction->setVisible(false);
    fileOpenAction->setVisible(false);
    recent->menuAction()->setVisible(false);

    // enable and disable actions as appropriate for the current state
    fileNewAction->setEnabled(false);
    fileOpenAction->setEnabled(false);
    fileSaveAction->setEnabled(false);
    closeAction->setEnabled(true);

#if defined(Q_WS_HILDON) || defined(Q_WS_MAEMO_5)
    QMenuBar *mb = mainWindow->menuBar();
    mb->clear();
    int count = extraFileActions.count();
    for (int i = 0; i < count; i++) {
        if (extraFileActions[i]->isVisible()) {
            mb->addAction(extraFileActions[i]);
        }
    }
    mb->addAction(prefsAction);
    mb->addAction(helpAction);
#endif

#ifndef Q_OS_ANDROID
    documentToolBar->show();
#endif

#if defined(Q_OS_MAC)
    mainWindow->setWindowIcon(docIcon);
#endif
}

/**
 * Return the current "File" menu.  Usually called between the methods
 * which start and finish creating the menu, in order to add custom actions
 * and separators to it.
 * @return The current "File" menu.
 */
QMenu *QQMenuHelper::fileMenu()
{
    return file;
}

/**
 * Return the current "Help" menu.  Sometimes needed in order to change its
 * font, etc.
 * @return The current "Help" menu.
 */
QMenu *QQMenuHelper::helpMenu()
{
    return help;
}

/**
 * Add the specified action to the "File" menu just before the standard
 * footer items for the current mode (either "Preferences" or the separator
 * before "Close").
 *
 * @param action The action to be added to the File menu
 */
void QQMenuHelper::addToFileMenu(QAction *action)
{
    file->insertAction(insertionPoint, action);
    extraFileActions << action;
}

/**
 * Add the specified action to the document screen toolbar.
 */
void QQMenuHelper::addToDocumentToolBar(QAction *action)
{
#ifdef Q_OS_ANDROID
    Q_UNUSED(action)
#else
    documentToolBar->add(action);
#endif
}

/**
 * Add the specified action to the file selector screen toolbar.
 */
void QQMenuHelper::addToFileSelectorToolBar(QAction *action)
{
#ifdef Q_OS_ANDROID
    Q_UNUSED(action)
#else
    fileSelectorToolBar->add(action);
#endif
}

/**
 * Get the requested action object.  Occasionally needed externally, for
 * example if creating a button with equivalent functionality.
 *
 * @param actionId The ID of the action to get
 * @return The requested action
 */
QAction *QQMenuHelper::action(Action actionId)
{
    return actions.value(actionId);
}

/**
 * Determine if the file has been changed, and if so ask the user if they
 * would like to save the changes.  Emit the saveFile() signal if
 * appropriate.  Often called in an overridden QMainWindow::closeEvent();
 * if both the closeFile() and quit() signals are connected to the window's
 * close() slot, then both of these actions and a pressing of the window's
 * "X" button can all be handled in that method.
 *
 * @return The code returned from the message box (Yes, No, or Cancel)
 */
int QQMenuHelper::saveChangesPrompt()
{
    int choice = QMessageBox::Yes;
    if (fileSaveAction->isEnabled()) {
        choice = QMessageBox::warning(mainWindow, qApp->applicationName(),
                                      tr("Save changes?"),
                                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                      QMessageBox::Cancel);
        if (choice == QMessageBox::Yes) {
            emit saveFile();
        }
        else if (choice == QMessageBox::No) {
            setEdited(false);
        }
    }
    return choice;
}

/**
 * Start the process of creating a new file.  Asks the user where to create
 * it and what to name it.  If this is completed successfully, emit the
 * newFile() signal.
 */
void QQMenuHelper::emitNewFile()
{
    QString filename = createNewFile(description, extension);
    if (filename.isEmpty()) {
        return;
    }
    emit(newFile(filename));
}

/**
 * Launch a file dialog which allows the user to create a new file of the
 * specified type.  Used by <code>emitNewFile()</code> to create new native
 * document files, but can also be used to create files for exporting data,
 * etc.
 *
 * @param fileDescription The description of the file type as it is to appear
 *                        in the file dialog (null for the application default)
 * @param fileExtension The file extension of the file type (do not include
 *                      the period, use null for the application default)
 * @return The path of the file to be created (or an empty string if none)
 */
QString QQMenuHelper::createNewFile(const QString &fileDescription,
                      const QString &fileExtension)
{
    QString desc = fileDescription.isNull() ? description : fileDescription;
    QString ext = fileExtension.isNull() ? extension : fileExtension;
    QString filter = QString("%1 (*.%2)").arg(desc).arg(ext);
    QSettings settings;
    QString lastDir = getLastDir(&settings);
    QString filename = QQFileDialog::getSaveFileName(mainWindow,
                           tr("Choose a filename to save under"), lastDir,
                           filter);
    if (filename.isEmpty()) {
        return "";
    }
    if (!filename.endsWith(ext, Qt::CaseInsensitive)) {
        filename += QString(".%1").arg(ext);
    }
#if !defined(Q_OS_MAC)
    // The native Mac file dialog confirms overwrites internally
    if (QFile::exists(filename)) {
        int choice = QMessageBox::warning(mainWindow,
                             qApp->applicationName(),
                             tr("File already exists; overwrite it?"),
                             QMessageBox::Yes|QMessageBox::No,
                             QMessageBox::No);
        if (choice != QMessageBox::Yes) {
            return "";
        }
    }
#endif
    QFileInfo info(filename);
    settings.setValue("Files/LastDir", info.absolutePath());
    return filename;
}

/**
 * Handle a selection of the "File" menu's "Open" action by emitting
 * openFile() with an appropriate argument.  If no file has been selected,
 * return without doing anything.
 */
void QQMenuHelper::emitOpenFile()
{
    QSettings settings;
    QString lastDir = getLastDir(&settings);
    QString filter = QString("%1 (*.%2)").arg(description).arg(extension);
    QString filename = QQFileDialog::getOpenFileName(
                mainWindow, tr("Choose a file"), lastDir, filter);
    if (filename.isEmpty()) {
        return;
    }
    QFileInfo info(filename);
    settings.setValue("Files/LastDir", info.absolutePath());
    emit openFile(filename);
}

/**
 * Update the recent files list to reflect the fact that the file at the
 * specified path was just opened (or created).
 *
 * @param file The path of the file that was opened or created
 */
void QQMenuHelper::opened(const QString &file)
{
    QFileInfo info(file);
    QString absPath = info.absoluteFilePath();
    int index = recentFiles.indexOf(absPath);
    if (index == -1) {
        recentFiles.prepend(absPath);
        if (recentFiles.count() > MAX_RECENT_FILES) {
            recentFiles.removeLast();
        }
    }
    else if (index != 0) {
        recentFiles.removeAt(index);
        recentFiles.prepend(absPath);
    }
}

/**
 * Start the process of opening a file selected from the recent files menu.
 * Emit openFile() if the file still exists, otherwise display an error
 * message.
 */
void QQMenuHelper::openRecent()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action) {
        return;
    }
    QString path = action->text();
    if (!QFile::exists(path)) {
        QMessageBox::warning(mainWindow, qApp->applicationName(),
                             tr("File does not exist"));
        recentFiles.removeOne(path);
        updateRecentMenu();
        return;
    }
    if (QFileInfo(path).isDir()) {
        QMessageBox::warning(mainWindow, qApp->applicationName(),
                             tr("The selected item is a directory"));
    }
    emit openFile(path);
}

/**
 * Get a darker version of the provided pixmap.  Used on Mac OS X to show via
 * the titlebar icon that the current document has been modified.
 *
 * @param pixmap The pixmap to be darkened
 * @return The darkened version of the pixmap
 */
QPixmap QQMenuHelper::darkenPixmap(const QPixmap &pixmap)
{
    QImage img = pixmap.toImage().convertToFormat(QImage::Format_ARGB32);
    int imgh = img.height();
    int imgw = img.width();
    int h, s, v, a;
    QRgb pixel;
    for (int y = 0; y < imgh; ++y) {
        for (int x = 0; x < imgw; ++x) {
            pixel = img.pixel(x, y);
            a = qAlpha(pixel);
            QColor hsvColor(pixel);
            hsvColor.getHsv(&h, &s, &v);
            s = qMin(100, s * 2);
            v = v / 2;
            hsvColor.setHsv(h, s, v);
            pixel = hsvColor.rgb();
            img.setPixel(x, y, qRgba(qRed(pixel), qGreen(pixel), qBlue(pixel), a));
        }
    }
    return QPixmap::fromImage(img);
}
