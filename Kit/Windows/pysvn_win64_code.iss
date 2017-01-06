function InitializeSetup(): Boolean;
var
    install_path : string;
    rcb : Boolean;
begin
    rcb := RegQueryStringValue( HKLM,
        'SOFTWARE\Python\PythonCore\%(py_maj)d.%(py_min)d\InstallPath',
        '', install_path );
    if not rcb then
    begin
        rcb := RegQueryStringValue( HKCU,
            'SOFTWARE\Python\PythonCore\%(py_maj)d.%(py_min)d\InstallPath', '', install_path );
        if not rcb then
        begin
            MsgBox( 'pysvn requires Win64 Python %(py_maj)d.%(py_min)d to be installed.' #13 #13
                    'Quitting installation',
                 mbError, MB_OK );
        end;
    end;
    Result := rcb;
end;

function pythondir(Default: String): String; 
var
    install_path : string;
    rcb : Boolean;
begin
    rcb := RegQueryStringValue( HKLM,
        'SOFTWARE\Python\PythonCore\%(py_maj)d.%(py_min)d\InstallPath',
        '', install_path );
    if rcb then
    begin
        Result := install_path;
    end
    else
        begin
            rcb := RegQueryStringValue( HKCU,
                'SOFTWARE\Python\PythonCore\%(py_maj)d.%(py_min)d\InstallPath',
                '', install_path );
            if rcb then
            begin
                Result := install_path;
            end
            else
            begin
                Result := 'c:\python%(py_maj)d.%(py_min)d';
            end;
        end;
    end;
end;
