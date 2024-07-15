REM Make sure these paths are appropriate for your system:

SET MINGW_BIN_DIR=%IQTA_TOOLS%\mingw810_64\bin
SET QT_LIB_DIR=%QT_ROOT_DIR%\bin

IF NOT EXIST build\windows MKDIR build\windows
COPY CHANGES build\windows\CHANGES.txt
COPY COPYING build\windows\COPYING.txt
COPY README.rst build\windows\README.txt
packaging\windows\UNIX2DOS build\windows\CHANGES.txt
packaging\windows\UNIX2DOS build\windows\COPYING.txt
packaging\windows\UNIX2DOS build\windows\README.rst
COPY %MINGW_BIN_DIR%\libgcc_s_seh-1.dll build\windows
COPY "%MINGW_BIN_DIR%\libstdc++-6.dll" build\windows
COPY %QT_LIB_DIR%\Qt5Core.dll build\windows
COPY %QT_LIB_DIR%\Qt5Gui.dll build\windows
COPY %QT_LIB_DIR%\Qt5PrintSupport.dll build\windows
COPY %QT_LIB_DIR%\Qt5Widgets.dll build\windows
COPY %QT_LIB_DIR%\Qt5Xml.dll build\windows
IF NOT EXIST build\windows\imageformats MKDIR build\windows\imageformats
COPY %QT_PLUGIN_PATH%\imageformats\qjpeg.dll build\windows\imageformats

CALL packaging\windows\generate_all_help.bat

IF NOT "%1" EQU "" (
    iscc.exe /dSignIt=true "/SkSign=$qC:\Program Files\kSign\kSignCMD.exe$q /f $qC:\Users\Jeremy Bowman\Documents\jb_certificate.pfx$q /p %1 $p" portabase.iss
) ELSE (
    iscc.exe "portabase.iss"
)
