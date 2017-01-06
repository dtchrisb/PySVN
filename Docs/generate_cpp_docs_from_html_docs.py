#!/usr/bin/python
import sys
import os
import xml.dom.minidom

def main( argv ):
    svn_include = argv[1]
    html_doc = argv[2]
    cpp_header_filename = argv[3]
    cpp_module_filename = argv[4]

    print( 'Info: svn_include %s' % svn_include )
    print( 'Info: html_doc %s' % html_doc )
    print( 'Info: cpp_header_filename %s' % cpp_header_filename )
    print( 'Info: cpp_module_filename %s' % cpp_module_filename )

    debug.enable( '--debug' in argv )
    try:
        parser = XhtmlParser( open( html_doc ).read() )
    except ParseException as e:
        print( repr(e) )
        print( str(e) )
        return 1

    parser.setSvnVersion( svn_include )
    parser.htmlToCpp()

    cpp_header = open( cpp_header_filename, 'w' )
    cpp_module = open( cpp_module_filename, 'w' )

    cpp_header.write( '// Created by %s\n\n' % argv[0] )
    cpp_module.write( '// Created by %s\n' % argv[0] )
    cpp_module.write( '#include "pysvn_docs.hpp"\n' )

    parser.writeCppDocs( cpp_header, cpp_module )

    cpp_header.close()
    cpp_module.close()

    return 0

class DebugTrace:
    def __init__( self, enable=False ):
        self._enable = enable

    def __call__( self, *args ):
        if self.isEnabled():
            for s in args:
                print( s, )
            print

    def enable( self, enable ):
        self._enable = enable

    def isEnabled( self ):
        return self._enable

debug = DebugTrace()

class ParseException(Exception):
    def __init__(self, msg):
        Exception.__init__(self)
        self.msg = msg

    def __repr__( self ):
        return '<ParseException: %s>' % self.msg

    def __str__( self ):
        return self.msg


class XhtmlParser:
    def __init__( self, definition_text ):
        self.dom = None
        self.all_docs = {}
        self.svn_version = None

        try:
            self.dom = xml.dom.minidom.parseString( definition_text )

        except xml.parsers.expat.ExpatError as e:
            raise ParseException( 'Ln:%d Col:%d %s' % (e.lineno, e.offset, e.args[0]) )

    def htmlToCpp( self ):
        all_anchors = self.dom.getElementsByTagName("a")
        all_named_nodes = [(node.parentNode, node.getAttribute( 'name' ))
                                for node in all_anchors
                                    if node.hasAttribute( 'name' )]

        for node, name in all_named_nodes:
            self.docsFromNode( node, name )

        copyright = ''' Copyright (c) 2003-2007 Barry A Scott.  All rights reserved.

 This software is licensed as described in the file LICENSE.txt,
 which you should have received as part of this distribution.

 =============================================================

 This product includes software developed by
 CollabNet (http://www.Collab.Net/).
'''

        docs = Documentation( 'copyright' )
        docs.addText( copyright )
        self.all_docs[ docs.getName() ] = docs

    def writeCppDocs( self, cpp_header, cpp_module ):
        all_names = list( self.all_docs.keys() )
        all_names.sort()
        for name in all_names:
            docs = self.all_docs[ name ]
            cpp_header.write( 'extern const char %s_doc[];\n' % name )
            cpp_module.write( '\nconst char %s_doc[] =\n' % name )
            for line in docs.getBody().split('\n' ):
                cpp_module.write( '   "%s\\n"\n' % line )
            cpp_module.write( ';\n'  )

    def docsFromNode( self, node, name ):
        docs = Documentation( name )
        while True:
            node = node.nextSibling
            if node is None:
                break

            if node.nodeName in ['h1','h2','h3','h4']:
                break

            self.__extractText( None, docs, node )

        debug( 'docsFromNode adding %s' % name )
        self.all_docs[ name ] = docs

    def __extractText( self, parent, docs, node ):
        debug( '__extractText( node=',repr(node), ')' )
        if node.nodeName == 'p':
            docs.addText( '\n' )
        if node.nodeName == 'li':
            docs.addText( '* ' )

        for child in node.childNodes:
            debug('__extractText child nodeName=%s' % child.nodeName)

            if child.nodeType == xml.dom.minidom.Node.TEXT_NODE:
                debug( '__extractText TEXT_NODE: %r %r' % (node, child.data) )
                if node.nodeName in ['p','li','span']:
                    docs.addText( child.data.strip() )
                elif node.nodeName in ['a']:
                    docs.addText( '%s ' % child.data.strip() )
                elif node.nodeName in ['pre']:
                    docs.addText( child.data )

            elif child.nodeType == xml.dom.minidom.Node.DOCUMENT_TYPE_NODE:
                #debug( '__extractText DOCUMENT_TYPE_NODE' )
                pass

            elif child.nodeType == xml.dom.minidom.Node.PROCESSING_INSTRUCTION_NODE:
                #debug( '__extractText PROCESSING_INSTRUCTION_NODE' )
                pass

            elif child.nodeType == xml.dom.minidom.Node.COMMENT_NODE:
                #debug( '__extractText COMMENT_NODE' )
                pass

            elif child.nodeType == xml.dom.minidom.Node.ELEMENT_NODE:
                debug( '__extractText ELEMENT_NODE %r' % child.nodeName )

                if child.nodeName in ['h1','h2','h3','h4']:
                    break

                include_children = True
                # check for conditional span and div sections
                if child.nodeName in ['span','div']:
                    version_class = child.getAttribute( 'class' )
                    if version_class.startswith( 'svn_' ):
                        encoded_version = int( version_class[len('svn_'):] )
                        if debug.isEnabled():
                            docs.addText( '[if %d]' % encoded_version )
                        if encoded_version > self.svn_version:
                            debug( 'Skipping %r > %r' % (encoded_version, self.svn_version) )
                            include_children = False
                        else:
                            debug( 'Including %r <= %r' % (encoded_version, self.svn_version) )

                if include_children:
                    self.__extractText( node, docs, child )

        if node.nodeName == 'p':
            docs.addText( '\n' )
        if node.nodeName == 'li':
            docs.addText( '\n' )

        debug('__extractText done')
        return

    def setSvnVersion( self, svn_include ):
        all_svn_version_lines = open( os.path.join( svn_include, 'svn_version.h' ) ).readlines()
        major = None
        minor = None
        patch = None
        for line in all_svn_version_lines:
            words = line.strip().split()
            if len(words) > 2 and words[0] == '#define':
                if words[1] == 'SVN_VER_MAJOR':
                    major = int(words[2])
                elif words[1] == 'SVN_VER_MINOR':
                    minor = int(words[2])
                elif words[1] == 'SVN_VER_PATCH':
                    patch = int(words[2])
 
        self.svn_version = ((major * 1000) + minor) * 1000 + patch
        # force to the version to test with
        #self.svn_version = 1001000
        print( 'Info: Building against SVN %d.%d.%d code %d' % (major, minor, patch, self.svn_version) )

class Documentation:
    def __init__( self, name ):
        self.name = name
        self.body = []

    def addText( self, text ):
        self.body.append( text )

    def getName( self ):
        return self.name

    def getBody( self ):
        body = ''.join( self.body ).strip()
        body = body.replace( '\\', '\\\\' )
        body = body.replace( '"', '\\"' )
        return body

if __name__ == "__main__":
    sys.exit( main( sys.argv ) )
