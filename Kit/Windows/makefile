#
#	makefile - make the windows kit
#
!ifndef ARCH
!error "ARCH must be provided"
!endif


!ifndef VC_VER
!error "VC_VER must be provided"
!endif

all: kit_msvc

kit_msvc: setup_kit_files.py
	if exist tmp rmdir /s /q tmp
	mkdir tmp
	copy ..\..\LICENSE.txt tmp\LICENSE.txt
	xcopy /q $(SVN_BIN)\*.dll tmp
	$(PYTHON) setup_kit_files.py $(ARCH) $(VC_VER)
	"C:\Program Files (x86)\Inno Setup 5\ISCC.exe" tmp\pysvn.iss
	tmp\setup_copy.cmd


debug:
	"C:\Program Files (x86)\Inno Setup 5\Compil32.exe" pysvn.iss

clean:
	if exist tmp rmdir /s /q tmp
