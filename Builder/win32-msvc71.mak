BUILD_TYPE=Release
SVN_VER_MAJ_MIN=1.5

build: all test kit

all: all-$(SVN_VER_MAJ_MIN)

all-1.4:
	cd ..\Source & devenv pysvn-for-svn-1-4-msvc71.sln /useenv /build "$(BUILD_TYPE)"  /project "pysvn"

all-1.5:
	cd ..\Source & devenv pysvn-for-svn-1-5-msvc71.sln /useenv /build "$(BUILD_TYPE)"  /project "pysvn"

all-1.6:
	cd ..\Source & devenv pysvn-for-svn-1-6-msvc71.sln /useenv /build "$(BUILD_TYPE)"  /project "pysvn"

clean: clean-$(SVN_VER_MAJ_MIN)

clean-1.4:
	cd ..\Source & devenv pysvn-for-svn-1-4-msvc71.sln /useenv /clean "$(BUILD_TYPE)"  /project "pysvn"
	cd ..\Source & del sept
	cd ..\Tests & $(MAKE) -f win32.mak SVN_VER_MAJ_MIN=1.4 clean
	cd ..\kit\Win32-1.4 & $(MAKE) clean

clean-1.5:
	cd ..\Source & devenv pysvn-for-svn-1-5-msvc71.sln /useenv /clean "$(BUILD_TYPE)"  /project "pysvn"
	cd ..\Source & del sept
	cd ..\Tests & $(MAKE) -f win32.mak SVN_VER_MAJ_MIN=1.5 clean
	cd ..\kit\Win32-1.5 & $(MAKE) clean

clean-1.6:
	cd ..\Source & devenv pysvn-for-svn-1-6-msvc71.sln /useenv /clean "$(BUILD_TYPE)"  /project "pysvn"
	cd ..\Source & del sept
	cd ..\Tests & $(MAKE) -f win32.mak SVN_VER_MAJ_MIN=1.6 clean
	cd ..\kit\Win32-1.6 & $(MAKE) clean

kit: kit-$(SVN_VER_MAJ_MIN)

kit-1.4:
	cd ..\kit\Win32-1.4 & $(MAKE) all_msvc71

kit-1.5:
	cd ..\kit\Win32-1.5 & $(MAKE) all_msvc71

kit-1.6:
	cd ..\kit\Win32-1.6 & $(MAKE) all_msvc71

install: install-$(SVN_VER_MAJ_MIN)

install-1.4:
	..\kit\Win32\tmp\output\setup.exe

install-1.5:
	..\kit\Win32\tmp\output\setup.exe

install-1.6:
	..\kit\Win32\tmp\output\setup.exe

test: test-$(SVN_VER_MAJ_MIN)

test-1.4:
	cd  ..\Tests & $(MAKE) -f win32.mak SVN_VER_MAJ_MIN=1.4 KNOWN_GOOD_VERSION=py$(PY_MAJ)-svn$(SVN_VER_MAJ_MIN)

test-1.5:
	cd  ..\Tests & $(MAKE) -f win32.mak SVN_VER_MAJ_MIN=1.5 KNOWN_GOOD_VERSION=py$(PY_MAJ)-svn$(SVN_VER_MAJ_MIN)

test-1.6:
	cd  ..\Tests & $(MAKE) -f win32.mak SVN_VER_MAJ_MIN=1.6 KNOWN_GOOD_VERSION=py$(PY_MAJ)-svn$(SVN_VER_MAJ_MIN)

