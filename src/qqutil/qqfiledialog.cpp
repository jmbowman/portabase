/*
 * qqfiledialog.cpp
 *
 * (c) 2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file qqutil/qqfiledialog.cpp
 * Source file for QQFileDialog
 */

#include "qqfiledialog.h"

#if defined(Q_OS_ANDROID)
#include <QtAndroidExtras>
#include <QApplication>
#include <QComboBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListView>
#include <QScreen>
#include <QSplitter>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QToolButton>
#include <QUrl>
#include "qqfactory.h"

/**
 * Constructor.
 *
 * @param parent The file dialog's parent widget
 * @param caption The caption in the file dialog's title bar. If not
 *                specified, a default caption will be used
 * @param dir The file dialog's working directory
 * @param filter A restriction on which types of files are shown
 */
QQFileDialog::QQFileDialog(QWidget *parent, const QString &caption,
                           const QString &dir, const QString &filter)
    : QFileDialog(parent, caption, dir, filter), pressing(false)
{
    QString fileNameText = tr("Name") + ":";
    QString fileTypeText = tr("Type") + ":";
    QString internalStorageText = tr("Internal storage");
    QString sdCardText = tr("SD card");
    int buttonPixels = QQFactory::dpToPixels(48);

    // Use better label text
    QLabel *fileNameLabel = findChild<QLabel*>(QStringLiteral("fileNameLabel"));
    fileNameLabel->setText(fileNameText);
    QLabel *fileTypeLabel = findChild<QLabel*>(QStringLiteral("fileTypeLabel"));
    fileTypeLabel->setText(fileTypeText);

    // Make the combo menu item heights finger-friendly
    QComboBox *lookInCombo = findChild<QComboBox*>(QStringLiteral("lookInCombo"));
    lookInCombo->setStyleSheet(QString("QComboBox QAbstractItemView::item {min-height: %1px}").arg(buttonPixels));
    lookInCombo->setView(new QListView(lookInCombo));

    // Hide the back, forward, and new folder buttons to free up some space
    QToolButton *backButton = findChild<QToolButton*>(QStringLiteral("backButton"));
    backButton->hide();
    QToolButton *forwardButton = findChild<QToolButton*>(QStringLiteral("forwardButton"));
    forwardButton->hide();
    QToolButton *newFolderButton = findChild<QToolButton*>(QStringLiteral("newFolderButton"));
    newFolderButton->hide();

    // Hide the display mode buttons, detail mode is inappropriate for small screens
    setViewMode(QFileDialog::List);
    QToolButton *listModeButton = findChild<QToolButton*>(QStringLiteral("listModeButton"));
    listModeButton->hide();
    QToolButton *detailModeButton = findChild<QToolButton*>(QStringLiteral("detailModeButton"));
    detailModeButton->hide();

    // Use better icons for the remaining buttons
    QToolButton *toParentButton = findChild<QToolButton*>(QStringLiteral("toParentButton"));
    toParentButton->setMinimumWidth(buttonPixels);
    toParentButton->setMinimumHeight(buttonPixels);

    // Align the buttons vertically within the hbox layout
    QGridLayout *grid = qobject_cast<QGridLayout*>(layout());
    QHBoxLayout *hbox = qobject_cast<QHBoxLayout*>(grid->itemAtPosition(0, 1)->layout());
    hbox->setAlignment(toParentButton, Qt::AlignCenter);
    hbox->setAlignment(newFolderButton, Qt::AlignCenter);

    // Make the sidebar wide enough to read its entries
    QSplitter *splitter = findChild<QSplitter*>(QStringLiteral("splitter"));
    QList<int> sizes;
    int availableWidth = qApp->primaryScreen()->availableSize().width();
    int sidebarWidth = QQFactory::dpToPixels(170);
    sizes.append(sidebarWidth);
    sizes.append(availableWidth - sidebarWidth);
    splitter->setSizes(sizes);

    // Enable kinetic scrolling for the list views
    QListView *sidebar = findChild<QListView*>(QStringLiteral("sidebar"));
    sidebar->setAttribute(Qt::WA_AcceptTouchEvents);
    sidebar->setContextMenuPolicy(Qt::NoContextMenu);
    sidebar->setDragDropMode(QAbstractItemView::NoDragDrop);
    QQFactory::configureAbstractItemView(sidebar);
    QListView *listView = findChild<QListView*>(QStringLiteral("listView"));
    listView->setContextMenuPolicy(Qt::NoContextMenu);
    listView->setDragDropMode(QAbstractItemView::NoDragDrop);
    listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    listView->setResizeMode(QListView::Fixed);
    listView->setWrapping(false);
    QQFactory::configureAbstractItemView(listView);

    // Use appropriate font and icon sizes in the list views
    QString styleSheet(QStringLiteral("QListView {font-size: 16pt;}"));
    sidebar->setStyleSheet(styleSheet);
    int iconPixels = QQFactory::dpToPixels(24);
    QSize iconSize(iconPixels, iconPixels);
    sidebar->setIconSize(iconSize);
    listView->setStyleSheet(styleSheet);

    // Use appropriate locations in the sidebar
    QStandardItemModel *sidebarModel = new QStandardItemModel(sidebar);
    int UrlRole = Qt::UserRole + 1;
    QStandardItem *internalItem = new QStandardItem(QQFactory::icon(QStringLiteral("computer")),
                                                    internalStorageText);
    internalItem->setData(QUrl::fromLocalFile(defaultStoragePath()), UrlRole);
    sidebarModel->appendRow(internalItem);
    QString sdPath = sdCardPath();
    if (!sdPath.isEmpty()) {
        QStandardItem *sdItem = new QStandardItem(QQFactory::icon(QStringLiteral("drive")),
                                                  sdCardText);
        sdItem->setData(QUrl::fromLocalFile(sdPath), UrlRole);
        sidebarModel->appendRow(sdItem);
    }
    sidebar->setModel(sidebarModel);
    connect(sidebar->selectionModel(),
            SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            sidebar, SLOT(clicked(QModelIndex)));

    // Don't require double-clicking in the list view
    connect(listView, &QListView::pressed, this, &QQFileDialog::itemPressed);
    connect(listView, &QListView::clicked, this, &QQFileDialog::itemReleased);
    connect(QScroller::scroller(listView), &QScroller::stateChanged,
            this, &QQFileDialog::scrollerStateChanged);
}

/**
 * This is a convenience static function that returns an existing file
 * selected by the user. If the user presses Cancel, it returns a null string.
 *
 * @param parent The file dialog's parent widget
 * @param caption The caption in the file dialog's title bar. If not
 *                specified, a default caption will be used
 * @param dir The file dialog's working directory
 * @param filter A restriction on which types of files are shown
 * @return The path to the selected file
 */
QString QQFileDialog::getOpenFileName(QWidget *parent, const QString &caption,
    const QString &dir, const QString &filter)
{
    QQFileDialog *fileDialog = new QQFileDialog(parent, caption, dir, filter);
    fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog->showMaximized();
    QString filename;
    if (fileDialog->exec()) {
        filename = fileDialog->selectedFiles()[0];
    }
    return filename;
}

/**
 * This is a convenience static function that will return a file name selected
 * by the user. The file does not have to exist.
 *
 * @param parent The file dialog's parent widget
 * @param caption The caption in the file dialog's title bar. If not
 *                specified, a default caption will be used
 * @param dir The file dialog's working directory
 * @param filter A restriction on which types of files are shown
 * @return The path to the chosen file location
 */
QString QQFileDialog::getSaveFileName(QWidget *parent, const QString &caption,
    const QString &dir, const QString &filter)
{
    QQFileDialog *fileDialog = new QQFileDialog(parent, caption, dir, filter);
    fileDialog->setAcceptMode(QFileDialog::AcceptSave);
    fileDialog->showMaximized();
    QString filename;
    if (fileDialog->exec()) {
        filename = fileDialog->selectedFiles()[0];
    }
    return filename;
}

/**
 * Get the absolute path of the default PortaBase file storage location.
 * On Android, this is the shared storage directory in the device's
 * internal storage.  Elewhere, it's the default document storage
 * directory.
 *
 * @return The default PortaBase file storage directory path
 */
QString QQFileDialog::defaultStoragePath()
{
#if defined(Q_OS_ANDROID)
    return QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation)[0];
#elif QT_VERSION >= 0x050000
    return QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)[0];
#else
    return QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
#endif
}

/**
 * Get the absolute path of an Android device's removable SD card's root
 * directory.  If there is no such card, returns a null string.
 *
 * @return The SD card's root path or a null string
 */
QString QQFileDialog::sdCardPath()
{
#if defined(Q_OS_ANDROID)
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
               "org/qtproject/qt5/android/QtNative",
               "activity", "()Landroid/app/Activity;");
    QAndroidJniObject dirs = activity.callObjectMethod(
                "getExternalFilesDirs", "(Ljava/lang/String;)[Ljava/io/File;", NULL);
    if (dirs.isValid()) {
       QAndroidJniEnvironment env;
       jsize length = env->GetArrayLength(dirs.object<jarray>());
       if (length > 1) {
          QAndroidJniObject path = env->GetObjectArrayElement(dirs.object<jobjectArray>(), 1);
          // Move up from Android/data/org.portabase.android/files
          QDir dir(path.toString());
          bool ok = dir.cd("../../../..");
          if (ok) {
              return dir.absolutePath();
          }
       }
    }
#endif
    return QString();
}

/**
 * Handler for presses on items in the file/directory listing.  Used to help
 * help determine when to treat a release as an item selection instead of
 * part of a scrolling operation.
 *
 * @param index The item which was pressed
 */
void QQFileDialog::itemPressed(const QModelIndex &index)
{
    Q_UNUSED(index)
    pressing = true;
}

/**
 * Mouse/touch release handler for items in the file/directory listing.  Used
 * to enter directories or select files.
 *
 * @param index The item which was released
 */
void QQFileDialog::itemReleased(const QModelIndex &index)
{
    if (pressing) {
        QListView *listView = findChild<QListView*>(QStringLiteral("listView"));
        emit listView->activated(index);
    }
}

/**
 * Handler for changes in scrolling status for the file/directory listing.
 * Ensures that lifting your finger at the end of a scroll operation doesn't
 * open the item that was touched to start scrolling.
 *
 * @param newState The new state of the data grid's scroller
 */
void QQFileDialog::scrollerStateChanged(QScroller::State newState)
{
    if (newState == QScroller::Dragging || newState == QScroller::Scrolling) {
        pressing = false;
    }
}

#endif
