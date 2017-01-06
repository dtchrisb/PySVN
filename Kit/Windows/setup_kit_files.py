import sys
import time
import os
import datetime

def main( argv ):
    print( 'Info: setup_kit_files.py' )
    inno = InnoSetup( argv )
    return inno.createInnoInstall()

class InnoSetup:
    def __init__( self, argv ):

        self.arch = sys.argv[1]
        self.vc_ver = sys.argv[2]

        sys.path.insert( 0, r'..\..\Source')
        import pysvn

        self.py_maj = sys.version_info[0]
        self.py_min = sys.version_info[1]

        self.python_version_string = '%d.%d.%d' % (self.py_maj, self.py_min, sys.version_info[2])
        self.pysvn_version_string = '%d.%d.%d-%d' % (pysvn.version[0], pysvn.version[1], pysvn.version[2], pysvn.version[3])
        self.svn_version_package_string = '%d%d%d' % (pysvn.svn_version[0], pysvn.svn_version[1], pysvn.svn_version[2])
        self.svn_version_string = '%d.%d.%d' % (pysvn.svn_version[0], pysvn.svn_version[1], pysvn.svn_version[2])

        self.build_time  = time.time()
        self.build_time_str = time.strftime( '%d-%b-%Y %H:%M', time.localtime( self.build_time ) )

        self.year = datetime.datetime.now().year

        self.all_code_items = []
        self.all_setup_items = []
        self.all_file_items = []
        self.all_icon_items = []
        self.all_run_items = []

    def createInnoInstall( self ):
        self.setupInnoItems()
        self.generateInnoFile()

    def setupInnoItems( self ):
        print( 'Info: Create info_before.txt' )

        f = open( r'tmp\info_before.txt', 'w' )
        f.write(
'''PySVN %s for %s Python %s and Subversion %s

    Barry Scott

    %s

''' % (self.pysvn_version_string, self.arch, self.python_version_string, self.svn_version_string, self.build_time_str) )
        f.close()

        print( 'Info: Create setup_copy.cmd' )
        f = open( r'tmp\setup_copy.cmd', 'w' )
        f.write( r'copy tmp\Output\setup.exe tmp\Output\py%d%d-pysvn-svn%s-%s-%s.exe' '\n' %
                    (self.py_maj, self.py_min, self.svn_version_package_string, self.pysvn_version_string, self.arch) )
        f.close()

        self.all_setup_items.extend( [
                'AppName=Python %(py_maj)d.%(py_min)d PySVN for %(arch)s' % self.__dict__,
                'AppVerName=Python %(py_maj)d.%(py_min)d PySVN %(pysvn_version_string)s on %(arch)s' % self.__dict__,
                'AppCopyright=Copyright (C) 2003-%(year)s Barry A. Scott' % self.__dict__,
                r'DefaultDirName={code:pythondir}\lib\site-packages\pysvn',
                'DefaultGroupName=PySVN for Python %(py_maj)d.%(py_min)d on %(arch)s' % self.__dict__,
                'DisableStartupPrompt=yes',
                'InfoBeforeFile=info_before.txt',
                'Compression=bzip/9',
                ] )

        self.all_icon_items.extend( [
                r'Name: "{group}\PySVN Documentation"; Filename: "{app}\pysvn.html";',
                r'Name: "{group}\PySVN License"; Filename: "{app}\pysvn_LICENSE.txt";',
                r'Name: "{group}\PySVN Web Site"; Filename: "http://pysvn.tigris.org";',
                ] )

        self.all_file_items.extend( [
                r'Source: "..\..\..\Source\pysvn\__init__.py"; DestDir: "{app}";',
                r'Source: "..\..\..\Source\pysvn\_pysvn_%(py_maj)d_%(py_min)d.pyd"; DestDir: "{app}"; Flags: ignoreversion;' % self.__dict__,
                r'Source: "..\..\..\Docs\pysvn.html"; DestDir: "{app}";',
                r'Source: "..\..\..\Docs\pysvn_prog_guide.html"; DestDir: "{app}";',
                r'Source: "..\..\..\Docs\pysvn_prog_ref.html"; DestDir: "{app}";',
                r'Source: "..\..\..\Docs\pysvn_prog_ref.js"; DestDir: "{app}";',
                r'Source: "LICENSE.txt"; DestDir: "{app}";',
                r'',
                r'Source: "..\..\..\Examples\Client\svn_cmd.py"; DestDir: "{app}\Examples\Client";',
                r'Source: "..\..\..\Examples\Client\parse_datetime.py"; DestDir: "{app}\Examples\Client";',
                ] )

        for dll in [dll for dll in os.listdir( 'tmp' ) if dll.lower().endswith( '.dll' )]:
            self.all_file_items.append( 'Source: "%s"; DestDir: "{app}"; Flags: ignoreversion' % (dll,) )

        if self.vc_ver == '9.0':
            redist_year = '2008'

        elif self.vc_ver == '10.0':
            redist_year = '2010'

        elif self.vc_ver == '14.0':
            redist_year = '2015'

        else:
            print( 'Error: Unsupported VC_VER of %s' % (self.vc_ver,) )
            return 1

        if self.arch == 'Win32':
            redist_arch = 'x86'
            code_file = 'pysvn_win32_code.iss'

        elif self.arch == 'Win64':
            redist_arch = 'x64'
            code_file = 'pysvn_win32_code.iss'
            self.all_setup_items.append( 'ArchitecturesAllowed=x64' )
            self.all_setup_items.append( 'ArchitecturesInstallIn64BitMode=x64' )

        else:
            print( 'Error: Unsupported ARCH of %s' % (self.arch,) )
            return 1

        f = open( code_file, 'r' )
        self.all_code_items.append( f.read() % self.__dict__ )
        f.close()

        redist_file = 'vcredist_%s_%s.exe' % (redist_arch, redist_year)

        os.system( r'copy k:\subversion\%s tmp' % (redist_file,) )

        self.all_file_items.append( 'Source: "%s"; DestDir: {tmp}; Flags: deleteafterinstall' %
                                    (redist_file,) )
        self.all_run_items.append( r'Filename: {tmp}\%s; Parameters: "/q"; StatusMsg: Installing VC++ %s %s Redistributables...' %
                                    (redist_file, redist_year, self.arch) )

    def generateInnoFile( self ):
        inno_file = r'tmp\pysvn.iss'
        print( 'Info: Generating %s' % (inno_file,) )
        f = open( inno_file, 'w' )

        f.write( '[Code]\n' )
        for item in self.all_code_items:
            f.write( item )
            f.write( '\n' )

        f.write( '[Setup]\n' )
        for item in self.all_setup_items:
            f.write( item )
            f.write( '\n' )

        f.write( '[Files]\n' )
        for item in self.all_file_items:
            f.write( item )
            f.write( '\n' )

        f.write( '[Icons]\n' )
        for item in self.all_icon_items:
            f.write( item )
            f.write( '\n' )

        f.write( '[Run]\n' )
        for item in self.all_run_items:
            f.write( item )
            f.write( '\n' )

        f.close()

if __name__ == '__main__':
    sys.exit( main( sys.argv ) )
