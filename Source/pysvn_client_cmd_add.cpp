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
//  pysvn_client_cmd_prop.cpp
//
#if defined( _MSC_VER )
// disable warning C4786: symbol greater than 255 character,
// nessesary to ignore as <map> causes lots of warning
#pragma warning(disable: 4786)
#endif

#include "pysvn.hpp"
#include "pysvn_svnenv.hpp"
#include "pysvn_static_strings.hpp"

Py::Object pysvn_client::cmd_add( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_path },
    { false, name_recurse },
#if defined( PYSVN_HAS_CLIENT_ADD2 )
    { false, name_force },
#endif
#if defined( PYSVN_HAS_CLIENT_ADD3 )
    { false, name_ignore },
#endif
#if defined( PYSVN_HAS_CLIENT_ADD4 )
    { false, name_depth },
    { false, name_add_parents },
#endif
#if defined( PYSVN_HAS_CLIENT_ADD5 )
    { false, name_autoprops },
#endif

    { false, NULL }
    };
    FunctionArguments args( "add", args_desc, a_args, a_kws );
    args.check();

    Py::List path_list( toListOfStrings( args.getArg( name_path ) ) );

#if defined( PYSVN_HAS_CLIENT_ADD2 )
    bool force = args.getBoolean( name_force, false );
#endif
#if defined( PYSVN_HAS_CLIENT_ADD3 )
    bool ignore = args.getBoolean( name_ignore, true );
#endif
#if defined( PYSVN_HAS_CLIENT_ADD4 )
    svn_depth_t depth = args.getDepth( name_depth, name_recurse, svn_depth_infinity, svn_depth_infinity, svn_depth_empty );
    bool add_parents = args.getBoolean( name_add_parents, false );
#else
    bool recurse = args.getBoolean( name_recurse, true );
#endif
#if defined( PYSVN_HAS_CLIENT_ADD5 )
    bool autoprops = args.getBoolean( name_autoprops, true );
#endif

    SvnPool pool( m_context );
    try
    {
        for( Py::List::size_type i=0; i<path_list.length(); i++ )
        {
            Py::Bytes path_str( asUtf8Bytes( path_list[i] ) );
            std::string norm_path( svnNormalisedIfPath( path_str.as_std_string(), pool ) );

            checkThreadPermission();

            PythonAllowThreads permission( m_context );

            SvnPool pool( m_context );

#if defined( PYSVN_HAS_CLIENT_ADD5 )
            svn_error_t * error = svn_client_add5
                (
                norm_path.c_str(),
                depth,
                force,
                !ignore,
                !autoprops,
                add_parents,
                m_context,
                pool
                );
#elif defined( PYSVN_HAS_CLIENT_ADD4 )
            svn_error_t * error = svn_client_add4
                (
                norm_path.c_str(),
                depth,
                force,
                !ignore,
                add_parents,
                m_context,
                pool
                );
#elif defined( PYSVN_HAS_CLIENT_ADD3 )
            svn_error_t * error = svn_client_add3
                (
                norm_path.c_str(),
                recurse,
                force,
                !ignore,
                m_context,
                pool
                );
#elif defined( PYSVN_HAS_CLIENT_ADD2 )
            svn_error_t * error = svn_client_add2
                (
                norm_path.c_str(),
                recurse,
                force,
                m_context,
                pool
                );
#else
            svn_error_t * error = svn_client_add
                (
                norm_path.c_str(),
                recurse,
                m_context,
                pool
                );
#endif
            permission.allowThisThread();
            if( error != NULL )
                throw SvnException( error );
        }
    }
    catch( SvnException &e )
    {
        // use callback error over ClientException
        m_context.checkForError( m_module.client_error );

        throw_client_error( e );
    }

    return Py::None();
}

Py::Object pysvn_client::cmd_cat( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_url_or_path },
    { false, name_revision },
#if defined( PYSVN_HAS_CLIENT_CAT2 )
    { false, name_peg_revision },
#endif
#if defined( PYSVN_HAS_CLIENT_CAT3 )
    { false, name_expand_keywords },
    { false, name_get_props },
#endif
    { false, NULL }
    };
    FunctionArguments args( "cat", args_desc, a_args, a_kws );
    args.check();

    std::string path( args.getUtf8String( name_url_or_path ) );
    svn_opt_revision_t revision = args.getRevision( name_revision, svn_opt_revision_head );
#if defined( PYSVN_HAS_CLIENT_CAT2 )
    svn_opt_revision_t peg_revision = args.getRevision( name_peg_revision, revision );

#endif
    bool is_url = is_svn_url( path );
#if defined( PYSVN_HAS_CLIENT_CAT2 )
    revisionKindCompatibleCheck( is_url, peg_revision, name_peg_revision, name_url_or_path );
#endif
    revisionKindCompatibleCheck( is_url, revision, name_revision, name_url_or_path );

    SvnPool pool( m_context );

    svn_stringbuf_t * stringbuf = svn_stringbuf_create( empty_string, pool );
    svn_stream_t * stream = svn_stream_from_stringbuf( stringbuf, pool );

#if defined( PYSVN_HAS_CLIENT_CAT3 )
    bool get_props = args.getBoolean( name_get_props, false );
    bool expand_keywords = args.getBoolean( name_expand_keywords, false );

    apr_hash_t *props = NULL;
    apr_hash_t **props_ptr;
    if( get_props )
    {
        props_ptr = &props;
    }
    else
    {
        props_ptr = NULL;
    }
#endif

    try
    {
        std::string norm_path( svnNormalisedIfPath( path, pool ) );

        checkThreadPermission();

        PythonAllowThreads permission( m_context );
#if defined( PYSVN_HAS_CLIENT_CAT3 )
        svn_error_t *error = svn_client_cat3
            (
            props_ptr,
            stream,
            norm_path.c_str(),
            &peg_revision,
            &revision,
            expand_keywords,
            m_context,
            pool,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_CAT2 )
        svn_error_t *error = svn_client_cat2
            (
            stream,
            norm_path.c_str(),
            &peg_revision,
            &revision,
            m_context,
            pool
            );
#else
        svn_error_t *error = svn_client_cat
            (
            stream,
            norm_path.c_str(),
            &revision,
            m_context,
            pool
            );
#endif

        permission.allowThisThread();
        if (error != 0)
            throw SvnException (error);
    }
    catch( SvnException &e )
    {
        // use callback error over ClientException
        m_context.checkForError( m_module.client_error );

        throw_client_error( e );
    }

    // return the bytes as is to the application
    // we can assume nothing about them
    Py::Bytes contents( stringbuf->data, (int)stringbuf->len );
#if defined( PYSVN_HAS_CLIENT_CAT3 )
    if( get_props )
    {
        Py::Tuple result( 2 );

        result[0] = contents;
        result[1] = propsToObject( props, pool );

        return result;
    }
    else
#endif
    {
        return contents;
    }
}

Py::Object pysvn_client::cmd_mkdir( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_url_or_path },
    { false, name_log_message },
#if defined( PYSVN_HAS_CLIENT_MKDIR3 )
    { false, name_make_parents },
    { false, name_revprops },
#endif
    { false, NULL }
    };
    FunctionArguments args( "mkdir", args_desc, a_args, a_kws );
    args.check();

    // message that explains to the user the type error that may be thrown next
    std::string type_error_message;

    // args to the mkdir call
    std::string message;
    bool have_message = false;

    SvnPool pool( m_context );

    apr_array_header_t *targets = targetsFromStringOrList( args.getArg( name_url_or_path ), pool );

#if defined( PYSVN_HAS_CLIENT_MKDIR3 )
    bool make_parents = args.getBoolean( name_make_parents, false );

    apr_hash_t *revprops = NULL;
    if( args.hasArg( name_revprops ) )
    {
        Py::Object py_revprop = args.getArg( name_revprops );
        if( !py_revprop.isNone() )
        {
            revprops = hashOfStringsFromDictOfStrings( py_revprop, pool );
        }
    }
#endif

    try
    {
        type_error_message = "expecting string message (arg 2)";
        if( args.hasArg( name_log_message ) )
        {
            message = args.getUtf8String( name_log_message );
            have_message = true;
        }
    }
    catch( Py::TypeError & )
    {
        throw Py::TypeError( type_error_message );
    }


#if defined( PYSVN_HAS_CLIENT_MKDIR4 )
    CommitInfoResult commit_info( pool );
#else
    pysvn_commit_info_t *commit_info = NULL;
#endif

    try
    {
        checkThreadPermission();

        PythonAllowThreads permission( m_context );

        if( have_message )
        {
            m_context.setLogMessage( message.c_str() );
        }

#if defined( PYSVN_HAS_CLIENT_MKDIR4 )
        svn_error_t *error = svn_client_mkdir4
            (
            targets,
            make_parents,
            revprops,
            commit_info.callback(),
            commit_info.baton(),
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_MKDIR3 )
        svn_error_t *error = svn_client_mkdir3
            (
            &commit_info,       // changed type
            targets,
            make_parents,
            revprops,
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_MKDIR2 )
        svn_error_t *error = svn_client_mkdir2
            (
            &commit_info,       // changed type
            targets,
            m_context,
            pool
            );
#else
        svn_error_t *error = svn_client_mkdir
            (
            &commit_info,
            targets,
            m_context,
            pool
            );
#endif
        permission.allowThisThread();
        if( error != 0 )
            throw SvnException( error );
    }
    catch( SvnException &e )
    {
        // use callback error over ClientException
        m_context.checkForError( m_module.client_error );

        throw_client_error( e );
    }

#if defined( PYSVN_HAS_CLIENT_MKDIR4 )
    return toObject( commit_info, m_wrapper_commit_info, m_commit_info_style );
#else
    return toObject( commit_info, m_commit_info_style );
#endif
}

Py::Object pysvn_client::cmd_remove( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_url_or_path },
    { false, name_force },
#if defined( PYSVN_HAS_CLIENT_DELETE3 )
    { false, name_keep_local },
    { false, name_revprops },
#endif
    { false, NULL }
    };
    FunctionArguments args( "remove", args_desc, a_args, a_kws );
    args.check();

    SvnPool pool( m_context );

    bool force = args.getBoolean( name_force, false );
#if defined( PYSVN_HAS_CLIENT_DELETE3 )

    bool keep_local = args.getBoolean( name_keep_local, false );

    apr_hash_t *revprops = NULL;
    if( args.hasArg( name_revprops ) )
    {
        Py::Object py_revprop = args.getArg( name_revprops );
        if( !py_revprop.isNone() )
        {
            revprops = hashOfStringsFromDictOfStrings( py_revprop, pool );
        }
    }
#endif

    apr_array_header_t *targets = targetsFromStringOrList( args.getArg( name_url_or_path ), pool );

#if defined( PYSVN_HAS_CLIENT_MKDIR4 )
    CommitInfoResult commit_info( pool );
#else
    pysvn_commit_info_t *commit_info = NULL;
#endif

    try
    {
        checkThreadPermission();

        PythonAllowThreads permission( m_context );

#if defined( PYSVN_HAS_CLIENT_DELETE4 )
        svn_error_t *error = svn_client_delete4
            (
            targets,
            force,
            keep_local,
            revprops,
            commit_info.callback(),
            commit_info.baton(),
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_DELETE3 )
        svn_error_t *error = svn_client_delete3
            (
            &commit_info,
            targets,
            force,
            keep_local,
            revprops,
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_DELETE2 )
        svn_error_t *error = svn_client_delete2
            (
            &commit_info,       // commit_info changed
            targets,
            force,
            m_context,
            pool
            );
#else
        svn_error_t *error = svn_client_delete
            (
            &commit_info,
            targets,
            force,
            m_context,
            pool
            );
#endif
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

#if defined( PYSVN_HAS_CLIENT_MKDIR4 )
    return toObject( commit_info, m_wrapper_commit_info, m_commit_info_style );
#else
    return toObject( commit_info, m_commit_info_style );
#endif
}

Py::Object pysvn_client::cmd_revert( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_path },
    { false, name_recurse },
#if defined( PYSVN_HAS_CLIENT_REVERT2 )
    { false, name_depth },
    { false, name_changelists },
#endif
#if defined( PYSVN_HAS_CLIENT_REVERT3 )
    { false, name_clear_changelists },
    { false, name_metadata_only },
#endif
    { false, NULL }
    };
    FunctionArguments args( "revert", args_desc, a_args, a_kws );
    args.check();

    std::string type_error_message;

    SvnPool pool( m_context );
    apr_array_header_t *targets = targetsFromStringOrList( args.getArg( name_path ), pool );

    try
    {
#if defined( PYSVN_HAS_CLIENT_REVERT2 )
        apr_array_header_t *changelists = NULL;

        if( args.hasArg( name_changelists ) )
        {
            changelists = arrayOfStringsFromListOfStrings( args.getArg( name_changelists ), pool );
        }
        svn_depth_t depth = args.getDepth( name_depth, name_recurse, svn_depth_empty, svn_depth_infinity, svn_depth_empty );
#else
        bool recurse = args.getBoolean( name_recurse, false );
#endif

#if defined( PYSVN_HAS_CLIENT_REVERT3 )
    bool clear_changelists = args.getBoolean( name_clear_changelists, false );
    bool metadata_only = args.getBoolean( name_metadata_only, false );
#endif
        try
        {
            checkThreadPermission();

            PythonAllowThreads permission( m_context );

#if defined( PYSVN_HAS_CLIENT_REVERT3 )
            svn_error_t *error = svn_client_revert3
                (
                targets,
                depth,
                changelists,
                clear_changelists,
                metadata_only,
                m_context,
                pool
                );
#elif defined( PYSVN_HAS_CLIENT_REVERT2 )
            svn_error_t *error = svn_client_revert2
                (
                targets,
                depth,
                changelists,
                m_context,
                pool
                );
#else
            svn_error_t *error = svn_client_revert
                (
                targets,
                recurse,
                m_context,
                pool
                );
#endif
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

