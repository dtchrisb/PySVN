import pysvn
import sys

c = pysvn.Client( sys.argv[1] )
c.exception_style = 1

c.add_to_changelist( 'file_a1.txt', 'changelist-one' )
c.add_to_changelist( 'file_a2.txt', 'changelist-two' )
c.add_to_changelist( 'file_b1.txt', 'changelist-one' )
c.add_to_changelist( 'file_b2.txt', 'changelist-two' )
print( 'After add_to_changelist show all' )
for path, changelist in sorted( c.get_changelist( '.' ) ):
    print( '   %s %s' % (changelist, path) )

print( 'After add_to_changelist show changelist-two' )
for path, changelist in sorted( c.get_changelist( '.', changelists=['changelist-two'] ) ):
    print( '   %s %s' % (changelist, path) )

c.remove_from_changelists( '.', changelists=['changelist-two'] )
print( 'After remove_from_changelists all changelist-two show all' )
for path, changelist in sorted( c.get_changelist( '.' ) ):
    print( '   %s %s' % (changelist, path) )

print( 'After remove_from_changelists all show all' )
c.remove_from_changelists( 'file_a1.txt' )
for path, changelist in sorted( c.get_changelist( '.' ) ):
    print( '   %s %s' % (changelist, path) )
