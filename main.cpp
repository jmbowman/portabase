/*
 * main.cpp
 *
 * (c) 2002-2004,2008-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file main.cpp
 * Source file for the PortaBase main() method
 */

#include <QApplication>
#include <QIcon>
#include <QProcess>
#include <QRegExp>
#include <QTranslator>
#include "commandline.h"
#include "eventfilter.h"
#include "factory.h"
#include "portabase.h"

#if defined(Q_WS_HILDON) || defined(Q_WS_MAEMO_5)
#include <QDBusConnection>
#include <QDBusError>
#endif

/**
 * @mainpage PortaBase Source Code Reference
 *
 * This is the reference documentation for the source code of %PortaBase and
 * the software libraries it uses.  For more information on what %PortaBase
 * is and how to use it, see the %PortaBase home page at
 * http://portabase.sourceforge.net/.
 */

/**
 * The application main method, which receives the command line parameters
 * and starts the application accordingly.
 *
 * @param argc The number of command line parameters received
 * @param argv The content of the command line parameters
 * @return The return value of the application process; typically 0 for
 *         normal completion, 1 if some sort of error occurred
 */
int main(int argc, char **argv) {
    QApplication app(argc, argv);
    app.setOrganizationName("portabase");
    app.setOrganizationDomain("sourceforge.net");
    app.setApplicationName("PortaBase");
    app.setWindowIcon(QIcon(":/appicon/PortaBase.png"));
    // can't use QProcessEnvironment because Diablo doesn't have it yet
    QStringList env = QProcess::systemEnvironment();
    int qmIndex = env.indexOf(QRegExp("PORTABASE_QM=.*"));
    int qtQmIndex = env.indexOf(QRegExp("PORTABASE_QT_QM=.*"));
    QTranslator qtTranslator;
    if (qtQmIndex != -1) {
        QString path = env[qtQmIndex];
        path = path.right(path.length() - 16);
        if (qtTranslator.load(path)) {
            app.installTranslator(&qtTranslator);
        }
    }
    else {
        if (qtTranslator.load(QString(":/i18n/Qt.qm"))) {
            app.installTranslator(&qtTranslator);
        }
    }
    QTranslator translator;
    if (qmIndex != -1) {
        QString path = env[qmIndex];
        path = path.right(path.length() - 13);
        if (translator.load(path)) {
            app.installTranslator(&translator);
        }
    }
    else {
        if (translator.load(QString(":/i18n/PortaBase.qm"))) {
            app.installTranslator(&translator);
        }
    }
    QStringList args = app.arguments();
    if ((args.count() > 1 && args[1].startsWith("-")) || args.count() > 2) {
        CommandLine commandLine;
        return commandLine.process();
    }
    else {
        PortaBase pb;
        pb.setWindowTitle("PortaBase");
        EventFilter ef(&pb);
        app.installEventFilter(&ef);
#if defined(Q_WS_HILDON) || defined(Q_WS_MAEMO_5)
        QDBusConnection bus = QDBusConnection::sessionBus();
        if (!bus.isConnected()) {
            qWarning("Cannot connect to the D-Bus session bus.");
            exit(1);
        }
        if (!bus.registerService("net.sourceforge.portabase")) {
            qWarning("%s", qPrintable(bus.lastError().message()));
            exit(2);
        }
        if (!bus.registerObject("/net/sourceforge/portabase", &pb,
                                 QDBusConnection::ExportScriptableSlots)) {
            qWarning("%s", qPrintable(bus.lastError().message()));
            exit(3);
        }
#endif
        pb.show();
        if (args.count() == 2) {
            pb.openFile(args[1]);
        }
        return app.exec();
    }
}
