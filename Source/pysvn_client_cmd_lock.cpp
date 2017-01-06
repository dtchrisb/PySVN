//
// ====================================================================
// Copyright (c) 2003-2009 Barry A Scott.  All rights reserved.
//
// This software is licensed as described in the file LICENSE.txt,
// which you should have received as part of this distribution.
//
// ====================================================================
//
//
//  pysvn_client_cmd_list.cpp
//
#if defined( _MSC_VER )
// disable warning C4786: symbol greater than 255 character,
// nessesary to ignore as <map> causes lots of warning
#pragma warning(disable: 4786)
#endif

#include "pysvn.hpp"
#include "pysvn_svnenv.hpp"
#include "pysvn_static_strings.hpp"


#if defined( PYSVN_HAS_CLIENT_LOCK )
Py::Object pysvn_client::cmd_lock( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_url_or_path },
    { true,  name_comment },
    { false, name_force },
    { false, NULL }
    };
    FunctionArguments args( "lock", args_desc, a_args, a_kws );
    args.check();

    SvnPool pool( m_context );

    apr_array_header_t *targets = targetsFromStringOrList( args.getArg( name_url_or_path ), pool );

    std::string type_error_message;
    try
    {
        type_error_message = "expecting string for comment (arg 2)";
        std::string comment( args.getUtf8String( name_comment ) );

        type_error_message = "expecting boolean for force keyword arg";
        bool force = args.getBoolean( name_force, false );

        try
        {
            checkThreadPermission();

            PythonAllowThreads permission( m_context );

            svn_error_t *error = svn_client_lock
                (
                targets,
                comment.c_str(),
                force,        // non recursive
                m_context,
                pool
                );
            permission.allowThisThread();
            if( error != NULL )
                throw SvnException( error );
        }
        catch( SvnException &e )
        {
            // use callback error over ClientException
            m_context.checkForError( m_module.client_error );

            throw_client_error( e );
        }
    }
    catch( Py::TypeError & )
    {
        throw Py::TypeError( type_error_message );
    }

    return Py::None();
}
#endif

#if defined( PYSVN_HAS_CLIENT_LOCK )
Py::Object pysvn_client::cmd_unlock( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_url_or_path },
    { false, name_force },
    { false, NULL }
    };
    FunctionArguments args( "unlock", args_desc, a_args, a_kws );
    args.check();

    SvnPool pool( m_context );

    apr_array_header_t *targets = targetsFromStringOrList( args.getArg( name_url_or_path ), pool );

    std::string type_error_message;
    try
    {
        type_error_message = "expecting boolean for force keyword arg";
        bool force = args.getBoolean( name_force, true );

        try
        {
            checkThreadPermission();

            PythonAllowThreads permission( m_context );

            svn_error_t *error = svn_client_unlock
                (
                targets,
                force,
                m_context,
                pool
                );
            permission.allowThisThread();
            if( error != NULL )
                throw SvnException( error );
        }
        catch( SvnException &e )
        {
            // use callback error over ClientException
            m_context.checkForError( m_module.client_error );

            throw_client_error( e );
        }
    }
    catch( Py::TypeError & )
    {
        throw Py::TypeError( type_error_message );
    }

    return Py::None();
}
#endif

