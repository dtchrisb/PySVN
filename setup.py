#
# ====================================================================
# (c) 2005-2009 Barry A Scott.  All rights reserved.
#
# This software is licensed as described in the file LICENSE.txt,
# which you should have received as part of this distribution.
#
# ====================================================================
#
#
#   setup.py
#
#   Handles creation of Python eggs, wheels, and source distributions.
#
#   To build a source distribution, run:
#
#       $ python setup.py sdist
#
#   To build an egg, run:
#
#       $ python setup.py bdist_egg
#
#   To build a wheel, make sure you have the wheel package
#   (`pip install wheel`). This is available on newer systems by defaul.
#   Then run:
#
#       $ python setup.py bdist_wheel
#
#   These can all be built and uploaded to PyPI in one command:
#
#       $ python setup.py sdist bdist_egg bdist_wheel upload
#
#   Subsequent native eggs and wheels can be uploaded to PyPI with:
#
#       $ python setup.py bdist_egg bdist_wheel upload
#
import setuptools
import distutils.sysconfig

import platform
import shutil
import sys
import os
import os.path
from setuptools.command.build_ext import build_ext

pysvn_version_info = {}
f = open( 'Builder/version.info', 'r' )
for line in f:
    key, value = line.strip().split('=')
    pysvn_version_info[ key ] = value


name = "pysvn"
_pysvn_soname = '_pysvn_%d_%d' % sys.version_info[:2]


class BuildExtensions(build_ext):
    """Builds the packages using the PySVN Makefile build system.

    This overrides the Python builder's build_ext command, invoking the
    Makefile build system to compile the deliverable pysvn/ directory. It
    then installs the deliverables to the right place so that they can be
    included in the package.

    By going through the build_ext route, the Python packaging infrastructure
    will generate compiled eggs/wheels that only install on systems compatible
    with the compiled pysvn.so. pip/easy_install will locate and install a
    compatible build, falling back on automatically compiling the source
    distribution.
    """

    def build_extension( self, ext ):
        if ext.name == _pysvn_soname:
            self._build_pysvn( ext )
        else:
            super( BuildExtensions, self ).build_extension( ext )

    def _build_pysvn( self, ext ):
        dest_dir = os.path.join( os.path.abspath( self.build_lib ), 'pysvn' )

        # Generate metadata first
        self.run_command( "egg_info" )

        if platform.system() == 'Darwin':
            # For Mac, figure out the major.minor version of the OS, and
            # pass that information to the build system.
            os.putenv( 'MACOSX_DEPLOYMENT_TARGET',
                       '.'.join( platform.mac_ver()[ 0 ].split( '.' )[ :2 ] ) )

        # Invoke the build system. This will generate the __init__.py and
        # .so that we'll package.
        os.chdir( 'Source' )
        os.system( sys.executable + ' setup.py configure' )
        os.system( 'make clean' )
        os.system( 'make' )

        # Copy the built files to the destination pysvn/ directory.
        self.mkpath( dest_dir )
        shutil.copy( os.path.join( 'pysvn', '__init__.py' ), dest_dir )
        shutil.copy( os.path.join( 'pysvn', '%s.so' % _pysvn_soname ),
                     dest_dir )

        # We're done. Restore the working directory.
        os.chdir( '..' )


setuptools.setup(
    name = name,
    version='%(MAJOR)s.%(MINOR)s.%(PATCH)s' % pysvn_version_info,
    author="Barry Scott",
    author_email="barryscott@tigris.org",
    description="Subversion support for Python",
    long_description="",
    url="http://pysvn.tigris.org/",
    license="Apache Software License",
    keywords="subversion",
    include_package_data=True,
    zip_safe=False,
    cmdclass={
        'build_ext': BuildExtensions,
    },
    ext_modules = [
        setuptools.Extension(
            _pysvn_soname, [])  # This used to tell setuptools that
                                # there is native extension, but
                                # they're not build using setuptools.
    ],
    classifiers=[
        "Topic :: Software Development :: Version Control",
    ],
)   
