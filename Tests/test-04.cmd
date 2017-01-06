@prompt $P$S$G$S
@rem
@rem   test-04.cmd
@rem       test Tranaction object
@rem

@echo WorkDir: %BUILDER_TOP_DIR%
@echo PYTHON: %PYTHON%
@echo Username: %USERNAME%

setlocal
set PYTHONPATH=%BUILDER_TOP_DIR%\Source;%BUILDER_TOP_DIR%\Examples\Client
set PYSVN=%PYTHON% %BUILDER_TOP_DIR%\Examples\Client\svn_cmd.py --pysvn-testing 01.02.01 --config-dir b:\configdir
mkdir testroot-04
subst b: %CD%\testroot-04
cd /d B:\

svnadmin create b:\repos

rem mkdir
%PYSVN% mkdir file:///b:/repos/trunk -m "test-01 add trunk"
%PYSVN% mkdir file:///b:/repos/trunk/test -m "test-01 add test"

rem Install hooks
echo echo %PYTHON% %BUILDER_TOP_DIR%\Tests\test_04_commit_hook_test_1.py pre-commit %%* ^>b:\pre_test_1.output >>b:\repos\hooks\pre-commit.cmd
echo set PYTHONPATH=%PYTHONPATH% >>b:\repos\hooks\pre-commit.cmd
echo %PYTHON% %BUILDER_TOP_DIR%\Tests\test_04_commit_hook_test_1.py pre-commit %%* ^>^>b:\pre_test_1.output >>b:\repos\hooks\pre-commit.cmd

echo echo %PYTHON% %BUILDER_TOP_DIR%\Tests\test_04_commit_hook_test_1.py post-commit %%* is_revision ^>b:\post_test_1.output >>b:\repos\hooks\post-commit.cmd
echo set PYTHONPATH=%PYTHONPATH% >>b:\repos\hooks\post-commit.cmd
echo %PYTHON% %BUILDER_TOP_DIR%\Tests\test_04_commit_hook_test_1.py post-commit %%* is_revision ^>^>b:\post_test_1.output >>b:\repos\hooks\post-commit.cmd

rem Add one dir
%PYSVN% mkdir file:///b:/repos/trunk/test/a -m "pre-commit test 1"
rem pre_test_1.output start ----------------------------------------
type b:\pre_test_1.output
rem  pre_test_1.output end ------------------------------------------
rem post_test_1.output start ----------------------------------------
type b:\post_test_1.output
rem  post_test_1.output end ------------------------------------------

rem Add two files
%PYSVN% co file:///b:/repos/trunk/test b:\wc
echo file1 ROOT >b:\wc\file1.txt
echo file1 A >b:\wc\a\file1.txt

%PYSVN% add b:\wc\file1.txt
%PYSVN% add b:\wc\a\file1.txt
%PYSVN% checkin -m "Add two files" b:\wc
rem pre_test_1.output start ----------------------------------------
type b:\pre_test_1.output
rem  pre_test_1.output end ------------------------------------------
rem post_test_1.output start ----------------------------------------
type b:\post_test_1.output
rem  post_test_1.output end ------------------------------------------

rem Mod one file Mod one prop

echo file1 ROOT ln 2 >b:\wc\file1.txt
%PYSVN% propset svn:eol-style native b:\wc\a\file1.txt
%PYSVN% checkin -m "Mod one file Mod one prop" b:\wc
rem pre_test_1.output start ----------------------------------------
type b:\pre_test_1.output
rem  pre_test_1.output end ------------------------------------------
rem post_test_1.output start ----------------------------------------
type b:\post_test_1.output
rem  post_test_1.output end ------------------------------------------

rem Delete one file

%PYSVN% rm b:\wc\a\file1.txt
%PYSVN% checkin -m "Delete one file" b:\wc
rem pre_test_1.output start ----------------------------------------
type b:\pre_test_1.output
rem  pre_test_1.output end ------------------------------------------
rem post_test_1.output start ----------------------------------------
type b:\post_test_1.output
rem  post_test_1.output end ------------------------------------------

endlocal
