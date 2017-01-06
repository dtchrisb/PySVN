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
//  pysvn_client_cmd_export.cpp
//
#if defined( _MSC_VER )
// disable warning C4786: symbol greater than 255 character,
// nessesary to ignore as <map> causes lots of warning
#pragma warning(disable: 4786)
#endif

#include "pysvn.hpp"
#include "pysvn_static_strings.hpp"

static const char *g_utf_8 = "utf-8";

Py::Object pysvn_client::cmd_export( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_src_url_or_path },
    { true,  name_dest_path },
    { false, name_force },
    { false, name_revision },
#if defined( PYSVN_HAS_CLIENT_EXPORT2 )
    { false, name_native_eol },
#endif
#if defined( PYSVN_HAS_CLIENT_EXPORT3 )
    { false, name_ignore_externals },
    { false, name_recurse },
    { false, name_peg_revision },
#endif
#if defined( PYSVN_HAS_CLIENT_EXPORT4 )
    { false, name_depth },
#endif
#if defined( PYSVN_HAS_CLIENT_EXPORT5 )
    { false, name_ignore_keywords },
#endif
    { false, NULL }
    };
    FunctionArguments args( "export", args_desc, a_args, a_kws );
    args.check();

    std::string src_path( args.getUtf8String( name_src_url_or_path ) );
    std::string dest_path( args.getUtf8String( name_dest_path ) );
    bool is_url = is_svn_url( src_path );

    bool force = args.getBoolean( name_force, false );
    svn_opt_revision_t revision;
    if( is_url )
         revision = args.getRevision( name_revision, svn_opt_revision_head );
    else
         revision = args.getRevision( name_revision, svn_opt_revision_working );

#if defined( PYSVN_HAS_CLIENT_EXPORT2 )
    const char *native_eol = NULL;
    if( args.hasArg( name_native_eol ) )
    {
        Py::Object native_eol_obj = args.getArg( name_native_eol );
        if( native_eol_obj != Py::None() )
        {
            Py::String eol_py_str( native_eol_obj );
            std::string eol_str = eol_py_str.as_std_string( g_utf_8 );
            if( eol_str == "CR" )
                native_eol = "CR";
            else if( eol_str == "CRLF" )
                native_eol = "CRLF";
            else if( eol_str == "LF" )
                native_eol = "LF";
            else
                throw Py::ValueError( "native_eol must be one of None, \"LF\", \"CRLF\" or \"CR\"" );
        }
    }
#endif
#if defined( PYSVN_HAS_CLIENT_EXPORT3 )
#if defined( PYSVN_HAS_CLIENT_EXPORT4 )
    svn_depth_t depth = args.getDepth( name_depth, name_recurse, svn_depth_infinity, svn_depth_infinity, svn_depth_files );
#else
    bool recurse = args.getBoolean( name_recurse, true );
#endif
    bool ignore_externals = args.getBoolean( name_ignore_externals, false );
    svn_opt_revision_t peg_revision = args.getRevision( name_peg_revision, revision );

    revisionKindCompatibleCheck( is_url, peg_revision, name_peg_revision, name_url_or_path );
#endif

#if defined( PYSVN_HAS_CLIENT_EXPORT5 )
    bool ignore_keywords = args.getBoolean( name_ignore_keywords, false );
#endif

    revisionKindCompatibleCheck( is_url, revision, name_revision, name_url_or_path );

    svn_revnum_t revnum = 0;

    SvnPool pool( m_context );

    try
    {
        std::string norm_src_path( svnNormalisedIfPath( src_path, pool ) );
        std::string norm_dest_path( svnNormalisedIfPath( dest_path, pool ) );

        checkThreadPermission();

        PythonAllowThreads permission( m_context );

#if defined( PYSVN_HAS_CLIENT_EXPORT5 )
        svn_error_t * error = svn_client_export5
            (
            &revnum,
            norm_src_path.c_str(),
            norm_dest_path.c_str(),
            &peg_revision,
            &revision,
            force,
            ignore_externals,
            ignore_keywords,
            depth,
            native_eol,
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_EXPORT4 )
        svn_error_t * error = svn_client_export4
            (
            &revnum,
            norm_src_path.c_str(),
            dest_path.c_str(),
            &peg_revision,
            &revision,
            force,
            ignore_externals,
            depth,
            native_eol,
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_EXPORT3 )
        svn_error_t * error = svn_client_export3
            (
            &revnum,
            norm_src_path.c_str(),
            dest_path.c_str(),
            &peg_revision,
            &revision,
            force,
            ignore_externals,
            recurse,
            native_eol,
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_EXPORT2 )
        svn_error_t * error = svn_client_export2
            (
            &revnum,
            norm_src_path.c_str(),
            dest_path.c_str(),
            &revision,
            force,
            native_eol,
            m_context,
            pool
            );
#else
        svn_error_t * error = svn_client_export
            (
            &revnum,
            norm_src_path.c_str(),
            dest_path.c_str(),
            &revision,
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

    return Py::asObject( new pysvn_revision( svn_opt_revision_number, 0, revnum ) );
}

Py::Object pysvn_client::cmd_import( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_path },
    { true,  name_url },
    { true,  name_log_message },
    { false, name_recurse },
#if defined( PYSVN_HAS_CLIENT_IMPORT2 )
    { false, name_ignore },
#endif
#if defined( PYSVN_HAS_CLIENT_IMPORT3 )
    { false, name_depth },
    { false, name_ignore_unknown_node_types },
    { false, name_revprops },
#endif
#if defined( PYSVN_HAS_CLIENT_IMPORT5 )
    { false, name_autoprops },
#endif
    { false, NULL }
    };
    FunctionArguments args( "import_", args_desc, a_args, a_kws );
    args.check();

    std::string path( args.getUtf8String( name_path ) );
    std::string url( args.getUtf8String( name_url ) );
    std::string message( args.getUtf8String( name_log_message ) );

    SvnPool pool( m_context );

#if defined( PYSVN_HAS_CLIENT_IMPORT3 )
    svn_depth_t depth = args.getDepth( name_depth, name_recurse, svn_depth_infinity, svn_depth_infinity, svn_depth_files );
    bool ignore_unknown_node_types = args.getBoolean( name_ignore_unknown_node_types, false );

    apr_hash_t *revprops = NULL;
    if( args.hasArg( name_revprops ) )
    {
        Py::Object py_revprop = args.getArg( name_revprops );
        if( !py_revprop.isNone() )
        {
            revprops = hashOfStringsFromDictOfStrings( py_revprop, pool );
        }
    }
#else
    bool recurse = args.getBoolean( name_recurse, true );
#endif
#if defined( PYSVN_HAS_CLIENT_IMPORT2 )
    bool ignore = args.getBoolean( name_ignore, false );
#endif

#if defined( PYSVN_HAS_CLIENT_IMPORT5 )
    bool autoprops = args.getBoolean( name_autoprops, true );
#endif

#if defined( PYSVN_HAS_CLIENT_IMPORT4 )
    CommitInfoResult commit_info( pool );
#else
    pysvn_commit_info_t *commit_info = NULL;
#endif

    try
    {
        std::string norm_path( svnNormalisedIfPath( path, pool ) );
        std::string norm_url( svnNormalisedUrl( url, pool ) );

        checkThreadPermission();

        PythonAllowThreads permission( m_context );

        m_context.setLogMessage( message.c_str() );

#if defined( PYSVN_HAS_CLIENT_IMPORT5 )
        svn_error_t *error = svn_client_import5
            (
            norm_path.c_str(),
            norm_url.c_str(),
            depth,
            !ignore,
            !autoprops,
            ignore_unknown_node_types,
            revprops,
            NULL,       // filter_callback
            NULL,       // filter_baton
            commit_info.callback(),
            commit_info.baton(),
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_IMPORT4 )
        svn_error_t *error = svn_client_import4
            (
            norm_path.c_str(),
            norm_url.c_str(),
            depth,
            !ignore,
            ignore_unknown_node_types,
            revprops,
            commit_info.callback(),
            commit_info.baton(),
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_IMPORT3 )
        svn_error_t *error = svn_client_import3
            (
            &commit_info,       // changed type
            norm_path.c_str(),
            norm_url.c_str(),
            depth,
            !ignore,
            ignore_unknown_node_types,
            revprops,
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_IMPORT2 )
        svn_error_t *error = svn_client_import2
            (
            &commit_info,       // changed type
            norm_path.c_str(),
            url.c_str(),
            !recurse,           // non_recursive
            !ignore,
            m_context,
            pool
            );
#else
        svn_error_t *error = svn_client_import
            (
            &commit_info,
            norm_path.c_str(),
            url.c_str(),
            !recurse,           // non_recursive
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

#if defined( PYSVN_HAS_CLIENT_IMPORT4 )
    return toObject( commit_info, m_wrapper_commit_info, m_commit_info_style );
#else
    return toObject( commit_info, m_commit_info_style );
#endif
}
