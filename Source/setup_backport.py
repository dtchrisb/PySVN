#
#   backport_sources.py
#
#   fixup the python sources to work with python2.5 and earlier
#
import os
import sys

def backportRequired():
    try:
        import setup_backport_probe
        return False

    except SyntaxError:
        return True

def cmd_help( argv ):
    progname = os.path.basename( argv[0] )
    print( '''    Backport the PySVN sources to work with python 2.5 and earlier

        python %(progname)s backport [<pysvn-folder>]
''' % {'progname': progname} )

def cmd_backport( argv ):
    if len(argv) > 2:
        pysvn_root = argv[2]
    else:
        pysvn_root = '..'

    if os.path.exists( os.path.join( pysvn_root, '.svn' ) ):
        print( 'Error: Cannot apply backport command to a subversion working copy - becuase of risk of backported files being committed.' )
        return 1

    all_files = []

    walk( pysvn_root, all_files )

    all_files.sort()

    backported = 0
    for filename in all_files:
        if fixup_source( filename ):
            backported += 1

    if backported == 0:
        print( 'Warning: Of %d candidate files no files required backporting' % len(all_files) )
    else:
        print( 'Info: Backported source for %d files' % backported )

    return 0

def walk( dirname, all_files ):
    for filename in os.listdir( dirname ):
        filename = os.path.join( dirname, filename )

        if os.path.isdir( filename ):    
            walk( filename, all_files )

        elif filename.endswith( '.py' ):
            all_files.append( filename )

        elif filename.endswith( '.py.template' ):
            all_files.append( filename )

def fixup_source( filename ):
    f = open( filename, 'r' )
    all_lines = f.readlines()
    f.close()

    replaced = False

    for index in range( len( all_lines ) ):
        if( all_lines[ index ].strip().startswith( 'except ' )
        and ' as e:' in all_lines[ index ] ):
            replaced = True
            print( 'Info: processing %s' % filename )
            print( 'Info: Before: %r' % all_lines[ index ] )
            all_lines[ index ] = all_lines[ index ].replace( ' as e:', ', e:' )
            print( 'Info:  After: %r' % all_lines[ index ] )

    if replaced:
        print( 'Info: Updating %s' % filename )
        f = open( filename, 'w' )
        f.write( ''.join( all_lines ) )
        f.close()

    return replaced
