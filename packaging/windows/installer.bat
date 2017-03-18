REM Make sure these paths are appropriate for your system:

SET mingw_dir=C:\Qt\2010.05\mingw
SET qt_dir=C:\Qt\4.8.2
path = %PATH%;C:\Python27;C:\Python27\Scripts

IF NOT EXIST build\windows MKDIR build\windows
COPY CHANGES build\windows\CHANGES.txt
COPY COPYING build\windows\COPYING.txt
COPY README.rst build\windows\README.txt
packaging\windows\UNIX2DOS build\windows\CHANGES.txt
packaging\windows\UNIX2DOS build\windows\COPYING.txt
packaging\windows\UNIX2DOS build\windows\README.rst
COPY %mingw_dir%\bin\mingwm10.dll build\windows
COPY %mingw_dir%\bin\libgcc_s_dw2-1.dll build\windows
COPY %qt_dir%\bin\QtCore4.dll build\windows
COPY %qt_dir%\bin\QtGui4.dll build\windows
COPY %qt_dir%\bin\QtXml4.dll build\windows
IF NOT EXIST build\windows\accessible MKDIR build\windows\accessible
COPY %qt_dir%\plugins\accessible\qtaccessiblewidgets4.dll build\windows\accessible
IF NOT EXIST build\windows\imageformats MKDIR build\windows\imageformats
COPY %qt_dir%\plugins\imageformats\qjpeg4.dll build\windows\imageformats

REM CALL packaging\windows\generate_all_help.bat

IF NOT "%1" EQU "" (
    "C:\Program Files\Inno Setup 5\ISCC" /dSignIt=true "/SkSign=$qC:\Program Files\kSign\kSignCMD.exe$q /f $qC:\Users\Jeremy Bowman\Documents\jb_certificate.pfx$q /p %1 $p" portabase.iss
) ELSE (
    "C:\Program Files\Inno Setup 5\ISCC" portabase.iss
)
