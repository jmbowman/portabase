REM Generate help files for a single language

PATH = %PATH%;C:\Python27;C:\Python27\Scripts
CD resources\help

:loop
IF NOT "%1" EQU "" (
    IF "%1" EQU "--no-clean" (
        SET no_clean="yes"
        SHIFT
        GOTO :loop
    )
    IF "%1" EQU "--update" (
        SET update="yes"
        SHIFT
        GOTO :loop
    )
    SET HELP_LANG=%1
    SHIFT
    GOTO :loop
)
IF NOT DEFINED no_clean (
    IF EXIST _build (
        CALL .\make clean
    )
)
IF NOT DEFINED HELP_LANG (
    SET HELP_LANG=en
)
IF DEFINED update (
    tx pull --mode developer -f -l %HELP_LANG%
)
IF EXIST translations\%HELP_LANG% (
    CD translations\%HELP_LANG%
    IF NOT EXIST LC_MESSAGES MD LC_MESSAGES
    FOR /F %%G IN ('DIR /B *.po') DO (
        msgfmt %%G -o LC_MESSAGES\%%~nG.mo
    )
    IF EXIST sphinx.js COPY sphinx.js LC_MESSAGES
    CD ..\..
)
CALL .\make html
CALL .\make text
MOVE _build\text\*.txt _build\html\_sources
IF EXIST translations\%HELP_LANG%\LC_MESSAGES (
    RD /S /Q translations\%HELP_LANG%\LC_MESSAGES
)

cd ..\..
