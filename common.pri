# Shared sources/settings for both portabase.pro and tests/dbeditor/dbeditor_test.pro.

CONFIG         += qt warn_on
#DEFINES        += TRACE_ENABLED # enables TRACE macro for crash debugging
QT             += xml
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport
DESTDIR         = build
OBJECTS_DIR     = build
MOC_DIR         = build
RCC_DIR         = build
TRANSLATIONS    = $$PWD/resources/translations/portabase_cs.ts \
                  $$PWD/resources/translations/portabase_de.ts \
                  $$PWD/resources/translations/portabase_en.ts \
                  $$PWD/resources/translations/portabase_es.ts \
                  $$PWD/resources/translations/portabase_fr.ts \
                  $$PWD/resources/translations/portabase_it.ts \
                  $$PWD/resources/translations/portabase_ja.ts \
                  $$PWD/resources/translations/portabase_pl.ts \
                  $$PWD/resources/translations/portabase_sk.ts \
                  $$PWD/resources/translations/portabase_zh_CN.ts \
                  $$PWD/resources/translations/portabase_zh_TW.ts

# Files common between all versions
HEADERS         = $$PWD/src/calc/calcdateeditor.h \
                  $$PWD/src/calc/calceditor.h \
                  $$PWD/src/calc/calcnode.h \
                  $$PWD/src/calc/calcnodeeditor.h \
                  $$PWD/src/calc/calctimeeditor.h \
                  $$PWD/src/calc/calcwidget.h \
                  $$PWD/src/calculator.h \
                  $$PWD/src/color_picker/qtcolorpicker.h \
                  $$PWD/src/columneditor.h \
                  $$PWD/src/columninfo.h \
                  $$PWD/src/commandline.h \
                  $$PWD/src/condition.h \
                  $$PWD/src/conditioneditor.h \
                  $$PWD/src/csverror.h \
                  $$PWD/src/csvutils.h \
                  $$PWD/src/database.h \
                  $$PWD/src/datamodel.h \
                  $$PWD/src/datatypes.h \
                  $$PWD/src/datedialog.h \
                  $$PWD/src/datewidget.h \
                  $$PWD/src/dbeditor.h \
                  $$PWD/src/dynamicedit.h \
                  $$PWD/src/encryption/blowfish.h \
                  $$PWD/src/encryption/bytestream.h \
                  $$PWD/src/encryption/crypto.h \
                  $$PWD/src/encryption/randomkit/randomkit.h \
                  $$PWD/src/encryption/randomkit/rk_isaac.h \
                  $$PWD/src/encryption/randomkit/rk_mt.h \
                  $$PWD/src/enumeditor.h \
                  $$PWD/src/enummanager.h \
                  $$PWD/src/eventfilter.h \
                  $$PWD/src/factory.h \
                  $$PWD/src/filter.h \
                  $$PWD/src/filtereditor.h \
                  $$PWD/src/formatting.h \
                  $$PWD/src/image/imageeditor.h \
                  $$PWD/src/image/imageselector.h \
                  $$PWD/src/image/imageutils.h \
                  $$PWD/src/image/imageviewer.h \
                  $$PWD/src/image/imagewidget.h \
                  $$PWD/src/image/slideshowdialog.h \
                  $$PWD/src/importdialog.h \
                  $$PWD/src/importutils.h \
                  $$PWD/src/menuactions.h \
                  $$PWD/src/metakitfuncs.h \
                  $$PWD/src/mobiledb.h \
                  $$PWD/src/notebutton.h \
                  $$PWD/src/noteeditor.h \
                  $$PWD/src/numberwidget.h \
                  $$PWD/src/oldconfig.h \
                  $$PWD/src/passdialog.h \
                  $$PWD/src/pbdialog.h \
                  $$PWD/src/pbinputdialog.h \
                  $$PWD/src/pbmaemo5style.h \
                  $$PWD/src/pdbfile.h \
                  $$PWD/src/portabase.h \
                  $$PWD/src/preferences.h \
                  $$PWD/src/propertiesdialog.h \
                  $$PWD/src/qqutil/qqdialog.h \
                  $$PWD/src/qqutil/qqfactory.h \
                  $$PWD/src/qqutil/qqfiledialog.h \
                  $$PWD/src/qqutil/qqlineedit.h \
                  $$PWD/src/qqutil/qqmainwindow.h \
                  $$PWD/src/qqutil/qqmenuhelper.h \
                  $$PWD/src/qqutil/qqspinbox.h \
                  $$PWD/src/qqutil/qqtoolbar.h \
                  $$PWD/src/qqutil/qqtoolbarstretch.h \
                  $$PWD/src/qqutil/qqtreeview.h \
                  $$PWD/src/roweditor.h \
                  $$PWD/src/rowviewer.h \
                  $$PWD/src/sorteditor.h \
                  $$PWD/src/timewidget.h \
                  $$PWD/src/view.h \
                  $$PWD/src/viewdisplay.h \
                  $$PWD/src/vieweditor.h \
                  $$PWD/src/vsfmanager.h \
                  $$PWD/src/xmlexport.h \
                  $$PWD/src/xmlimport.h
SOURCES         = $$PWD/src/calc/calcdateeditor.cpp \
                  $$PWD/src/calc/calceditor.cpp \
                  $$PWD/src/calc/calcnode.cpp \
                  $$PWD/src/calc/calcnodeeditor.cpp \
                  $$PWD/src/calc/calctimeeditor.cpp \
                  $$PWD/src/calc/calcwidget.cpp \
                  $$PWD/src/calculator.cpp \
                  $$PWD/src/color_picker/qtcolorpicker.cpp \
                  $$PWD/src/columneditor.cpp \
                  $$PWD/src/columninfo.cpp \
                  $$PWD/src/commandline.cpp \
                  $$PWD/src/condition.cpp \
                  $$PWD/src/conditioneditor.cpp \
                  $$PWD/src/csverror.cpp \
                  $$PWD/src/csvutils.cpp \
                  $$PWD/src/database.cpp \
                  $$PWD/src/datamodel.cpp \
                  $$PWD/src/datedialog.cpp \
                  $$PWD/src/datewidget.cpp \
                  $$PWD/src/dbeditor.cpp \
                  $$PWD/src/dynamicedit.cpp \
                  $$PWD/src/encryption/blowfish.cpp \
                  $$PWD/src/encryption/bytestream.cpp \
                  $$PWD/src/encryption/crypto.cpp \
                  $$PWD/src/encryption/randomkit/rk_isaac.c \
                  $$PWD/src/encryption/randomkit/rk_mt.c \
                  $$PWD/src/enumeditor.cpp \
                  $$PWD/src/enummanager.cpp \
                  $$PWD/src/eventfilter.cpp \
                  $$PWD/src/factory.cpp \
                  $$PWD/src/filter.cpp \
                  $$PWD/src/filtereditor.cpp \
                  $$PWD/src/formatting.cpp \
                  $$PWD/src/image/imageeditor.cpp \
                  $$PWD/src/image/imageselector.cpp \
                  $$PWD/src/image/imageutils.cpp \
                  $$PWD/src/image/imageviewer.cpp \
                  $$PWD/src/image/imagewidget.cpp \
                  $$PWD/src/image/slideshowdialog.cpp \
                  $$PWD/src/importdialog.cpp \
                  $$PWD/src/importutils.cpp \
                  $$PWD/src/menuactions.cpp \
                  $$PWD/src/metakitfuncs.cpp \
                  $$PWD/src/mobiledb.cpp \
                  $$PWD/src/notebutton.cpp \
                  $$PWD/src/noteeditor.cpp \
                  $$PWD/src/numberwidget.cpp \
                  $$PWD/src/oldconfig.cpp \
                  $$PWD/src/passdialog.cpp \
                  $$PWD/src/pbdialog.cpp \
                  $$PWD/src/pbinputdialog.cpp \
                  $$PWD/src/pbmaemo5style.cpp \
                  $$PWD/src/pdbfile.cpp \
                  $$PWD/src/portabase.cpp \
                  $$PWD/src/preferences.cpp \
                  $$PWD/src/propertiesdialog.cpp \
                  $$PWD/src/qqutil/qqdialog.cpp \
                  $$PWD/src/qqutil/qqfactory.cpp \
                  $$PWD/src/qqutil/qqfiledialog.cpp \
                  $$PWD/src/qqutil/qqlineedit.cpp \
                  $$PWD/src/qqutil/qqmainwindow.cpp \
                  $$PWD/src/qqutil/qqmenuhelper.cpp \
                  $$PWD/src/qqutil/qqspinbox.cpp \
                  $$PWD/src/qqutil/qqtoolbar.cpp \
                  $$PWD/src/qqutil/qqtoolbarstretch.cpp \
                  $$PWD/src/qqutil/qqtreeview.cpp \
                  $$PWD/src/roweditor.cpp \
                  $$PWD/src/rowviewer.cpp \
                  $$PWD/src/sorteditor.cpp \
                  $$PWD/src/timewidget.cpp \
                  $$PWD/src/view.cpp \
                  $$PWD/src/viewdisplay.cpp \
                  $$PWD/src/vieweditor.cpp \
                  $$PWD/src/vsfmanager.cpp \
                  $$PWD/src/xmlexport.cpp \
                  $$PWD/src/xmlimport.cpp

# Stuff for all Linux/UNIX versions
unix {
    LIBS               += -lm -lmk4
    VERSION             = $$system(cat $$PWD/packaging/version_number)
    COPYRIGHT_YEARS     = $$system(cat $$PWD/packaging/copyright_years)
}

# Stuff for Android
android-clang {
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/packaging/android/apk_template
    ANDROID_TARGET_SDK_VERSION = 33
    CONFIG             += release
    DEFINES            += MOBILE ANDROID
    LIBS               += -L$$PWD/packaging/android/apk_template/libs/$$QT_ARCH
    # Avoid overwriting intermediate files written in parallel for different ABIs
    OBJECTS_DIR     = build/$$QT_ARCH
    MOC_DIR         = build/$$QT_ARCH
    RCC_DIR         = build/$$QT_ARCH
    DISTFILES += \
        $$PWD/packaging/android/apk_template/AndroidManifest.xml \
        $$PWD/packaging/android/apk_template/gradle/wrapper/gradle-wrapper.jar \
        $$PWD/packaging/android/apk_template/gradlew \
        $$PWD/packaging/android/apk_template/res/drawable-ldpi/icon.png \
        $$PWD/packaging/android/apk_template/res/drawable-mdpi/icon.png \
        $$PWD/packaging/android/apk_template/res/drawable-tvdpi/icon.png \
        $$PWD/packaging/android/apk_template/res/drawable-hdpi/icon.png \
        $$PWD/packaging/android/apk_template/res/drawable-xhdpi/icon.png \
        $$PWD/packaging/android/apk_template/res/drawable-xxhdpi/icon.png \
        $$PWD/packaging/android/apk_template/res/drawable-xxxhdpi/icon.png \
        $$PWD/packaging/android/apk_template/res/values/libs.xml \
        $$PWD/packaging/android/apk_template/build.gradle \
        $$PWD/packaging/android/apk_template/gradle/wrapper/gradle-wrapper.properties \
        $$PWD/packaging/android/apk_template/gradlew.bat
    HEADERS            += $$PWD/src/qqutil/actionbar.h \
                          $$PWD/src/qqutil/qqandroidstyle.h
    INCLUDEPATH        += $$PWD/metakit/include
    QT                 += androidextras svg
    RESOURCES           = $$PWD/resources/android.qrc
    SOURCES            += $$PWD/src/qqutil/actionbar.cpp \
                          $$PWD/src/qqutil/qqandroidstyle.cpp
    TARGET              = PortaBase
}

# Stuff for macOS
macx {
    CONFIG             += c++11 release x86_64
    QMAKE_CXXFLAGS     += -stdlib=libc++ -std=c++11
    QT                 += macextras
    INCLUDEPATH        += /usr/local/include
    LIBS               += -L/usr/local/lib -framework Foundation
    TARGET              = PortaBase
    HEADERS            += $$PWD/src/qqutil/qqmactoolbardelegate.h \
                          $$PWD/src/qqutil/qqmactoolbarutils.h
    OBJECTIVE_SOURCES  += $$PWD/src/qqutil/qqmactoolbardelegate.mm \
                          $$PWD/src/qqutil/qqmactoolbarutils.mm
    RESOURCES           = $$PWD/resources/mac.qrc
    ICON                = $$PWD/packaging/mac/PortaBase.icns
    DOCUMENT_ICON.files = $$PWD/packaging/mac/PortaBaseFile.icns
    DOCUMENT_ICON.path  = Contents/Resources
    CS_LPROJ.files      = $$PWD/packaging/mac/cs.lproj/InfoPlist.strings \
                          $$PWD/packaging/mac/cs.lproj/locversion.plist
    CS_LPROJ.path       = Contents/Resources/cs.lproj
    DE_LPROJ.files      = $$PWD/packaging/mac/de.lproj/InfoPlist.strings \
                          $$PWD/packaging/mac/de.lproj/locversion.plist
    DE_LPROJ.path       = Contents/Resources/de.lproj
    EN_LPROJ.files      = $$PWD/packaging/mac/en.lproj/InfoPlist.strings \
                          $$PWD/packaging/mac/en.lproj/locversion.plist
    EN_LPROJ.path       = Contents/Resources/en.lproj
    ES_LPROJ.files      = $$PWD/packaging/mac/es.lproj/InfoPlist.strings \
                          $$PWD/packaging/mac/es.lproj/locversion.plist
    ES_LPROJ.path       = Contents/Resources/es.lproj
    FR_LPROJ.files      = $$PWD/packaging/mac/fr.lproj/InfoPlist.strings \
                          $$PWD/packaging/mac/fr.lproj/locversion.plist
    FR_LPROJ.path       = Contents/Resources/fr.lproj
    IT_LPROJ.files      = $$PWD/packaging/mac/it.lproj/InfoPlist.strings \
                          $$PWD/packaging/mac/it.lproj/locversion.plist
    IT_LPROJ.path       = Contents/Resources/it.lproj
    JA_LPROJ.files      = $$PWD/packaging/mac/ja.lproj/InfoPlist.strings \
                          $$PWD/packaging/mac/ja.lproj/locversion.plist
    JA_LPROJ.path       = Contents/Resources/ja.lproj
    PL_LPROJ.files      = $$PWD/packaging/mac/pl.lproj/InfoPlist.strings \
                          $$PWD/packaging/mac/pl.lproj/locversion.plist
    PL_LPROJ.path       = Contents/Resources/pl.lproj
    SK_LPROJ.files      = $$PWD/packaging/mac/sk.lproj/InfoPlist.strings \
                          $$PWD/packaging/mac/sk.lproj/locversion.plist
    SK_LPROJ.path       = Contents/Resources/sk.lproj
    ZH_HANS_LPROJ.files = $$PWD/packaging/mac/zh-Hans.lproj/InfoPlist.strings \
                          $$PWD/packaging/mac/zh-Hans.lproj/locversion.plist
    ZH_HANS_LPROJ.path  = Contents/Resources/zh-Hans.lproj
    ZH_HANT_LPROJ.files = $$PWD/packaging/mac/zh-Hant.lproj/InfoPlist.strings \
                          $$PWD/packaging/mac/zh-Hant.lproj/locversion.plist
    ZH_HANT_LPROJ.path  = Contents/Resources/zh-Hant.lproj
    QMAKE_BUNDLE_DATA  += DOCUMENT_ICON \
                          CS_LPROJ \
                          DE_LPROJ \
                          EN_LPROJ \
                          ES_LPROJ \
                          FR_LPROJ \
                          IT_LPROJ \
                          JA_LPROJ \
                          PL_LPROJ \
                          SK_LPROJ \
                          ZH_HANS_LPROJ \
                          ZH_HANT_LPROJ
    QMAKE_INFO_PLIST    = $$PWD/packaging/mac/Info.plist
}

# Stuff for Maemo
maemo5|contains(QT_CONFIG, hildon) {
    CONFIG             += debug qdbus
    DEFINES            += MOBILE
    LIBS               += -L$$PWD/../../src/metakit/builds
    isEmpty(PREFIX) {
        PREFIX          = /usr/local
    }
    DATADIR             = $$PREFIX/share
    DEFINES            += DATADIR=\\\"$$DATADIR\\\" PKGDATADIR=\\\"$$PKGDATADIR\\\"
    TARGET              = portabase
    INSTALLS           += target desktop service icon16 icon22 icon32 icon48 icon64 mime
    desktop.path        = $$DATADIR/applications/hildon
    desktop.files       = $$PWD/packaging/maemo/portabase.desktop
    service.path        = $$DATADIR/dbus-1/services
    service.files       = $$PWD/packaging/maemo/net.sourceforge.portabase.service
    icon16.path         = $$DATADIR/icons/hicolor/16x16/apps
    icon16.files        = $$PWD/resources/icons/linux/16x16/portabase.png
    icon22.path         = $$DATADIR/icons/hicolor/22x22/apps
    icon22.files        = $$PWD/resources/icons/linux/22x22/portabase.png
    icon32.path         = $$DATADIR/icons/hicolor/32x32/apps
    icon32.files        = $$PWD/resources/icons/linux/32x32/portabase.png
    icon48.path         = $$DATADIR/icons/hicolor/48x48/apps
    icon48.files        = $$PWD/resources/icons/linux/48x48/portabase.png
    icon64.path         = $$DATADIR/icons/hicolor/64x64/apps
    icon64.files        = $$PWD/resources/icons/linux/64x64/portabase.png
    mime.path           = $$DATADIR/mime/packages
    mime.files          = $$PWD/packaging/maemo/portabase-mime.xml
}
contains(QT_CONFIG, hildon):!maemo5 {
    target.path         = $$PREFIX/bin
    RESOURCES           = $$PWD/resources/diablo.qrc
}
maemo5 {
    QT                 += dbus maemo5 network webkit
    target.path         = /opt/maemo
    RESOURCES           = $$PWD/resources/fremantle.qrc
    HEADERS            += $$PWD/src/image/imagereply.h \
                          $$PWD/src/pbnetworkaccessmanager.h
    SOURCES            += $$PWD/src/image/imagereply.cpp \
                          $$PWD/src/pbnetworkaccessmanager.cpp
}

# Stuff for other Linux/UNIX platforms
unix:!android-clang:!macx:!maemo5:!contains(QT_CONFIG, hildon) {
    #QMAKE_CXXFLAGS       += -O0 # for valgrind
    CONFIG               += debug
    LIBS                 += -L$$PWD/metakit/builds
    RESOURCES             = $$PWD/resources/linux.qrc
}

# Stuff for Windows
win32 {
    CONFIG                 += release
    TARGET                  = PortaBase
    RESOURCES               = $$PWD/resources/windows.qrc
    LIBS                   += $$PWD/metakit/builds/libmk4.a
    RC_FILE                 = $$PWD/portabase.rc
    INCLUDEPATH            += $$PWD/metakit/include
    VERSION                 = $$system(type $$PWD\\packaging\\version_number)
    COPYRIGHT_YEARS         = $$system(type $$PWD\\packaging\\copyright_years)
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
