import pysvn
import sys

c = pysvn.Client( sys.argv[1] )
c.exception_style = 1

src_list = [
    ('file_a1.txt',),
    ('file_a2.txt',),
    ]
print( 'Info: Copy2 with no revision and no peg_revision' )
c.copy2( src_list, 'wc_copy_1', make_parents=True, copy_as_child=True )

src_list = [
    ('file_a1.txt', pysvn.Revision( pysvn.opt_revision_kind.number, 4 )),
    ('file_a2.txt', pysvn.Revision( pysvn.opt_revision_kind.number, 4 )),
    ]
print( 'Info: Copy2 with no peg_revision' )
c.copy2( src_list, 'wc_copy_2', make_parents=True, copy_as_child=True )

src_list = [
    ('file_a1.txt', pysvn.Revision( pysvn.opt_revision_kind.number, 4 ), pysvn.Revision( pysvn.opt_revision_kind.number, 4 )),
    ('file_a2.txt', pysvn.Revision( pysvn.opt_revision_kind.number, 4 ), pysvn.Revision( pysvn.opt_revision_kind.number, 4 )),
    ]
print( 'Info: Copy2' )
c.copy2( src_list, 'wc_copy_3', make_parents=True, copy_as_child=True )

print( 'Info: Copy2 will raise error' )
try:
    c.copy2( src_list, 'wc_copy_3', make_parents=True )

except pysvn.ClientError as e:
    print( 'Info: Error: %r' % e.args[0] )
    for message, code in e.args[1]:
        print( 'Info: Code: %d, Message: %r' % (code, message) )
