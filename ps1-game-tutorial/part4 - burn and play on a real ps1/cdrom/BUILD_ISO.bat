@ECHO OFF

cd ..

psymake
del mem.map
del main.sym
del main.cpe

cd cdrom

DEL *.ISO
DEL *.IMG
DEL *.TOC
CLS
ECHO Please Wait!
ECHO Building image
TOOLS\BUILDCD -l -iGAME.IMG TOOLS\GAME.CTI
REM PAUSE
ECHO Converting GAME.IMG to GAME.ISO...
TOOLS\STRIPISO S 2352 GAME.IMG GAME.ISO
ECHO.
ECHO The GAME CD-ROM Image was Created Successfully!
TOOLS\PSXLICENSE /eu /i GAME.ISO

SET HOUR=%time:~0,2%
SET dtStamp9=%date:~-4%%date:~4,2%%date:~7,2%_0%time:~1,1%%time:~3,2%%time:~6,2% 
SET dtStamp24=%date:~-4%%date:~4,2%%date:~7,2%_%time:~0,2%%time:~3,2%%time:~6,2%

if "%HOUR:~0,1%" == " " (SET dtStamp=%dtStamp9%) else (SET dtStamp=%dtStamp24%)

if not exist "builds" mkdir builds
mkdir builds\%dtStamp%

move GAME.ISO builds\%dtStamp%

del QSHEET.TOC
del CDW900E.TOC
del GAME.IMG
cd ..
del main.exe
ECHO Your game have been built into an ISO file in the %dtStamp% folder. 
PAUSE