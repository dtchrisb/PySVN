@echo off
@prompt $P$S$G
@rem
@rem   test-10.cmd for 1.9 commands
@rem       test vacuum
@rem

@echo WorkDir: %BUILDER_TOP_DIR%
@echo PYTHON: %PYTHON%
@echo Username: %USERNAME%

setlocal
set PYTHONPATH=%BUILDER_TOP_DIR%\Source;%BUILDER_TOP_DIR%\Examples\Client
set PYSVN=%PYTHON% %BUILDER_TOP_DIR%\Examples\Client\svn_cmd.py --pysvn-testing 01.09.00 --config-dir b:\configdir
echo Info: PYSVN CMD %PYSVN%
call :cmd_shell mkdir testroot-10
call :cmd_shell subst b: %CD%\testroot-10
call :cmd_shell cd /d b:\

call :cmd_shell mkdir tmp
set TMPDIR=b:\tmp

call :cmd_shell svnadmin create b:\repos

echo Info: Setup - mkdir
call :cmd_pysvn mkdir file:///b:/repos/trunk -m "test-10 add trunk"

echo Info: Setup - checkout wc1
call :cmd_pysvn checkout file:///b:/repos/trunk b:\wc1
call :cmd_shell cd /d b:\wc1

echo Info: Setup - add files and folders

call :cmd_pysvn mkdir folder1
call :cmd_pysvn propset svn:ignore "*~" folder1

call :cmd_createfile folder1\file-a.txt test add file 1
call :cmd_pysvn add folder1\file-a.txt

call :cmd_pysvn checkin -m "commit added files"

call :cmd_createfile folder1/file-a.txt~ test add file 1
call :cmd_createfile folder1/file-b.txt test add file 1

echo Info: vacuum no removes
call :cmd_pysvn status2 --verbose --no-ignore .
call :cmd_pysvn vacuum

echo Info: vacuum remove ignored
call :cmd_pysvn status2 --verbose --no-ignore .
call :cmd_pysvn vacuum --remove-ignored-items

echo Info: vacuum remove versioned
call :cmd_pysvn status2 --verbose --no-ignore .
call :cmd_pysvn vacuum --remove-unversioned-items

echo Info: check final state
call :cmd_pysvn status2 --verbose --no-ignore .


goto :eof
endlocal

:cmd_shell
    echo.
    echo Info: CMD %*
    %*
    goto :eof

:cmd_pysvn
    echo. 1>&2
    echo Info: PYSVN CMD %* 1>&2
    %PYSVN% %*
    goto :eof

:cmd_createfile
    set FILENAME=%1
    shift
    echo Info: Create File %FILENAME% - %1 %2 %3 %4 %5 %6 %7 %8 %9
    call :cmd__echo %1 %2 %3 %4 %5 %6 %7 %8 %9 >%FILENAME%
    goto :eof

:cmd_appendfile
    set FILENAME=%1
    shift
    echo Info: Append File %FILENAME% - %1 %2 %3 %4 %5 %6 %7 %8 %9
    call :cmd__echo %1 %2 %3 %4 %5 %6 %7 %8 %9 >>%FILENAME%
    goto :eof

:cmd__echo
    echo %*
    goto :eof
