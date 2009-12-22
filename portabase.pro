TEMPLATE        = app
CONFIG         += qt warn_on debug thread
QT             += xml
TARGET          = PortaBase
DESTDIR         = build
OBJECTS_DIR     = build
MOC_DIR         = build
RCC_DIR         = build
VERSION         = 2.0.0
TRANSLATIONS    = resources/translations/portabase_cs.ts \
                  resources/translations/portabase_de.ts \
                  resources/translations/portabase_fr.ts \
                  resources/translations/portabase_ja.ts \
                  resources/translations/portabase_zh_TW.ts

# Files common between all versions
HEADERS         = calc/calcdateeditor.h \
                  calc/calceditor.h \
                  calc/calcnode.h \
                  calc/calcnodeeditor.h \
                  calc/calctimeeditor.h \
                  calc/calcwidget.h \
                  calculator.h \
                  columneditor.h \
                  commandline.h \
                  condition.h \
                  conditioneditor.h \
                  csverror.h \
                  csvutils.h \
                  database.h \
                  datatypes.h \
                  datedialog.h \
                  datewidget.h \
                  dbeditor.h \
                  dynamicedit.h \
                  encryption/blowfish.h \
                  encryption/bytestream.h \
                  encryption/crypto.h \
                  encryption/randomkit/randomkit.h \
                  encryption/randomkit/rk_isaac.h \
                  encryption/randomkit/rk_mt.h \
                  enumeditor.h \
                  enummanager.h \
                  eventfilter.h \
                  factory.h \
                  filter.h \
                  filtereditor.h \
                  image/imageeditor.h \
                  image/imageselector.h \
                  image/imageutils.h \
                  image/imageviewer.h \
                  image/imagewidget.h \
                  image/slideshowdialog.h \
                  importdialog.h \
                  importutils.h \
                  menuactions.h \
                  metakitfuncs.h \
                  mobiledb.h \
                  notebutton.h \
                  noteeditor.h \
                  numberwidget.h \
                  oldconfig.h \
                  passdialog.h \
                  pbdialog.h \
                  pdbfile.h \
                  portabase.h \
                  preferences.h \
                  qqutil/qqdialog.h \
                  qqutil/qqhelpbrowser.h \
                  qqutil/qqmenuhelper.h \
                  roweditor.h \
                  rowviewer.h \
                  sorteditor.h \
                  timewidget.h \
                  view.h \
                  viewdisplay.h \
                  vieweditor.h \
                  xmlexport.h \
                  xmlimport.h
SOURCES         = calc/calcdateeditor.cpp \
                  calc/calceditor.cpp \
                  calc/calcnode.cpp \
                  calc/calcnodeeditor.cpp \
                  calc/calctimeeditor.cpp \
                  calc/calcwidget.cpp \
                  calculator.cpp \
                  columneditor.cpp \
                  commandline.cpp \
                  condition.cpp \
                  conditioneditor.cpp \
                  csverror.cpp \
                  csvutils.cpp \
                  database.cpp \
                  datedialog.cpp \
                  datewidget.cpp \
                  dbeditor.cpp \
                  dynamicedit.cpp \
                  encryption/blowfish.cpp \
                  encryption/bytestream.cpp \
                  encryption/crypto.cpp \
                  encryption/randomkit/rk_isaac.c \
                  encryption/randomkit/rk_mt.c \
                  enumeditor.cpp \
                  enummanager.cpp \
                  eventfilter.cpp \
                  factory.cpp \
                  filter.cpp \
                  filtereditor.cpp \
                  image/imageeditor.cpp \
                  image/imageselector.cpp \
                  image/imageutils.cpp \
                  image/imageviewer.cpp \
                  image/imagewidget.cpp \
                  image/slideshowdialog.cpp \
                  importdialog.cpp \
                  importutils.cpp \
                  main.cpp \
                  menuactions.cpp \
                  metakitfuncs.cpp \
                  mobiledb.cpp \
                  notebutton.cpp \
                  noteeditor.cpp \
                  numberwidget.cpp \
                  oldconfig.cpp \
                  passdialog.cpp \
                  pbdialog.cpp \
                  pdbfile.cpp \
                  portabase.cpp \
                  preferences.cpp \
                  qqutil/qqdialog.cpp \
                  qqutil/qqhelpbrowser.cpp \
                  qqutil/qqmenuhelper.cpp \
                  roweditor.cpp \
                  rowviewer.cpp \
                  sorteditor.cpp \
                  timewidget.cpp \
                  view.cpp \
                  viewdisplay.cpp \
                  vieweditor.cpp \
                  xmlexport.cpp \
                  xmlimport.cpp

include(color_picker/qtcolorpicker.pri)

# Stuff for all Linux/UNIX versions
unix:LIBS       += -lm -lmk4 -ljpeg

# Stuff for Mac OS X
macx {
    RESOURCES           = resources/mac.qrc
    ICON                = packaging/PortaBase.icns
    DOCUMENT_ICON.files = packaging/PortaBaseFile.icns
    DOCUMENT_ICON.path  = Contents/Resources
    QMAKE_BUNDLE_DATA  += DOCUMENT_ICON
    QMAKE_INFO_PLIST    = packaging/Info.plist
    #CONFIG             += x86 ppc
    #INCLUDEPATH        += /opt/local/include
}

# Stuff for Windows
win32:LIBS                   += mk4vc60s.lib libjpeg.lib
win32:DEFINES                += QT_DLL
win32:RC_FILE                 = portabase.rc
win32:QMAKE_CXXFLAGS_RELEASE += /MD
win32:INCLUDEPATH            += D:\Devel\metakit-2.4.9.3\include \
                                D:\Devel\jpeg-6b \
                                D:\Devel
