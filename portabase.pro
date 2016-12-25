TEMPLATE        = app
CONFIG         += qt warn_on thread
#DEFINES        += TRACE_ENABLED # enables TRACE macro for crash debugging
QT             += xml
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport
DESTDIR         = build
OBJECTS_DIR     = build
MOC_DIR         = build
RCC_DIR         = build
TRANSLATIONS    = resources/translations/portabase_cs.ts \
                  resources/translations/portabase_de.ts \
                  resources/translations/portabase_en.ts \
                  resources/translations/portabase_es.ts \
                  resources/translations/portabase_fr.ts \
                  resources/translations/portabase_ja.ts \
                  resources/translations/portabase_sk.ts \
                  resources/translations/portabase_zh_CN.ts \
                  resources/translations/portabase_zh_TW.ts

# Files common between all versions
HEADERS         = src/calc/calcdateeditor.h \
                  src/calc/calceditor.h \
                  src/calc/calcnode.h \
                  src/calc/calcnodeeditor.h \
                  src/calc/calctimeeditor.h \
                  src/calc/calcwidget.h \
                  src/calculator.h \
                  src/color_picker/qtcolorpicker.h \
                  src/columneditor.h \
                  src/columninfo.h \
                  src/commandline.h \
                  src/condition.h \
                  src/conditioneditor.h \
                  src/csverror.h \
                  src/csvutils.h \
                  src/database.h \
                  src/datamodel.h \
                  src/datatypes.h \
                  src/datedialog.h \
                  src/datewidget.h \
                  src/dbeditor.h \
                  src/dynamicedit.h \
                  src/encryption/blowfish.h \
                  src/encryption/bytestream.h \
                  src/encryption/crypto.h \
                  src/encryption/randomkit/randomkit.h \
                  src/encryption/randomkit/rk_isaac.h \
                  src/encryption/randomkit/rk_mt.h \
                  src/enumeditor.h \
                  src/enummanager.h \
                  src/eventfilter.h \
                  src/factory.h \
                  src/filter.h \
                  src/filtereditor.h \
                  src/formatting.h \
                  src/image/imageeditor.h \
                  src/image/imageselector.h \
                  src/image/imageutils.h \
                  src/image/imageviewer.h \
                  src/image/imagewidget.h \
                  src/image/slideshowdialog.h \
                  src/importdialog.h \
                  src/importutils.h \
                  src/menuactions.h \
                  src/metakitfuncs.h \
                  src/mobiledb.h \
                  src/notebutton.h \
                  src/noteeditor.h \
                  src/numberwidget.h \
                  src/oldconfig.h \
                  src/passdialog.h \
                  src/pbdialog.h \
                  src/pbmaemo5style.h \
                  src/pdbfile.h \
                  src/portabase.h \
                  src/preferences.h \
                  src/propertiesdialog.h \
                  src/qqutil/qqdialog.h \
                  src/qqutil/qqmainwindow.h \
                  src/qqutil/qqmenuhelper.h \
                  src/qqutil/qqtoolbar.h \
                  src/qqutil/qqtoolbarstretch.h \
                  src/roweditor.h \
                  src/rowviewer.h \
                  src/sorteditor.h \
                  src/timewidget.h \
                  src/view.h \
                  src/viewdisplay.h \
                  src/vieweditor.h \
                  src/vsfmanager.h \
                  src/xmlexport.h \
                  src/xmlimport.h
SOURCES         = src/calc/calcdateeditor.cpp \
                  src/calc/calceditor.cpp \
                  src/calc/calcnode.cpp \
                  src/calc/calcnodeeditor.cpp \
                  src/calc/calctimeeditor.cpp \
                  src/calc/calcwidget.cpp \
                  src/calculator.cpp \
                  src/color_picker/qtcolorpicker.cpp \
                  src/columneditor.cpp \
                  src/columninfo.cpp \
                  src/commandline.cpp \
                  src/condition.cpp \
                  src/conditioneditor.cpp \
                  src/csverror.cpp \
                  src/csvutils.cpp \
                  src/database.cpp \
                  src/datamodel.cpp \
                  src/datedialog.cpp \
                  src/datewidget.cpp \
                  src/dbeditor.cpp \
                  src/dynamicedit.cpp \
                  src/encryption/blowfish.cpp \
                  src/encryption/bytestream.cpp \
                  src/encryption/crypto.cpp \
                  src/encryption/randomkit/rk_isaac.c \
                  src/encryption/randomkit/rk_mt.c \
                  src/enumeditor.cpp \
                  src/enummanager.cpp \
                  src/eventfilter.cpp \
                  src/factory.cpp \
                  src/filter.cpp \
                  src/filtereditor.cpp \
                  src/formatting.cpp \
                  src/image/imageeditor.cpp \
                  src/image/imageselector.cpp \
                  src/image/imageutils.cpp \
                  src/image/imageviewer.cpp \
                  src/image/imagewidget.cpp \
                  src/image/slideshowdialog.cpp \
                  src/importdialog.cpp \
                  src/importutils.cpp \
                  src/main.cpp \
                  src/menuactions.cpp \
                  src/metakitfuncs.cpp \
                  src/mobiledb.cpp \
                  src/notebutton.cpp \
                  src/noteeditor.cpp \
                  src/numberwidget.cpp \
                  src/oldconfig.cpp \
                  src/passdialog.cpp \
                  src/pbdialog.cpp \
                  src/pbmaemo5style.cpp \
                  src/pdbfile.cpp \
                  src/portabase.cpp \
                  src/preferences.cpp \
                  src/propertiesdialog.cpp \
                  src/qqutil/qqdialog.cpp \
                  src/qqutil/qqmainwindow.cpp \
                  src/qqutil/qqmenuhelper.cpp \
                  src/qqutil/qqtoolbar.cpp \
                  src/qqutil/qqtoolbarstretch.cpp \
                  src/roweditor.cpp \
                  src/rowviewer.cpp \
                  src/sorteditor.cpp \
                  src/timewidget.cpp \
                  src/view.cpp \
                  src/viewdisplay.cpp \
                  src/vieweditor.cpp \
                  src/vsfmanager.cpp \
                  src/xmlexport.cpp \
                  src/xmlimport.cpp

# Stuff for all Linux/UNIX versions
unix {
    LIBS               += -lm -lmk4
    VERSION             = $$system(cat packaging/version_number)
    COPYRIGHT_YEARS     = $$system(cat packaging/copyright_years)
}

# Stuff for Mac OS X
macx {
    CONFIG             += c++11 release x86_64
    QMAKE_CXXFLAGS     += -stdlib=libc++ -std=c++11
    QMAKE_MAC_SDK       = macosx10.11
    QT                 += macextras
    INCLUDEPATH        += /usr/local/include
    LIBS               += -L/usr/local/lib -framework Foundation
    TARGET              = PortaBase
    HEADERS            += src/qqutil/qqmactoolbardelegate.h \
                          src/qqutil/qqmactoolbarutils.h
    OBJECTIVE_SOURCES  += src/qqutil/qqmactoolbardelegate.mm \
                          src/qqutil/qqmactoolbarutils.mm
    RESOURCES           = resources/mac.qrc
    ICON                = packaging/mac/PortaBase.icns
    DOCUMENT_ICON.files = packaging/mac/PortaBaseFile.icns
    DOCUMENT_ICON.path  = Contents/Resources
    CS_LPROJ.files      = packaging/mac/cs.lproj/InfoPlist.strings \
                          packaging/mac/cs.lproj/locversion.plist
    CS_LPROJ.path       = Contents/Resources/cs.lproj
    DE_LPROJ.files      = packaging/mac/de.lproj/InfoPlist.strings \
                          packaging/mac/de.lproj/locversion.plist
    DE_LPROJ.path       = Contents/Resources/de.lproj
    EN_LPROJ.files      = packaging/mac/en.lproj/InfoPlist.strings \
                          packaging/mac/en.lproj/locversion.plist
    EN_LPROJ.path       = Contents/Resources/en.lproj
    ES_LPROJ.files      = packaging/mac/es.lproj/InfoPlist.strings \
                          packaging/mac/es.lproj/locversion.plist
    ES_LPROJ.path       = Contents/Resources/es.lproj
    FR_LPROJ.files      = packaging/mac/fr.lproj/InfoPlist.strings \
                          packaging/mac/fr.lproj/locversion.plist
    FR_LPROJ.path       = Contents/Resources/fr.lproj
    JA_LPROJ.files      = packaging/mac/ja.lproj/InfoPlist.strings \
                          packaging/mac/ja.lproj/locversion.plist
    JA_LPROJ.path       = Contents/Resources/ja.lproj
    SK_LPROJ.files      = packaging/mac/sk.lproj/InfoPlist.strings \
                          packaging/mac/sk.lproj/locversion.plist
    SK_LPROJ.path       = Contents/Resources/sk.lproj
    ZH_HANS_LPROJ.files = packaging/mac/zh-Hans.lproj/InfoPlist.strings \
                          packaging/mac/zh-Hans.lproj/locversion.plist
    ZH_HANS_LPROJ.path  = Contents/Resources/zh-Hans.lproj
    ZH_HANT_LPROJ.files = packaging/mac/zh-Hant.lproj/InfoPlist.strings \
                          packaging/mac/zh-Hant.lproj/locversion.plist
    ZH_HANT_LPROJ.path  = Contents/Resources/zh-Hant.lproj
    QMAKE_BUNDLE_DATA  += DOCUMENT_ICON \
                          CS_LPROJ \
                          DE_LPROJ \
                          EN_LPROJ \
                          ES_LPROJ \
                          FR_LPROJ \
                          JA_LPROJ \
                          SK_LPROJ \
                          ZH_HANS_LPROJ \
                          ZH_HANT_LPROJ
    QMAKE_INFO_PLIST    = packaging/mac/Info.plist
}

# Stuff for Maemo
maemo5|contains(QT_CONFIG, hildon) {
    CONFIG             += debug qdbus
    LIBS               += -L../../src/metakit/builds
    isEmpty(PREFIX) {
        PREFIX          = /usr/local
    }
    DATADIR             = $$PREFIX/share
    DEFINES            += DATADIR=\\\"$$DATADIR\\\" PKGDATADIR=\\\"$$PKGDATADIR\\\"
    TARGET              = portabase
    INSTALLS           += target desktop service icon16 icon22 icon32 icon48 icon64 mime
    desktop.path        = $$DATADIR/applications/hildon
    desktop.files       = packaging/maemo/portabase.desktop
    service.path        = $$DATADIR/dbus-1/services
    service.files       = packaging/maemo/net.sourceforge.portabase.service
    icon16.path         = $$DATADIR/icons/hicolor/16x16/apps
    icon16.files        = resources/icons/linux/16x16/portabase.png
    icon22.path         = $$DATADIR/icons/hicolor/22x22/apps
    icon22.files        = resources/icons/linux/22x22/portabase.png
    icon32.path         = $$DATADIR/icons/hicolor/32x32/apps
    icon32.files        = resources/icons/linux/32x32/portabase.png
    icon48.path         = $$DATADIR/icons/hicolor/48x48/apps
    icon48.files        = resources/icons/linux/48x48/portabase.png
    icon64.path         = $$DATADIR/icons/hicolor/64x64/apps
    icon64.files        = resources/icons/linux/64x64/portabase.png
    mime.path           = $$DATADIR/mime/packages
    mime.files          = packaging/maemo/portabase-mime.xml
}
contains(QT_CONFIG, hildon):!maemo5 {
    target.path         = $$PREFIX/bin
    RESOURCES           = resources/diablo.qrc
}
maemo5 {
    QT                 += dbus maemo5 network webkit
    target.path         = /opt/maemo
    RESOURCES           = resources/fremantle.qrc
    HEADERS            += src/image/imagereply.h \
                          src/pbnetworkaccessmanager.h
    SOURCES            += src/image/imagereply.cpp \
                          src/pbnetworkaccessmanager.cpp
}

# Stuff for other Linux/UNIX platforms
unix:!macx:!maemo5:!contains(QT_CONFIG, hildon) {
    #QMAKE_CXXFLAGS       += -O0 # for valgrind
    CONFIG               += debug
    LIBS                 += -Lmetakit/builds
    RESOURCES             = resources/linux.qrc
}

# Stuff for Windows
win32 {
    CONFIG                 += release
    TARGET                  = PortaBase
    RESOURCES               = resources/windows.qrc
    LIBS                   += c:/portabase/metakit/builds/libmk4.a
    RC_FILE                 = portabase.rc
    INCLUDEPATH            += c:/portabase/metakit/include
    VERSION                 = $$system(type packaging\\version_number)
    COPYRIGHT_YEARS         = $$system(type packaging\\copyright_years)
}

# Stuff for static builds
static {
    QTPLUGIN           += qtaccessiblewidgets qjpeg
    DEFINES            += STATIC_QT
    QMAKE_CXXFLAGS     += -fvisibility=hidden
}

# Provide the version number and copyright year range
VERSION_STRING         = '\\"$${VERSION}\\"'
COPYRIGHT_YEARS_STRING = '\\"$${COPYRIGHT_YEARS}\\"'
DEFINES               += VERSION_NUMBER=\"$${VERSION_STRING}\"
DEFINES               += COPYRIGHT_YEARS=\"$${COPYRIGHT_YEARS_STRING}\"
