setlocal
set PY_VER=%1
set SVN_VER_MAJ_MIN=%2
set BUILD_ARCH=%3

pushd ..
call ..\Builder\builder_custom_init.cmd
popd

nmake /nologo ARCH=%BUILD_ARCH% VC_VER=%VC_VER%
endlocal
