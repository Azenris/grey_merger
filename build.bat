@echo OFF
cls

:: -O2					= Creates fast code.
:: -Ot					= Favors fast code.
:: -GF					= Enables string pooling.
:: -GR-					= Disable RTTI.
:: -Z7					= Produces object files that also contain full symbolic debugging information.
:: -Zc:preprocessor		= Use the new conforming preprocessor.
:: -Zc:strictStrings	= Disable string-literal to char*
:: -MD					= Compiles to create a multithreaded DLL, by using MSVCRT.lib.
:: -MDd					= Compiles to create a debug multithreaded DLL, by using MSVCRTD.lib.
:: -LD					= Creates a dynamic-link library.
:: -W0 -W1 -W2 -W3 -W4 	= Set output warning level.
:: -wd<Number> 			= Disable a specific warning.
:: -WX 					= Treat warnings as errors.
:: -FC 					= Displays the full path of source code files passed to cl.exe in diagnostic text.

SET debugMode=0
SET platform=PLATFORM_WINDOWS
SET name=grey_merger
SET buildDir=TEMP\
SET objectDir=%buildDir%Objects\
SET warnings=-WX -W4 -wd4100 -wd4201 -wd4706
SET includes=-Ithird_party\
SET defines=-DC_PLUS_PLUS -D_CRT_SECURE_NO_WARNINGS -D%platform%
SET links=
SET flags=-std:c++20 -Zc:preprocessor -Zc:strictStrings -GR-

if %debugMode% == 1 (
	SET defines=%defines% -DDEBUG
	SET flags=%flags% -Z7 -FC -MDd
	SET links=%links% third_party\zlib\zlibd64.lib
) else (
	SET flags=%flags% -MD -O2 -Ot -GF
	SET links=%links% third_party\zlib\zlib64.lib
)

if not exist %buildDir% ( mkdir %buildDir% )
if not exist %objectDir% ( mkdir %objectDir% )

del %buildDir%*.pdb > NUL 2> NUL

SET commands=-nologo %flags% %warnings% %defines% %math%

cl %commands% -Fe%buildDir%%name%.exe -Fo%objectDir% src\main.cpp %includes% %links% -INCREMENTAL:NO
if not %ERRORLEVEL% == 0 ( goto build_failed )

:build_success
echo Build success!
goto build_end

:build_failed
echo Build failed.
goto build_end

:build_end