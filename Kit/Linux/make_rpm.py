#
#    make_rpm.py
#
import os

print 'Info: setup_version_handling.py'
import sys
sys.path.insert( 0, '../../Source')
import pysvn
import time

pymaj, pymin, pypat, _, _ = sys.version_info
python_version_string = '%d.%d.%d' % (pymaj, pymin, pypat)
pysvnmaj, pysvnmin, pysvnpat, _ = pysvn.version
pysvn_version_string = '%d.%d.%d-%d' % (pysvn.version[0], pysvn.version[1], pysvn.version[2], pysvn.version[3])
pysvn_version_package_release_string = '%d' % pysvn.version[3]
pysvn_version_package_string = '%d.%d.%d' % (pysvn.version[0], pysvn.version[1], pysvn.version[2])
svn_version_string = '%d.%d.%d' % (pysvn.svn_version[0], pysvn.svn_version[1], pysvn.svn_version[2])
svn_compact_version_string = '%d%d%d' % (pysvn.svn_version[0], pysvn.svn_version[1], pysvn.svn_version[2])

build_time  = time.time()
build_time_str = time.strftime( '%d-%b-%Y %H:%M', time.localtime( build_time ) )

tmpdir = os.path.join( os.getcwd(), 'tmp' )
if os.path.exists( tmpdir ):
    print 'Info: Clean up tmp directory'
    os.system( 'rm -rf tmp' )

print 'Info: Create directories'

for kit_dir in [
    tmpdir,
    os.path.join( tmpdir, 'ROOT' ),
    os.path.join( tmpdir, 'BUILD' ),
    os.path.join( tmpdir, 'SPECS' ),
    os.path.join( tmpdir, 'RPMS' ),
    os.path.join( tmpdir, 'ROOT/usr' ),
    os.path.join( tmpdir, 'ROOT/usr/lib' ),
    os.path.join( tmpdir, 'ROOT/usr/lib/python%(pymaj)d.%(pymin)d' % locals() ),
    os.path.join( tmpdir, 'ROOT/usr/lib/python%(pymaj)d.%(pymin)d/site-packages' % locals() ),
    os.path.join( tmpdir, 'ROOT/usr/lib/python%(pymaj)d.%(pymin)d/site-packages/pysvn' % locals() ),
    os.path.join( tmpdir, 'ROOT/usr/share' ),
    os.path.join( tmpdir, 'ROOT/usr/share/doc' ),
    os.path.join( tmpdir, 'ROOT/usr/share/doc/pysvn' ),
    os.path.join( tmpdir, 'ROOT/usr/share/doc/pysvn/Examples' ),
    os.path.join( tmpdir, 'ROOT/usr/share/doc/pysvn/Examples/Client' ),
    ]:
    if not os.path.exists( kit_dir ):
        os.makedirs( kit_dir )

print 'Info: Copy files'
kit_files_info = [
    ('../../Source/pysvn/__init__.py',
        'ROOT/usr/lib/python%(pymaj)d.%(pymin)d/site-packages/pysvn', '444'),
    ('../../Source/pysvn/_pysvn_%(pymaj)d_%(pymin)d.so' % locals(),
        'ROOT/usr/lib/python%(pymaj)d.%(pymin)d/site-packages/pysvn', '444'),
    ('../../LICENSE.txt',
        'ROOT/usr/share/doc/pysvn', '444'),
    ('../../Docs/pysvn.html',
        'ROOT/usr/share/doc/pysvn', '444'),
    ('../../Docs/pysvn_prog_ref.html',
        'ROOT/usr/share/doc/pysvn', '444'),
    ('../../Docs/pysvn_prog_ref.js',
        'ROOT/usr/share/doc/pysvn', '444'),
    ('../../Docs/pysvn_prog_guide.html',
        'ROOT/usr/share/doc/pysvn', '444'),
    ('../../Examples/Client/svn_cmd.py',
        'ROOT/usr/share/doc/pysvn/Examples/Client', '555'),
    ('../../Examples/Client/parse_datetime.py',
        'ROOT/usr/share/doc/pysvn/Examples/Client', '444'),
    ]

for cp_src, cp_dst_dir_fmt, perm in kit_files_info:
    print 'Info:  cp %s' % cp_src
    os.system( 'cp -f %s tmp/%s' % (cp_src, cp_dst_dir_fmt % locals()) )

print 'Info: Create tmp/SPECS/pysvn.spec'
f = file('tmp/SPECS/pysvn.spec','w')
f.write('''BuildRoot:    %(tmpdir)s/ROOT
Name:        py%(pymaj)d%(pymin)d_pysvn_svn%(svn_compact_version_string)s
Version:    %(pysvn_version_package_string)s
Group:        Development/Libraries
Release:    %(pysvn_version_package_release_string)s
Summary:    pysvn %(pysvn_version_package_string)s Python extension for Subversion %(svn_version_string)s
License:    Apache Software License, Version 1.1 - Copyright Barry A. Scott (c) 2003-2009
Packager:    Barry A. Scott <barry@barrys-emacs.org>
%%description
PySVN %(pysvn_version_string)s for Python %(python_version_string)s and Subversion %(svn_version_string)s

Copyright Barry A. Scott (c) 2003-2009

mailto:barry@barrys-emacs.org
http://pysvn.tigris.org

     Barry Scott

%%prep
%%build
%%install
%%post
/usr/bin/python%(pymaj)d.%(pymin)d -c "import pysvn"
/usr/bin/python%(pymaj)d.%(pymin)d -O -c "import pysvn"
%%postun
rm -f /usr/lib/python%(pymaj)d.%(pymin)d/site-packages/pysvn/__init__.pyc
rm -f /usr/lib/python%(pymaj)d.%(pymin)d/site-packages/pysvn/__init__.pyo
%%files
%%defattr (-,root,root)
''' % locals() )

kit_filename = os.path.join( cp_dst_dir_fmt[len('ROOT'):] % locals(), os.path.basename( cp_src ) )
for cp_src, cp_dst_dir_fmt, perm in kit_files_info:
    kit_filename = os.path.join( cp_dst_dir_fmt[len('ROOT'):] % locals(), os.path.basename( cp_src ) )
    f.write( '%%attr(%s,root,root) %s\n' % (perm, kit_filename) )
    
f.close()

print 'Info: Create rpmrc'
os.system('grep ^macrofiles: /usr/lib/rpm/rpmrc |sed -e s!~/.rpmmacros!%(tmpdir)s/rpmmacros! >%(tmpdir)s/rpmrc' % locals() )
print 'Info: Create rpmmacros'
f = file( 'tmp/rpmmacros', 'w' )
f.write( '%%_topdir %(tmpdir)s' % locals() )
f.close()
print 'Info: rpmbuild'
os.system( 'rpmbuild --rcfile=/usr/lib/rpm/rpmrc:%(tmpdir)s/rpmrc -bb %(tmpdir)s/SPECS/pysvn.spec' % locals() )
