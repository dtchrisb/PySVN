#
#    make_pkg.py
#
import os

print( 'Info: setup vesion info' )
import sys
sys.path.insert( 0, '../../Source')
import pysvn
import time
import glob

package_maker = '/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker'
package_maker_kit = os.path.exists( package_maker )

os.system( 'lipo -info %s >lipo-info.tmp' % (sys.executable,) )
lipo_info = open( 'lipo-info.tmp' ).read()
os.remove( 'lipo-info.tmp' )

is_x86_64 = 'x86_64' in lipo_info
is_i386 = 'i386' in lipo_info

if is_x86_64 and is_i386:
    processor = 'intel'

elif is_x86_64:
    processor = 'x86_64'

elif is_i386:
    processor = 'i386'

else:
    assert False, 'Unknown processor type'

python_vendor = os.environ.get( 'BUILDER_VENDOR', 'unknown' )

if processor == 'i386':
    if hasattr( sys, 'maxsize' ):
        maxsize = sys.maxsize
    else:
        maxsize = sys.maxint
    
    if maxsize == (2**31-1):
        processor = 'i386'
    else:
        processor = 'x86_64'

print( 'Info: Process is %s' % (processor,) )

pymaj, pymin, pypat, _, _ = sys.version_info
python_version_string = '%d.%d.%d' % (pymaj, pymin, pypat)
pysvnmaj, pysvnmin, pysvnpat, _ = pysvn.version
pysvn_version_string = '%d.%d.%d-%d' % (pysvn.version[0], pysvn.version[1], pysvn.version[2], pysvn.version[3])
pysvn_short_version_string = '%d.%d.%d' % (pysvn.version[0], pysvn.version[1], pysvn.version[2])
svn_version_package_string = '%d%d%d' % (pysvn.svn_version[0], pysvn.svn_version[1], pysvn.svn_version[2])
svn_version_string = '%d.%d.%d' % (pysvn.svn_version[0], pysvn.svn_version[1], pysvn.svn_version[2])
pysvn_so_string = '_pysvn_%d_%d.so' % (pymaj, pymin)
pkg_filename = 'py%s%s_%s_pysvn_svn%s-%s-%s' % (pymaj, pymin, python_vendor, svn_version_package_string, pysvn_version_string, processor)

print( 'Info: Packageing %s' % (pkg_filename,) )
build_time  = time.time()
build_time_str = time.strftime( '%d-%b-%Y %H:%M', time.localtime( build_time ) )
year = time.strftime( '%Y', time.localtime( build_time ) )
tmpdir = os.path.join( os.getcwd(), 'tmp' )

if pymaj == 2 and pymin == 7:
    if python_vendor == 'apple_com':
        install_dir = '/Library/Python/2.7/site-packages'
    else:
        install_dir = '/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/site-packages'

elif pymaj == 3 and pymin == 1:
    install_dir = '/Library/Frameworks/Python.framework/Versions/3.1/lib/python3.1/site-packages'

elif pymaj == 3 and pymin == 2:
    install_dir = '/Library/Frameworks/Python.framework/Versions/3.2/lib/python3.2/site-packages'

elif pymaj == 3 and pymin == 3:
    install_dir = '/Library/Frameworks/Python.framework/Versions/3.3/lib/python3.3/site-packages'

elif pymaj == 3 and pymin == 4:
    install_dir = '/Library/Frameworks/Python.framework/Versions/3.4/lib/python3.4/site-packages'

elif pymaj == 3 and pymin == 5:
    install_dir = '/Library/Frameworks/Python.framework/Versions/3.5/lib/python3.5/site-packages'

else:
    raise RuntimeError( 'Unsupported version of python' )

if os.path.exists( tmpdir ):
    print( 'Info: Clean up tmp directory' )
    os.system( 'rm -rf tmp' )

print( 'Info: Create directories' )

for kit_dir in [
    tmpdir,
    os.path.join( tmpdir, 'Resources' ),
    os.path.join( tmpdir, 'Contents' ),
    os.path.join( tmpdir, 'Contents/pysvn' ),
    os.path.join( tmpdir, pkg_filename),
    os.path.join( tmpdir, '%s/Examples' % pkg_filename ),
    os.path.join( tmpdir, '%s/Examples/Client' % pkg_filename ),
    os.path.join( tmpdir, '%s/Documentation' % pkg_filename),
    ]:
    if not os.path.exists( kit_dir ):
        os.makedirs( kit_dir )


print( 'Info: Finding dylibs used by pysvn' )

def findDylibs( image, dylib_list, depth=0 ):
    cmd = 'otool -L "%s" >/tmp/pysvn_otool.tmp' % image
    #print( 'Debug: cmd %r' % cmd )
    os.system( cmd )
    # always skip the first line that lists the image being dumped
    for line in open( '/tmp/pysvn_otool.tmp' ).readlines()[1:]:
        line = line.strip()
        libpath = line.split()[0]
        #print( 'Debug: line %r' % line )
        if( libpath.startswith( '/' )               # lines with libs on them
        and not libpath.startswith( '/usr/lib' )    # ignore libs shipped by Apple
        and not libpath.startswith( '/System' )     # ignore libs shipped by Apple
        and not libpath.endswith( '/Python' ) ):    # do not need to ignore python
            if libpath not in dylib_list:
                #print( 'Info: ',depth,' Need lib',libpath,'for',image )
                dylib_list.append( libpath )
                findDylibs( libpath, dylib_list, depth+1 )
        
dylib_list = []
findDylibs( '../../Source/pysvn/%s' % pysvn_so_string, dylib_list )

print( 'Info: Copy files' )

cp_list = [
    ('../../Source/pysvn/__init__.py',
        'Contents/pysvn'),
    ('../../Source/pysvn/%s' % pysvn_so_string,
        'Contents/pysvn'),
    ('../../LICENSE.txt',
        'Resources/License.txt'),
    ('../../LICENSE.txt',
        '%s/License.txt' % pkg_filename ),
    ('../../Docs/pysvn.html',
        '%s/Documentation' % pkg_filename ),
    ('../../Docs/pysvn_prog_ref.html',
        '%s/Documentation' % pkg_filename ),
    ('../../Docs/pysvn_prog_ref.js',
        '%s/Documentation' % pkg_filename ),
    ('../../Docs/pysvn_prog_guide.html',
        '%s/Documentation' % pkg_filename ),
    ('../../Examples/Client/svn_cmd.py',
        '%s/Examples/Client' % pkg_filename ),
    ('../../Examples/Client/parse_datetime.py',
        '%s/Examples/Client' % pkg_filename ),
    ]

for libpath in dylib_list:
    cp_list.append( (libpath, 'Contents/pysvn') )

for cp_src, cp_dst_dir_fmt in cp_list:
    cmd = 'cp -f "%s" "tmp/%s"' % (cp_src, cp_dst_dir_fmt % locals())
    print( 'Info: CMD %r' % cmd )
    os.system( cmd )

for dylib in glob.glob( 'tmp/Contents/pysvn/*.dylib' ):
    # all the dylib need to be writable
    cmd = 'chmod +w "%s"' % (dylib,)
    print( 'Info: CMD %r' % cmd )
    os.system( cmd )

print( 'Info: Fix the install paths for the dylib files' )

fixup_path_list = ['tmp/Contents/pysvn/%s' % pysvn_so_string]
for libpath in dylib_list:
    fixup_path_list.append( 'tmp/Contents/pysvn/' + os.path.basename( libpath ) )

for fixup_path in fixup_path_list:
    for libpath in dylib_list:
        if libpath != fixup_path:
            os.system( cmd )
            cmd = ('install_name_tool'
                ' -change'
                ' %s'
                ' %s/pysvn/%s'
                ' %s' %
                    (libpath, install_dir, os.path.basename( libpath ), fixup_path))
            print( 'Info: CMD %s' % (cmd,) )
            os.system( cmd )

for dylib in glob.glob( 'tmp/Contents/pysvn/*.dylib' ):
    # can now remove the +w
    cmd = 'chmod -w "%s"' % (dylib,)
    print( 'Info: CMD %r' % cmd )
    os.system( cmd )

if python_vendor == 'apple_com':
    readme_vendor_name = "Apple's"

elif python_vendor == 'python_org':
    readme_vendor_name = "Python.org's"

else:
    readme_vendor_name = python_vendor

print( 'Info: Create tmp/Resources/ReadMe.txt' )
if package_maker_kit:
    f = open( 'tmp/Resources/ReadMe.txt', 'w' )

else:
    f = open( 'tmp/%s/ReadMe.txt' % (pkg_filename,), 'w' )

f.write('''<html>
<body>
<h1>PySVN %(pysvn_version_string)s for Mac OS X, %(readme_vendor_name)s Python %(pymaj)s.%(pymin)s and Subversion %(svn_version_string)s</h1>

<h2>Copyright Barry A. Scott (c) 2003-%(year)s</h2>

<h2>Mail <a href="mailto:barry@barrys-emacs.org">barry@barrys-emacs.org</a></h2>

<h2>Pysvn home <a href="http://pysvn.tigris.org">http://pysvn.tigris.org</a></h2>

<h2>&#160;&#160;&#160;&#160;&#160;Barry Scott</h2>
</body>
</html>
''' % locals() )
f.close()

if package_maker_kit:
    print( 'Info: Create tmp/Info.plist' )
    f = open( 'tmp/Info.plist', 'w' )
    f.write('''<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleGetInfoString</key>
    <string>pysvn Extension %(pysvn_version_string)s for Python %(pymaj)s.%(pymin)s</string>
    <key>CFBundleIdentifier</key>
    <string>org.tigris.pysvn.extension.py%(pymaj)s.%(pymin)s.%(python_vendor)s</string>
    <key>CFBundleName</key>
    <string>pysvn Extension</string>
    <key>CFBundleShortVersionString</key>
    <string>%(pysvn_short_version_string)s</string>
    <key>IFMajorVersion</key>
    <integer>%(pysvnmaj)s</integer>
    <key>IFMinorVersion</key>
    <integer>%(pysvnmin)s</integer>
    <key>IFPkgFlagAllowBackRev</key>
    <false/>
    <key>IFPkgFlagAuthorizationAction</key>
    <string>AdminAuthorization</string>
    <key>IFPkgFlagDefaultLocation</key>
    <string>%(install_dir)s</string>
    <key>IFPkgFlagInstallFat</key>
    <false/>
    <key>IFPkgFlagIsRequired</key>
    <false/>
    <key>IFPkgFlagRelocatable</key>
    <false/>
    <key>IFPkgFlagRestartAction</key>
    <string>NoRestart</string>
    <key>IFPkgFlagRootVolumeOnly</key>
    <true/>
    <key>IFPkgFlagUpdateInstalledLanguages</key>
    <false/>
    <key>IFPkgFormatVersion</key>
    <real>0.10000000149011612</real>
</dict>
</plist>
''' % locals() )
    f.close()

    print( 'Info: Create tmp/Description.plist' )
    f = open( 'tmp/Description.plist', 'w' )
    f.write('''<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>IFPkgDescriptionDescription</key>
	<string>PySVN Extension
</string>
	<key>IFPkgDescriptionTitle</key>
	<string>PySVN Extension</string>
</dict>
</plist>
''' )
    f.close()

    print( 'Info: PackageMaker' )
    cmd = [ package_maker,
            '-build',
            '-p %s' % os.path.abspath( 'tmp/%s/%s.pkg' % (pkg_filename, pkg_filename) ),
            '-f %s' % os.path.abspath( 'tmp/Contents' ),
            '-r %s' % os.path.abspath( 'tmp/Resources' ),
            '-i %s' % os.path.abspath( 'tmp/Info.plist' ),
            '-d %s' % os.path.abspath( 'tmp/Description.plist' ),
            ]
    os.system( ' '.join( cmd ) )

    print( 'Info: Make Disk Image' )
    os.system( 'hdiutil create -srcfolder tmp/%s tmp/tmp.dmg' % pkg_filename )
    os.system( 'hdiutil convert tmp/tmp.dmg -format UDZO -imagekey zlib-level=9 ' 
            '-o tmp/%s.dmg' % pkg_filename )

else:
    print( 'Info: Create installation script' )
    f = open( 'tmp/%s/Install PySVN' % (pkg_filename,), 'w' )
    f.write( '''#!/bin/bash
    if [ "$( id -u )" != "0" ]
    then
        clear
        echo "To install PYSVN required root (administrator) privileges."
        echo "Enter your password to proceed."
        exec sudo "$0"
    fi

    kit_dir=$( dirname "$0" )

    echo "Installing pysvn Extension %(pysvn_version_string)s for Python %(pymaj)s.%(pymin)s"

    tar xzf "${kit_dir}/%(pkg_filename)s.tar.gz" -C "%(install_dir)s"

    echo "Installation complete. Press RETURN to exit."
    read A

''' % locals() )
    f.close()
    os.system( 'chmod +x "tmp/%s/Install PYSVN"' % (pkg_filename,) )

    cmd = [ 'tar',
            'czf',
            'tmp/%s/%s.tar.gz' % (pkg_filename, pkg_filename),
            '-C' 'tmp/Contents',
            'pysvn']

    os.system( ' '.join( cmd ) )

    print( 'Info: Make Disk Image' )
    os.system( 'hdiutil create -srcfolder tmp/%s tmp/tmp.dmg' % pkg_filename )
    os.system( 'hdiutil convert tmp/tmp.dmg -format UDZO -imagekey zlib-level=9 ' 
            '-o tmp/%s.dmg' % pkg_filename )
