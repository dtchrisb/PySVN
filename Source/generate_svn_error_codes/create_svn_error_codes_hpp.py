#
#   create_svn_error_codes_hpp.py
#
import sys
import os

svn_include = sys.argv[1]

f = open( 'generate_svn_error_codes.hpp', 'w' )
svn_err_file = open( os.path.join( svn_include, 'svn_error_codes.h' ), 'r' )


emit = False
for line in svn_err_file:
    if line == 'SVN_ERROR_START\n':
        emit = True

    if emit and 'SVN_ERRDEF(' in line:
        symbol = line.split( 'SVN_ERRDEF(' )[1].split( ',' )[0]
        f.write( '    printf( "    %s = %%d\\n", %s );\n' % (symbol.lower()[len('SVN_ERR_'):], symbol) )

f.close()
