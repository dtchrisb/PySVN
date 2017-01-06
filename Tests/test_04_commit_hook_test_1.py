import sys
import pysvn

cmd = sys.argv[1]

print( 'Info: %s test 1' % (cmd,) )

if len(sys.argv) == 6 and sys.argv[5] == 'is_revision':
    # starting with svn 1.8 the transaction is in argv[4] but does not seem usable
    # continue to use the revision
    print( 'Info: Transaction( %s, %s, is_revision=True) ...' % (sys.argv[2], sys.argv[3]) )
    t = pysvn.Transaction( sys.argv[2], sys.argv[3], is_revision=True )

elif len(sys.argv) == 5 and sys.argv[4] == 'is_revision':
    # pre svn 1.8 interface
    print( 'Info: Transaction( %s, %s, is_revision=True) ...' % (sys.argv[2], sys.argv[3]) )
    t = pysvn.Transaction( sys.argv[2], sys.argv[3], is_revision=True )

else:
    print( 'Info: Transaction( %s, %s) ...' % (sys.argv[2], sys.argv[3]) )
    t = pysvn.Transaction( sys.argv[2], sys.argv[3] )

print( 'Info: revproplist() ...' )
all_props = t.revproplist()
for name, value in sorted( list( all_props.items() ) ):
    print( '%s: %s' % (name, value) )

print(  'Info: changed() ...' )
changes = t.changed()
change_list = list( changes.items() )
change_list.sort()
for name, (action, kind, text_mod, prop_mod) in change_list:
    print( '%s: action=%r, kind=%r, text_mod=%r, prop_mod=%r' % (name, action, kind, text_mod, prop_mod) )
    if action != 'D':
        all_props = t.proplist( name )
        for prop_name, prop_value in sorted( list( all_props.items() ) ):
            print( '     %s: %s' % (prop_name, prop_value) )
        if kind == pysvn.node_kind.file:
            print( '     contents: %r' % t.cat( name ) )

print( 'Info: changed( copy_info=True ) ...' )
changes = t.changed( copy_info=True )
change_list = list( changes.items() )
change_list.sort()
for name, (action, kind, text_mod, prop_mod, copyfrom_rev, copyfrom_path) in change_list:
    print( '%s: action=%r, kind=%r, text_mod=%r, prop_mod=%r copyfrom_rev=%r copyfrom_path=%r' %
            (name, action, kind, text_mod, prop_mod, copyfrom_rev, copyfrom_path) )
    if action != 'D':
        all_props = t.proplist( name )
        for prop_name, prop_value in sorted( list( all_props.items() ) ):
            print( '     %s: %s' % (prop_name, prop_value) )
        if kind == pysvn.node_kind.file:
            print( '     contents: %r' % t.cat( name ) )

print( 'Info: list() ...' )
def recursive_list( path ):
    entries = t.list( path )
    entry_list = list( entries.items() )
    entry_list.sort()
    for name, kind in entry_list:
        full = '%s/%s' % (path, name)
        print( '%s: kind=%r' % (full, kind) )
        if kind == pysvn.node_kind.dir:
            recursive_list( full )
recursive_list( '' )

sys.exit( 0 )
