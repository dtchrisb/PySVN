setlocal
@rem
@rem   test-02.cmd
@rem       test threading, but needs special svn server
@rem
set PYTHONPATH=.
if exist testroot rmdir /s /q testroot
mkdir testroot
%PYTHON% %BUILDER_TOP_DIR%\Tests\thread_tests.py 20 http://torment.chelsea.private/svn/repos1/Latest/
endlocal
