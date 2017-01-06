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
//  pysvn_client_cmd_merge.cpp
//
#if defined( _MSC_VER )
// disable warning C4786: symbol greater than 255 character,
// nessesary to ignore as <map> causes lots of warning
#pragma warning(disable: 4786)
#endif

#include "pysvn.hpp"
#include "pysvn_svnenv.hpp"
#include "svn_path.h"
#include "svn_config.h"
#include "svn_sorts.h"
#include "pysvn_static_strings.hpp"

static const char *g_utf_8 = "utf-8";

Py::Object pysvn_client::cmd_merge( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_url_or_path1 },
    { true,  name_revision1 },
    { true,  name_url_or_path2 },
    { true,  name_revision2 },
    { true,  name_local_path },
    { false, name_force },
    { false, name_recurse },
    { false, name_notice_ancestry },
    { false, name_dry_run },
#if defined( PYSVN_HAS_CLIENT_MERGE2 )
    { false, name_merge_options },
#endif
#if defined( PYSVN_HAS_CLIENT_MERGE3 )
    { false, name_depth },
    { false, name_record_only },
#endif
#if defined( PYSVN_HAS_CLIENT_MERGE4 )
    { false, name_allow_mixed_revisions },
#endif
#if defined( PYSVN_HAS_CLIENT_MERGE5 )
    { false, name_ignore_mergeinfo },
#endif
    { false, NULL }
    };
    FunctionArguments args( "merge", args_desc, a_args, a_kws );
    args.check();

    std::string path1( args.getUtf8String( name_url_or_path1 ) );
    svn_opt_revision_t revision1 = args.getRevision( name_revision1, svn_opt_revision_head );
    std::string path2( args.getUtf8String( name_url_or_path2 ) );
    svn_opt_revision_t revision2 = args.getRevision( name_revision2, svn_opt_revision_head );
    std::string local_path( args.getUtf8String( name_local_path ) );

    bool force = args.getBoolean( name_force, false );
#if defined( PYSVN_HAS_CLIENT_MERGE3 )
    svn_depth_t depth = args.getDepth( name_depth, name_recurse, svn_depth_infinity, svn_depth_infinity, svn_depth_files );
    bool record_only = args.getBoolean( name_record_only, false );
#else
    bool recurse = args.getBoolean( name_recurse, true );
#endif
    bool notice_ancestry = args.getBoolean( name_notice_ancestry, false );
    bool dry_run = args.getBoolean( name_dry_run, false );

#if defined( PYSVN_HAS_CLIENT_MERGE4 )
    bool allow_mixed_revisions = args.getBoolean( name_allow_mixed_revisions, false );
#endif
#if defined( PYSVN_HAS_CLIENT_MERGE5 )
    bool ignore_mergeinfo = args.getBoolean( name_ignore_mergeinfo, !notice_ancestry );
#endif

#if defined( PYSVN_HAS_CLIENT_MERGE2 )
    Py::List merge_options_list;
    if( args.hasArg( name_merge_options ) )
    {
        merge_options_list = args.getArg( name_merge_options );
        for( size_t i=0; i<merge_options_list.length(); i++ )
        {
            Py::String check_is_string( merge_options_list[i] );
        }
    }
#endif

    SvnPool pool( m_context );

#if defined( PYSVN_HAS_CLIENT_MERGE2 )
    apr_array_header_t *merge_options = NULL;
    if( merge_options_list.length() > 0 )
    {
        merge_options = apr_array_make( pool, merge_options_list.length(), sizeof( const char * ) );
        for( size_t i=0; i<merge_options_list.length(); i++ )
        {
            Py::String py_option( merge_options_list[i] );
            std::string option( py_option.as_std_string( g_utf_8 ) );
            
            *((const char **) apr_array_push(merge_options)) = apr_pstrdup( pool, option.c_str() );
        }
    }
#endif

    try
    {
        std::string norm_path1( svnNormalisedIfPath( path1, pool ) );
        std::string norm_path2( svnNormalisedIfPath( path2, pool ) );
        std::string norm_local_path( svnNormalisedIfPath( local_path, pool ) );

        checkThreadPermission();

        PythonAllowThreads permission( m_context );

#if defined( PYSVN_HAS_CLIENT_MERGE5 )
        svn_error_t *error = svn_client_merge5
            (
            norm_path1.c_str(),
            &revision1,
            norm_path2.c_str(),
            &revision2,
            norm_local_path.c_str(),
            depth,
            ignore_mergeinfo,
            !notice_ancestry,
            force,      // force_delete
            record_only,
            dry_run,
            allow_mixed_revisions,
            merge_options,
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_MERGE4 )
        svn_error_t *error = svn_client_merge4
            (
            norm_path1.c_str(),
            &revision1,
            norm_path2.c_str(),
            &revision2,
            norm_local_path.c_str(),
            depth,
            !notice_ancestry,
            force,
            record_only,
            dry_run,
            allow_mixed_revisions,
            merge_options,
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_MERGE3 )
        svn_error_t *error = svn_client_merge3
            (
            norm_path1.c_str(),
            &revision1,
            norm_path2.c_str(),
            &revision2,
            norm_local_path.c_str(),
            depth,
            !notice_ancestry,
            force,
            record_only,
            dry_run,
            merge_options,
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_MERGE2 )
        svn_error_t *error = svn_client_merge2
            (
            norm_path1.c_str(),
            &revision1,
            norm_path2.c_str(),
            &revision2,
            norm_local_path.c_str(),
            recurse,
            !notice_ancestry,
            force,
            dry_run,
            merge_options,
            m_context,
            pool
            );
#else
        svn_error_t *error = svn_client_merge
            (
            norm_path1.c_str(),
            &revision1,
            norm_path2.c_str(),
            &revision2,
            norm_local_path.c_str(),
            recurse,
            !notice_ancestry,
            force,
            dry_run,
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

    return Py::None();
}

#if defined( PYSVN_HAS_CLIENT_MERGE_PEG3 )
Py::Object pysvn_client::cmd_merge_peg2( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_sources },
    { true,  name_ranges_to_merge },
    { true,  name_peg_revision },
    { true,  name_target_wcpath },
    { false, name_depth },
    { false, name_notice_ancestry },
    { false, name_force },
    { false, name_record_only },
    { false, name_dry_run },
    { false, name_merge_options },
#if defined( PYSVN_HAS_CLIENT_MERGE_PEG4 )
    { false, name_allow_mixed_revisions },
#endif
#if defined( PYSVN_HAS_CLIENT_MERGE_PEG4 )
    { false, name_ignore_mergeinfo },
#endif
    { false, NULL }
    };
    FunctionArguments args( "merge_peg2", args_desc, a_args, a_kws );
    args.check();

    std::string sources( args.getUtf8String( name_sources ) );
    svn_opt_revision_t peg_revision = args.getRevision( name_peg_revision );
    std::string target_wcpath( args.getUtf8String( name_target_wcpath ) );
    bool force = args.getBoolean( name_force, false );
    svn_depth_t depth = args.getDepth( name_depth, svn_depth_infinity );
    bool record_only = args.getBoolean( name_record_only, true );
    bool notice_ancestry = args.getBoolean( name_notice_ancestry, false );
    bool dry_run = args.getBoolean( name_dry_run, false );
#if defined( PYSVN_HAS_CLIENT_MERGE4 )
    bool allow_mixed_revisions = args.getBoolean( name_allow_mixed_revisions, false );
#endif
#if defined( PYSVN_HAS_CLIENT_MERGE5 )
    bool ignore_mergeinfo = args.getBoolean( name_ignore_mergeinfo, false );
#endif

    Py::List merge_options_list;
    if( args.hasArg( name_merge_options ) )
    {
        merge_options_list = args.getArg( name_merge_options );
        for( size_t i=0; i<merge_options_list.length(); i++ )
        {
            Py::String check_is_string( merge_options_list[i] );
        }
    }

    bool is_url = is_svn_url( sources );
    revisionKindCompatibleCheck( is_url, peg_revision, name_peg_revision, name_url_or_path );

    SvnPool pool( m_context );

    apr_array_header_t *merge_options = NULL;
    if( merge_options_list.length() > 0 )
    {
        merge_options = apr_array_make( pool, merge_options_list.length(), sizeof( const char * ) );
        for( size_t i=0; i<merge_options_list.length(); i++ )
        {
            Py::String py_option( merge_options_list[i] );
            std::string option( py_option.as_std_string( g_utf_8 ) );
            
            *((const char **) apr_array_push(merge_options)) = apr_pstrdup( pool, option.c_str() );
        }
    }

    Py::List list_all_ranges = args.getArg( name_ranges_to_merge );

    apr_array_header_t *ranges_to_merge =
        apr_array_make( pool, list_all_ranges.length(), sizeof(svn_opt_revision_range_t *) );

    for( unsigned int index=0; index<list_all_ranges.length(); index++ )
    {
        Py::Tuple tuple_range( list_all_ranges[ index ] );

        svn_opt_revision_range_t *range = reinterpret_cast<svn_opt_revision_range_t *>( apr_palloc( pool, sizeof(*range) ) );

        if( tuple_range.length() != 2 )
        {
            std::string msg = "merge_peg2() expecting tuple with 2 values in ranges_to_merge list";
            throw Py::AttributeError( msg );
        }

        {
        Py::Object obj( tuple_range[0] );

        if( pysvn_revision::check( obj ) )
        {
            pysvn_revision *rev = static_cast<pysvn_revision *>( obj.ptr() );
            range->start = rev->getSvnRevision();
            revisionKindCompatibleCheck( is_url, range->start, name_ranges_to_merge, name_sources );
        }
        else
        {
            std::string msg = "merge_peg2() expecting revision for 1st tuple value in sources list";
            throw Py::AttributeError( msg );
        }
        }

        {
        Py::Object obj( tuple_range[1] );

        if( pysvn_revision::check( obj ) )
        {
            pysvn_revision *rev = static_cast<pysvn_revision *>( obj.ptr() );
            range->end = rev->getSvnRevision();
            revisionKindCompatibleCheck( is_url, range->end, name_ranges_to_merge, name_sources );
        }
        else
        {
            std::string msg = "merge_peg2() expecting revision for 2nd tuple value in sources list";
            throw Py::AttributeError( msg );
        }
        }


        APR_ARRAY_PUSH( ranges_to_merge, svn_opt_revision_range_t *) = range;
    }

    try
    {
        std::string norm_sources( svnNormalisedIfPath( sources, pool ) );
        std::string norm_target_wcpath( svnNormalisedIfPath( target_wcpath, pool ) );

        checkThreadPermission();

        PythonAllowThreads permission( m_context );

#if defined( PYSVN_HAS_CLIENT_MERGE_PEG5 )
        svn_error_t *error = svn_client_merge_peg5
            (
            norm_sources.c_str(),
            ranges_to_merge,
            &peg_revision,
            norm_target_wcpath.c_str(),
            depth,
            ignore_mergeinfo,
            !notice_ancestry,   // diff_ignore_ancestry
            force,              // force_delete
            record_only,
            dry_run,
            allow_mixed_revisions,
            merge_options,
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_MERGE_PEG4 )
        svn_error_t *error = svn_client_merge_peg4
            (
            norm_sources.c_str(),
            ranges_to_merge,
            &peg_revision,
            norm_target_wcpath.c_str(),
            depth,
            !notice_ancestry,
            force,      // force_delete
            record_only,
            dry_run,
            allow_mixed_revisions,
            merge_options,
            m_context,
            pool
            );
#else
        svn_error_t *error = svn_client_merge_peg3
            (
            norm_sources.c_str(),
            ranges_to_merge,
            &peg_revision,
            norm_target_wcpath.c_str(),
            depth,
            !notice_ancestry,
            force,
            record_only,
            dry_run,
            merge_options,
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

    return Py::None();
}
#endif
#if defined( PYSVN_HAS_CLIENT_MERGE_PEG )
Py::Object pysvn_client::cmd_merge_peg( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_url_or_path },
    { true,  name_revision1 },
    { true,  name_revision2 },
    { true,  name_peg_revision },
    { true,  name_local_path },
    { false, name_recurse },
    { false, name_notice_ancestry },
    { false, name_force },
    { false, name_dry_run },
#if defined( PYSVN_HAS_CLIENT_MERGE_PEG2 )
    { false, name_merge_options },
#endif
    { false, NULL }
    };
    FunctionArguments args( "merge_peg", args_desc, a_args, a_kws );
    args.check();

    std::string path( args.getUtf8String( name_url_or_path ) );
    svn_opt_revision_t revision1 = args.getRevision( name_revision1, svn_opt_revision_head );
    svn_opt_revision_t revision2 = args.getRevision( name_revision2, svn_opt_revision_head );
    svn_opt_revision_t peg_revision = args.getRevision( name_revision2, revision2 );
    std::string local_path( args.getUtf8String( name_local_path ) );

    bool force = args.getBoolean( name_force, false );
    bool recurse = args.getBoolean( name_recurse, true );
    bool notice_ancestry = args.getBoolean( name_notice_ancestry, false );
    bool dry_run = args.getBoolean( name_dry_run, false );

#if defined( PYSVN_HAS_CLIENT_MERGE_PEG2 )
    Py::List merge_options_list;
    if( args.hasArg( name_merge_options ) )
    {
        merge_options_list = args.getArg( name_merge_options );
        for( size_t i=0; i<merge_options_list.length(); i++ )
        {
            Py::String check_is_string( merge_options_list[i] );
        }
    }
#endif

    bool is_url = is_svn_url( path );
    revisionKindCompatibleCheck( is_url, peg_revision, name_peg_revision, name_url_or_path );
    revisionKindCompatibleCheck( is_url, revision1, name_revision1, name_url_or_path );
    revisionKindCompatibleCheck( is_url, revision2, name_revision2, name_url_or_path );

    SvnPool pool( m_context );

#if defined( PYSVN_HAS_CLIENT_MERGE_PEG2 )
    apr_array_header_t *merge_options = NULL;
    if( merge_options_list.length() > 0 )
    {
        merge_options = apr_array_make( pool, merge_options_list.length(), sizeof( const char * ) );
        for( size_t i=0; i<merge_options_list.length(); i++ )
        {
            Py::String py_option( merge_options_list[i] );
            std::string option( py_option.as_std_string( g_utf_8 ) );
            
            *((const char **) apr_array_push(merge_options)) = apr_pstrdup( pool, option.c_str() );
        }
    }
#endif

    try
    {
        std::string norm_path( svnNormalisedIfPath( path, pool ) );
        std::string norm_local_path( svnNormalisedIfPath( local_path, pool ) );

        checkThreadPermission();

        PythonAllowThreads permission( m_context );

#if defined( PYSVN_HAS_CLIENT_MERGE_PEG2 )
        svn_error_t *error = svn_client_merge_peg2 // ignore deprecation warning - support old Client().merge_peg()
            (
            norm_path.c_str(),
            &revision1,
            &revision2,
            &peg_revision,
            norm_local_path.c_str(),
            recurse,
            !notice_ancestry,
            force,
            dry_run,
            merge_options,
            m_context,
            pool
            );
#else
        svn_error_t *error = svn_client_merge_peg
            (
            norm_path.c_str(),
            &revision1,
            &revision2,
            &peg_revision,
            norm_local_path.c_str(),
            recurse,
            !notice_ancestry,
            force,
            dry_run,
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

    return Py::None();
}
#endif


#if defined( PYSVN_HAS_CLIENT_MERGE_REINTEGRATE )
Py::Object pysvn_client::cmd_merge_reintegrate( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_url_or_path },
    { true,  name_revision },
    { true,  name_local_path },
    { false, name_dry_run },
    { false, name_merge_options },
    { false, NULL }
    };
    FunctionArguments args( "merge", args_desc, a_args, a_kws );
    args.check();

    std::string path( args.getUtf8String( name_url_or_path ) );
    svn_opt_revision_t revision = args.getRevision( name_revision, svn_opt_revision_head );
    std::string local_path( args.getUtf8String( name_local_path ) );

    bool dry_run = args.getBoolean( name_dry_run, false );

    Py::List merge_options_list;
    if( args.hasArg( name_merge_options ) )
    {
        merge_options_list = args.getArg( name_merge_options );
        for( size_t i=0; i<merge_options_list.length(); i++ )
        {
            Py::String check_is_string( merge_options_list[i] );
        }
    }

    SvnPool pool( m_context );

    apr_array_header_t *merge_options = NULL;
    if( merge_options_list.length() > 0 )
    {
        merge_options = apr_array_make( pool, merge_options_list.length(), sizeof( const char * ) );
        for( size_t i=0; i<merge_options_list.length(); i++ )
        {
            Py::String py_option( merge_options_list[i] );
            std::string option( py_option.as_std_string( g_utf_8 ) );
            
            *((const char **) apr_array_push(merge_options)) = apr_pstrdup( pool, option.c_str() );
        }
    }

    try
    {
        std::string norm_path( svnNormalisedIfPath( path, pool ) );
        std::string norm_local_path( svnNormalisedIfPath( local_path, pool ) );

        checkThreadPermission();

        PythonAllowThreads permission( m_context );

        svn_error_t *error = svn_client_merge_reintegrate // ignore deprecation warning - support old Client().merge_reintegrate()
            (
            norm_path.c_str(),
            &revision,
            norm_local_path.c_str(),
            dry_run,
            merge_options,
            m_context,
            pool
            );
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

    return Py::None();
}
#endif
