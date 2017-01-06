import sys
import os
import re

version_details = sys.argv[1]
input_filename = sys.argv[2]
output_filename = input_filename[:-len('.template')]

# create dictionary of branding strings
branding_info = {}

for line in open( version_details ):
    line = line.strip()
    if len(line) == 0:
        continue
    if line[0:1] == ['#']:
        continue

    key, value = [s.strip() for s in line.split('=',1)]
    branding_info[ key ] = value

svnversion_image = os.environ.get( 'WC_SVNVERSION', 'svnversion' )
if ' ' in svnversion_image:
    cmd = ('"%s" -c "%s" 2>&1' %
        (svnversion_image
        ,os.environ.get( 'PYSVN_EXPORTED_FROM', '..' )))
else:
    cmd = ('%s -c "%s" 2>&1' %
        (svnversion_image
        ,os.environ.get( 'PYSVN_EXPORTED_FROM', '..' )))

print( 'Info: Running %s' % cmd )
build_revision = os.popen( cmd, 'r' ).read().strip()

# build_revision is either a range nnn:mmm or mmm
# we only want the mmm
build_revision = build_revision.split(':')[-1]
print( 'Info: revision %s' % build_revision )

if build_revision[0] not in '0123456789':
    branding_info['BUILD'] = '0'
else:
    revision, modifiers = re.compile( '(\d+)(.*)' ).search( build_revision ).groups()

    if modifiers:
        branding_info['BUILD'] = '0'
    else:
        branding_info['BUILD'] = revision


# read all the input text
text = open( input_filename, 'r' ).read()
# and write of a branded version
open( output_filename, 'w' ).write( text % branding_info )

sys.exit(0)
