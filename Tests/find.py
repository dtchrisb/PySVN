#
#   find.py
#
#   It turns out that Mac OS X and linux sort do not use the same
#   ordering. Python allows for a portable and reliable.
#
import os
import sys

all_files = []

def walk( dirname, all_files ):
    for filename in os.listdir( dirname ):
        if filename not in ['.svn']:
            filename = os.path.join( dirname, filename )
            if os.path.isdir( filename ):    
                walk( filename, all_files )
            else:
                all_files.append( filename )

walk( sys.argv[1], all_files )

all_files.sort()
for filename in all_files:
    print( filename )
