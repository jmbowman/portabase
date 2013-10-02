/*
 * qqmainwindow.cpp
 *
 * (c) 2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file qqutil/qqmainwindow.cpp
 * Source file for QQMainWindow
 */

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QEvent>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPrintDialog>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QProcess>
#include <QSettings>
#include <QStatusBar>
#include <QStyle>
#include <QToolBar>
#include <QUrl>
#include "qqmainwindow.h"
#include "qqmenuhelper.h"

/**
 * Constructor.
 *
 * @param fileDescription The description of the document file type as it is
 *                        to appear in file dialogs
 * @param fileExtension The file extension of the document file type (do not
 *                      include the period)
 * @param parent This window's parent widget, if any (usually none)
 */
QQMainWindow::QQMainWindow(const QString &fileDescription,
                           const QString &fileExtension,
                           QWidget *parent)
  : QMainWindow(parent), extension(fileExtension), docPath(""), printer(0)
{
#if defined(Q_WS_MAEMO_5)
    setAttribute(Qt::WA_Maemo5StackedWindow);
#else
#if defined(Q_WS_MAC)
    // Set the icon used in the about dialog; doc icon is main window icon
    statusBar()->setWindowIcon(qApp->windowIcon());
#else
    statusBar();
#endif
#endif
    toolbar = addToolBar("toolbar");
    toolbar->setObjectName("toolbar");
    mh = new QQMenuHelper(this, toolbar, fileDescription, fileExtension);
}

/**
 * Destructor.
 */
QQMainWindow::~QQMainWindow()
{
    QSettings settings;
    mh->saveSettings(&settings);
    saveWindowSettings(&settings);
    if (printer) {
        delete printer;
    }
}

/**
 * Get the menu/toolbar helper object.
 *
 * @return The requested utility object
 */
QQMenuHelper *QQMainWindow::menuHelper()
{
    return mh;
}

/**
 * Restore the window position, size, etc. from the provided application
 * settings object.
 *
 * @param settings The application settings to load from
 */
void QQMainWindow::restoreWindowSettings(QSettings *settings)
{
    settings->beginGroup("Geometry");
    int xpos = settings->value("X", -1).toInt();
    int ypos = settings->value("Y", -1).toInt();
    if (xpos != -1 && ypos != -1) {
        move(xpos, ypos);
    }
    if (settings->value("Maximized").toBool()) {
        resize(600, 400);
        showMaximized();
    }
    else {
        int w = settings->value("Width", 600).toInt();
        int h = settings->value("Height", 400).toInt();
        resize(w, h);
    }
    if (settings->contains("State")) {
        restoreState(settings->value("State").toByteArray());
        // make sure the toolbar isn't and can't be hidden
        if (toolbar->isHidden()) {
            toolbar->show();
        }
        toolbar->toggleViewAction()->setEnabled(false);
        toolbar->toggleViewAction()->setVisible(false);
    }
    settings->endGroup();
}

/**
 * Save the window position, size, etc. to the provided application
 * settings object.
 *
 * @param settings The application settings to save to
 */
void QQMainWindow::saveWindowSettings(QSettings *settings)
{
    settings->beginGroup("Geometry");
    settings->setValue("Maximized", isMaximized());
    settings->setValue("X", x());
    settings->setValue("Y", y());
    settings->setValue("Width", width());
    settings->setValue("Height", height());
    settings->setValue("State", saveState());
    settings->endGroup();
}

/**
 * Switch from normal mode to fullscreen or vice versa.
 */
void QQMainWindow::toggleFullscreen()
{
    if (isFullScreen()) {
        showNormal();
    }
    else {
        showFullScreen();
    }
}

/**
 * Get the path of the currently open document.
 */
QString QQMainWindow::documentPath()
{
    return docPath;
}

/**
 * Set the path of the currently open document.
 *
 * @param path The path of the open document ("" if none)
 */
void QQMainWindow::setDocumentPath(const QString &path)
{
    docPath = path;
}

/**
 * Update the text in the window's title bar.
 */
void QQMainWindow::updateCaption()
{
    if (docPath.isEmpty()) {
        setWindowTitle(qApp->applicationName());
    }
    else {
        QString name = QFileInfo(docPath).fileName();
        setWindowTitle(name + "[*] - " + qApp->applicationName());
    }
}

/**
 * Drag event handler which determines if this application allows a
 * particular item to be dragged over it (in preparation for a drop,
 * usually).  Currently accepts any URL drag events, but nothing else (text,
 * images, etc.)  We'll figure out if it's actually an appropriate document
 * file or not when a drop is attempted.  Note that drags aren't accepted if
 * there is already an open file; it has to be closed first.
 *
 * @param event A drag event
 */
void QQMainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (docPath.isEmpty() && event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

/**
 * Drop event handler which opens a dropped document file if there isn't
 * one already open.
 *
 * @param event A drop event
 */
void QQMainWindow::dropEvent(QDropEvent *event)
{
    if (!docPath.isEmpty()) {
        return;
    }
    if (event->mimeData()->hasUrls()) {
        QUrl url = event->mimeData()->urls()[0];
        QString file = url.toLocalFile();
        bool valid = true;
        if (file.isEmpty()) {
            valid = false;
        }
        else if (file.length() < 5) {
            valid = false;
        }
        else if (file.right(4).toLower() != (QString(".") + extension)) {
            valid = false;
        }
        if (!valid) {
            QString appName = qApp->applicationName();
            QMessageBox::warning(this, appName,
                                 tr("Not a %1 file").arg(appName));
            return;
        }
        event->acceptProposedAction();
        openFile(file);
    }
}

/**
 * Main event handler, overridden to support application icon click and drag
 * events.  Only useful on Mac OS X, where it allows copying or linking the
 * currently open file (with a drag) or viewing the path to the current file
 * (with a command-click).  Cribbed almost verbatim from Qt Quarterly:
 * http://doc.trolltech.com/qq/qq18-macfeatures.html
 *
 * @param event The event to be processed
 * @return True if the event was recognized, false otherwise
 */
bool QQMainWindow::event(QEvent *event)
{
#if defined(Q_WS_MAC)
    if (!isActiveWindow() || docPath.isEmpty()) {
        return QMainWindow::event(event);
    }
    switch (event->type()) {
        case QEvent::IconDrag: {
            event->accept();
            Qt::KeyboardModifiers modifiers = qApp->keyboardModifiers();
            if (modifiers == Qt::NoModifier) {
                QDrag *drag = new QDrag(this);
                QMimeData *data = new QMimeData();
                data->setUrls(QList<QUrl>() << QUrl::fromLocalFile(docPath));
                drag->setMimeData(data);
                QPixmap cursorPixmap(":/icons/document_large.png");
                drag->setPixmap(cursorPixmap);
                QPoint hotspot(cursorPixmap.width() - 5, 5);
                drag->setHotSpot(hotspot);
                drag->start(Qt::LinkAction | Qt::CopyAction);
            }
            else if (modifiers == Qt::ControlModifier) {
                QMenu menu(this);
                connect(&menu, SIGNAL(triggered(QAction *)), this, SLOT(openAt(QAction *)));

                QFileInfo info(docPath);
                QAction *action = menu.addAction(info.fileName());
                action->setIcon(QIcon(":/icons/document_small.png"));

                QStringList folders = info.absolutePath().split('/');
                QStringListIterator it(folders);
                it.toBack();
                while (it.hasPrevious()) {
                    QString string = it.previous();
                    QIcon icon;
                    if (!string.isEmpty()) {
                        icon = style()->standardIcon(QStyle::SP_DirClosedIcon, 0, this);
                    }
                    else { // At the root
                        string = "/";
                        icon = style()->standardIcon(QStyle::SP_DriveHDIcon, 0, this);
                    }
                    action = menu.addAction(string);
                    action->setIcon(icon);
                }
                QPoint pos(QCursor::pos().x() - 20, frameGeometry().y());
                menu.exec(pos);
            }
            else {
                event->ignore();
            }
            return true;
        }
        default: {
            return QMainWindow::event(event);
        }
    }
#else
    return QMainWindow::event(event);
#endif
}

/**
 * Handler for parent directory selections from the application icon menu.
 * Only called on Mac OS X.
 *
 * @param action The application icon menu action that was triggered
 */
void QQMainWindow::openAt(QAction *action)
{
    QString path = docPath.left(docPath.indexOf(action->text())) + action->text();
    if (path == docPath) {
        return;
    }
    QProcess process;
    process.start("/usr/bin/open", QStringList() << path, QIODevice::ReadOnly);
    process.waitForFinished();
}

/**
 * Show a print preview dialog.  In addition to showing what the printed
 * output will look like, this dialog also lets the user set up printing
 * options such as zoom factor, orientation, and paper size.  Configured
 * settings are preserved until the application exits.
 */
void QQMainWindow::printPreview()
{
    if (!printer) {
        printer = new QPrinter(QPrinter::HighResolution);
    }
    else {
        printer->setPrintRange(QPrinter::AllPages);
        printer->setFromTo(0, 0);
    }
    QPrintPreviewDialog dialog(printer, this);
    connect(&dialog, SIGNAL(paintRequested(QPrinter*)),
            this, SLOT(print(QPrinter*)));
    dialog.exec();
}

/**
 * Print the current document (or appropriate portion thereof).
 */
void QQMainWindow::print()
{
    if (!printer) {
        printer = new QPrinter(QPrinter::HighResolution);
    }
    else {
        printer->setPrintRange(QPrinter::AllPages);
        printer->setFromTo(0, 0);
    }
    QPrintDialog dialog(printer, this);
    if (dialog.exec() != QDialog::Accepted) {
        statusBar()->showMessage(tr("Printing aborted"), 2000);
        return;
    }
    statusBar()->showMessage(tr("Printing") + "...");
    print(printer);
    statusBar()->showMessage(tr("Printing completed"), 2000);
}

/**
 * Window close event handler.  Makes sure that the user is given the
 * opportunity to save their changes if they forgot to do so.
 *
 * @param e The window close event
 */
void QQMainWindow::closeEvent(QCloseEvent *e)
{
    if (!docPath.isEmpty()) {
        int choice = mh->saveChangesPrompt();
        e->ignore();
        if (choice != QMessageBox::Cancel) {
            closeViewer();
        }
    }
    else {
        e->accept();
    }
}

/**
 * Quit the application.  Called when the "Quit" action is triggered.  Makes
 * sure that the user is given the opportunity to save their changes if they
 * forgot to do so.
 */
void QQMainWindow::quit()
{
    if (!docPath.isEmpty()) {
        // close the current file; might be cancelled
        close();
    }
    if (docPath.isEmpty()) {
        // if there isn't an open file, go ahead and exit
        close();
    }
}
