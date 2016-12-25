/*
 * qqmainwindow.h
 *
 * (c) 2010,2016 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file qqutil/qqmainwindow.h
 * Header file for QQMainWindow
 */

#ifndef QQMAINWINDOW_H
#define QQMAINWINDOW_H

#include <QMainWindow>

class QPrinter;
class QQMenuHelper;
class QSettings;

/**
 * The base class for the main window of a document-based application
 * with a single file format.  Provides support for features typically
 * expected of such applications across different platforms.  Assumes the
 * existence of the following icon resources on Mac OS X:
 *
 * @li icons/document_small.png (should be 16x16)
 * @li icons/document_large.png (should be 32x32)
 *
 * These are different sizes of the icon used to represent the
 * application's documents in Finder.
 */
class QQMainWindow: public QMainWindow
{
    Q_OBJECT
public:
    QQMainWindow(const QString &fileDescription,
                 const QString &fileExtension,
                 QWidget *parent = 0);
    virtual ~QQMainWindow();

public slots:
    /**
     * Open the document file which is at the specified location.  To be
     * implemented by subclasses.
     *
     * @param path The path of the file to be opened
     */
    virtual void openFile(const QString &path) = 0;

protected slots:
    void printPreview();
    void print();
    /**
     * Do the work of actually printing the file, after the page layout has
     * been set up.  To be implemented by subclasses (but may do nothing if
     * printing is not supported).
     *
     * @param p The printer object to use
     */
    virtual void print(QPrinter *p) = 0;
    void quit();
    void toggleFullscreen();

protected:
    void closeEvent(QCloseEvent *e);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    bool event (QEvent *event);
    QString documentPath();
    void setDocumentPath(const QString &path);
    QQMenuHelper *menuHelper();
    void updateCaption();
    void restoreWindowSettings(QSettings *settings);
    /**
     * Close any open document and enter "select a file" mode.  To be
     * implemented by subclasses.
     */
    virtual void closeViewer() = 0;

private slots:
    void openAt(QAction *where);

private:
    void saveWindowSettings(QSettings *settings);

private:
    QString extension; /**< The extension of the document file type */
    QQMenuHelper *mh; /**< Support code for any document-based application */
    QString docPath; /**< Path of the current document file */
    QPrinter *printer; /**< The last-used printer settings (for this application instance) */
};

#endif
