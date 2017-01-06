if not "%1" == "" set PY_VER=%1
if not "%2" == "" set SVN_VER_MAJ_MIN=%2
if not "%3" == "" set BUILD_ARCH=%3

if "%PY_VER%" == "" goto :eof
if "%SVN_VER_MAJ_MIN%" == "" goto :eof
if "%BUILD_ARCH%" == "" goto :eof

if /i "%BUILD_ARCH%" EQU "win32" set BUILD_ARCH=Win32
if /i "%BUILD_ARCH%" EQU "win64" set BUILD_ARCH=Win64

rem restore the path on later runs
if "%BUILD_SAVED_PATH%" NEQ "" PATH %BUILD_SAVED_PATH%
if "%BUILD_SAVED_PATH%" EQU "" set BUILD_SAVED_PATH=%PATH%

set SVN_VER_MAJ_DASH_MIN=%SVN_VER_MAJ_MIN:.=-%
set PY_MAJ=%PY_VER:~0,1%
set PY_MIN=%PY_VER:~2%

echo on
rem Save CWD
pushd .

pushd ..\..\ReleaseEngineering\Windows
call build-config.cmd %PY_VER% %SVN_VER_MAJ_MIN% %BUILD_ARCH%
popd

set PYCXX=%BUILDER_TOP_DIR%\Import\pycxx-%PYCXX_VER%

set PYTHONPATH=%BUILDER_TOP_DIR%\Source

rem prove the python version selected works
%PYTHON% -c "import sys;print( 'Info: Python Version %%s' %% sys.version )"
echo SVN Version:
svn --version --quiet

rem restore original CWD
popd

%PYTHON% setup.py configure --verbose --platform=%BUILD_ARCH% --pycxx-dir=%USERPROFILE%\wc\svn\PyCxx --distro-dir=%TARGET%\dist

if exist pysvn\_pysvn* del pysvn\_pysvn*

if ERRORLEVEL 1 goto :EOF
nmake clean
if ERRORLEVEL 1 goto :EOF
nmake
if ERRORLEVEL 1 goto :EOF
cd ..\Tests
if ERRORLEVEL 1 goto :EOF
nmake clean
if ERRORLEVEL 1 goto :EOF
nmake
