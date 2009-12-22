/*
 * main.cpp
 *
 * (c) 2002-2004,2008-2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
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
#include <QTranslator>
#include "commandline.h"
#include "eventfilter.h"
#include "factory.h"
#include "portabase.h"

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
    QTranslator translator;
    if (translator.load(QString(":/i18n/PortaBase.qm"))) {
        app.installTranslator(&translator);
    }
    QStringList args = app.arguments();
    if (args.count() > 1 && args[1] != "-f") {
        CommandLine commandLine;
        return commandLine.process();
    }
    else {
        Factory::createCheckBoxIcons();
        PortaBase pb;
        pb.setWindowTitle("PortaBase");
        EventFilter ef(&pb);
        app.installEventFilter(&ef);
        pb.show();
        if (args.count() == 3 && args[1] == "-f") {
            pb.openFile(args[2]);
        }
        return app.exec();
    }
}
