BUILD_TYPE=Release
SVN_VER_MAJ_MIN=1.7

VCBUILD_OPT=/useenv /nologo /nocolor "/info:Info: " "/error:Error: " "/warning:Warn: "

build: all test kit

all:
	cd ..\Source & $(PYTHON) setup.py configure --verbose --platform=win32 --svn-root-dir=$(TARGET)\svn-win32-%SVN_VER% --pycxx-dir=$(PYCXX)
	cd ..\Source & $(MAKE)

test:
	cd  ..\Tests & $(MAKE)

clean:
	cd ..\Source & if exist makefile $(MAKE) clean
	cd ..\Source & if exist sept del sept
	cd ..\Tests & if exist makefile $(MAKE) clean
	cd ..\kit\Win32-$(SVN_VER_MAJ_MIN) & $(MAKE) clean

kit:
	cd ..\kit\Win32-$(SVN_VER_MAJ_MIN) & $(MAKE) all_msvc90

install:
	..\kit\Win32\tmp\output\setup.exe
