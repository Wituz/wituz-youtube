::---------------------------------------------------------------
:: NAME			- Images to header
:: DESCRIPTION	- Converts images in a folder to a single header
:: YEAR			- 2018
:: AUTHOR 		- Wituz (Frederik W.)
:: WEBSITE 		- http://wituz.com
:: VERSION		- 1.0
::---------------------------------------------------------------

@echo off
setlocal enabledelayedexpansion
echo -----------------------------------
echo  CONVERING IMAGES TO A HEADER FILE
echo -----------------------------------

:: Delete existing files and re-create the working directories
if exist "images\tim" rmdir /s /q "images\tim"
if exist "images\8bit" rmdir /s /q "images\8bit"
if exist "images\headers" rmdir /s /q "images\headers"
mkdir "images\tim"
mkdir "images\8bit"
mkdir "images\headers"

:: Delete the current images.h
echo Creating images.h
if exist "images.h" del "images.h"

:: Convert the images to 8-bit
for %%i in (images/*.*) do (
	.\bin\convert.exe images/%%i -depth 2 images\8bit\img_%%i 
	echo Converted %%i to 8-bit .BMP
)

SET currentX=320
SET currentY=0
SET maxHeight=0
SET canvasWidth=1024
SET canvasHeight=512
SET index=0

:: Convert the 8-bit images to TIM
for %%i in (images/8bit/*.*) do (
	FOR /F "tokens=* USEBACKQ" %%F IN (`.\bin\magick.exe identify -format "%%w" images\8bit\%%i`) DO (
		SET width=%%F
	)
	FOR /F "tokens=* USEBACKQ" %%F IN (`.\bin\magick.exe identify -format "%%h" images\8bit\%%i`) DO (
		SET height=%%F
	)
	
	if !height! GTR !maxHeight! set maxHeight = !height!
	
	set /A nextX = !currentX! + !width! / 2
	if !nextX! GTR !canvasWidth! (
		SET currentX = 320
		SET /A currentY = !currentY! + !maxHeight!
		SET /A maxHeight = 0
	)
	set /A clutX = 320
	set /A clutY = 480 - !index!
	.\bin\img2tim.exe -bpp 8 -b -tcol 0 0 0 -usealpha -plt !clutX! !clutY! -org !currentX! !currentY! -o "%CD%\images\tim\%%~ni.tim" "%CD%\images\8bit\%%i" 
	SET rawname=%%i
	echo unsigned short !rawname:~0,-4!_gpu_x = !currentX!; >> images.h
	echo unsigned short !rawname:~0,-4!_gpu_y = !currentY!; >> images.h
	echo Converted %%i to .TIM
	SET currentX=!nextX!
	SET /A index = !index! + 1
)

echo TIM files done
echo Converting TIM files to header

:: Convert the TIM images to C headers
for %%i in (images/tim/*.*) do (
	.\bin\bin2h.exe images\tim\%%i images\headers\%%~ni.h %%~ni -nosize 
	echo Converted %%i to a C header
)

:: Write image width and height to images.h
for %%i in (images/8bit/*.*) do (
	FOR /F "tokens=* USEBACKQ" %%F IN (`.\bin\magick identify -format "%%w" images\8bit\%%i`) DO (
		SET width=%%F
	)
	FOR /F "tokens=* USEBACKQ" %%F IN (`.\bin\magick identify -format "%%h" images\8bit\%%i`) DO (
		SET height=%%F
	)
	SET rawname=%%i
	echo unsigned short !rawname:~0,-4!_width = !width!; >> images.h
	echo unsigned short !rawname:~0,-4!_height = !height!; >> images.h
)

:: Write the actual images to images.h
echo Merging headers to one file
for /r "images\headers" %%F in (*.h) do (
	type "%%F" >> images.h
)

set /A imageCount = !index! + 1
echo.
echo -----------------------------------
echo              SUCCESS
echo -----------------------------------
echo images.h contains !imageCount! images
echo Header images.h successfully generated

:: Delete the working directories
::if exist "images\tim" rmdir /s /q "images\tim"
if exist "images\8bit" rmdir /s /q "images\8bit"
if exist "images\tim" rmdir /s /q "images\tim"
if exist "images\headers" rmdir /s /q "images\headers"

PAUSE

