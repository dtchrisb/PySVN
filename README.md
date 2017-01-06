# PySVN
Barry Scott's PySVN Subversion wrapper for Python, based off of the 1.9.3 version release. Official homepage for the project is http://pysvn.tigris.org/.

# Building
For instructions on building the PySVN Extension, refer to [INSTALL.html](/INSTALL.html/).

## My Build Notes
Building on OS X El Capitan for use with Python 3.5, with APR installed via Homebrew.

    MACOSX_DEPLOYMENT_TARGET=10.12
    export MACOSX_DEPLOYMENT_TARGET
    python3 setup.py configure --apr-inc-dir=/usr/local/Cellar/apr/1.5.2_3/libexec/include/apr-1/ --apu-inc-dir=/usr/local/Cellar/apr-util/1.5.4_4/libexec/include/apr-1/
    make

Still working through error encountered at this point.

# Changelog
| Date | Change |
| ---- | ------ |
| 2017.01.07 | Fixed problem with `Client.diff()` throwing Unicode translation errors when there are "ASCII" characters with values in the range of 128-255 in the returned diff. |
