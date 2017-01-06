#
#   create__init__.py
#
import sys
import os

init_template = sys.argv[1]
init_output = sys.argv[2]
gen_tool = sys.argv[3]
module_name = sys.argv[4].split('.')[0]

print( 'Info: Creating __init__.py for module %s' % module_name )
pysvn__init__file_contents = open( init_template ).readlines()
block_begin_index = pysvn__init__file_contents.index( '### IMPORT BLOCK BEGIN\n' )
block_end_index = pysvn__init__file_contents.index( '### IMPORT BLOCK END\n' ) + 1

if sys.version_info[0] >= 3:
    module_name = 'pysvn.%s' % (module_name,)

replacement = [ '    import %s\n' % (module_name,) ]

if module_name != '_pysvn':
    replacement.append( '    _pysvn = %s\n' % (module_name,) )

pysvn__init__file_contents[ block_begin_index:block_end_index ] = replacement

f = open( init_output, 'w' )
f.write( ''.join( pysvn__init__file_contents ) )
f.close()

cmd_gen = '%s >>%s' % (gen_tool, init_output)
print( 'Info: Running %r' % cmd_gen )
os.system( cmd_gen )
