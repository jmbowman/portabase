TEMPLATE        = app
CONFIG         += qt warn_on release thread
TRANSLATIONS    = portabase_ja.ts \
                  portabase_tw.ts \
                  portabase_fr.ts \
                  portabase_cs.ts
TARGET          = portabase
win32 {
    DEFINES    += QT_DLL
    RC_FILE     = portabase.rc
    QMAKE_CXXFLAGS_RELEASE += /MD
    INCLUDEPATH += D:\Devel\metakit-2.4.9.3\include \
                   D:\Devel\jpeg-6b \
                   D:\Devel
    LIBS       += mk4vc60s.lib beecrypt.lib libjpeg.lib
}
else {
    LIBS       += -lm -lmk4 -lbeecrypt -ljpeg
}
HEADERS         = portabase.h \
                  view.h \
                  viewdisplay.h \
                  database.h \
                  datatypes.h \
                  dbeditor.h \
                  columneditor.h \
                  roweditor.h \
                  csvutils.h \
                  vieweditor.h \
                  noteeditor.h \
                  notebutton.h \
                  sorteditor.h \
                  condition.h \
                  filter.h \
                  conditioneditor.h \
                  filtereditor.h \
                  datewidget.h \
                  preferences.h \
                  enumeditor.h \
                  enummanager.h \
                  timewidget.h \
                  pdbfile.h \
                  mobiledb.h \
                  importutils.h \
                  rowviewer.h \
                  desktop/QtaDatePicker.h \
                  desktop/applnk.h \
                  desktop/config.h \
                  desktop/filemanager.h \
                  desktop/fileselector.h \
                  desktop/importdialog.h \
                  desktop/qpeapplication.h \
                  desktop/resource.h \
                  desktop/timestring.h \
                  desktop/helpbrowser.h \
                  xmlexport.h \
                  xmlimport.h \
                  commandline.h \
                  bytestream.h \
                  crypto.h \
                  passdialog.h \
                  csverror.h \
                  shadedlistitem.h \
                  colorbutton.h \
                  calculator.h \
                  numberwidget.h \
                  desktop/dynamicedit.h \
                  calc/calcnode.h \
                  calc/calcnodeeditor.h \
                  calc/calcdateeditor.h \
                  calc/calceditor.h \
                  calc/calcwidget.h \
                  desktop/newfiledialog.h \
                  pbdialog.h \
                  calc/calctimeeditor.h \
                  menuactions.h \
                  image/imageeditor.h \
                  image/imagereader.h \
                  image/imageselector.h \
                  image/imageutils.h \
                  image/imageviewer.h \
                  image/imagewidget.h \
                  image/slideshowdialog.h \
                  desktop/oldconfig.h \
                  qqdialog.h
SOURCES         = main.cpp \
                  portabase.cpp \
                  view.cpp \
                  viewdisplay.cpp \
                  database.cpp \
                  dbeditor.cpp \
                  columneditor.cpp \
                  roweditor.cpp \
                  csvutils.cpp \
                  vieweditor.cpp \
                  noteeditor.cpp \
                  notebutton.cpp \
                  sorteditor.cpp \
                  condition.cpp \
                  filter.cpp \
                  conditioneditor.cpp \
                  filtereditor.cpp \
                  datewidget.cpp \
                  preferences.cpp \
                  enumeditor.cpp \
                  enummanager.cpp \
                  timewidget.cpp \
                  pdbfile.cpp \
                  mobiledb.cpp \
                  importutils.cpp \
                  rowviewer.cpp \
                  desktop/QtaDatePicker.cpp \
                  desktop/applnk.cpp \
                  desktop/config.cpp \
                  desktop/filemanager.cpp \
                  desktop/fileselector.cpp \
                  desktop/importdialog.cpp \
                  desktop/qpeapplication.cpp \
                  desktop/resource.cpp \
                  desktop/timestring.cpp \
                  desktop/helpbrowser.cpp \
                  xmlexport.cpp \
                  xmlimport.cpp \
                  commandline.cpp \
                  bytestream.cpp \
                  crypto.cpp \
                  passdialog.cpp \
                  csverror.cpp \
                  shadedlistitem.cpp \
                  colorbutton.cpp \
                  calculator.cpp \
                  numberwidget.cpp \
                  desktop/dynamicedit.cpp \
                  calc/calcnode.cpp \
                  calc/calcnodeeditor.cpp \
                  calc/calcdateeditor.cpp \
                  calc/calceditor.cpp \
                  calc/calcwidget.cpp \
                  desktop/newfiledialog.cpp \
                  pbdialog.cpp \
                  calc/calctimeeditor.cpp \
                  menuactions.cpp \
                  image/imageeditor.cpp \
                  image/imagereader.cpp \
                  image/imageselector.cpp \
                  image/imageutils.cpp \
                  image/imageviewer.cpp \
                  image/imagewidget.cpp \
                  image/slideshowdialog.cpp \
                  desktop/oldconfig.cpp \
                  qqdialog.cpp
