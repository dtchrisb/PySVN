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

Py::Object pysvn_client::cmd_checkin( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_path },
    { true,  name_log_message },
    { false, name_recurse },
#if defined( PYSVN_HAS_CLIENT_COMMIT2 )
    { false, name_keep_locks },
#endif
#if defined( PYSVN_HAS_CLIENT_COMMIT4 )
    { false, name_depth },
    { false, name_keep_changelist },
    { false, name_changelists },
    { false, name_revprops },
#endif
#if defined( PYSVN_HAS_CLIENT_COMMIT5 )
    { false, name_commit_as_operations },
#endif
#if defined( PYSVN_HAS_CLIENT_COMMIT6 )
    { false, name_include_file_externals },
    { false, name_include_dir_externals },
#endif
    { false, NULL }
    };
    FunctionArguments args( "checkin", args_desc, a_args, a_kws );
    args.check();

    SvnPool pool( m_context );
#if defined( PYSVN_HAS_CLIENT_COMMIT5 )
    CommitInfoResult commit_info( pool );
#else
    pysvn_commit_info_t *commit_info = NULL;
#endif

    apr_array_header_t *targets = targetsFromStringOrList( args.getArg( name_path ), pool );

    std::string type_error_message;
    try
    {
        type_error_message = "expecting string for message (arg 2)";
        std::string message( args.getUtf8String( name_log_message ) );

#ifndef PYSVN_HAS_CLIENT_COMMIT4
        type_error_message = "expecting boolean for recurse keyword arg";
        bool recurse = args.getBoolean( name_recurse, true );
#endif

#if defined( PYSVN_HAS_CLIENT_COMMIT2 )
        type_error_message = "expecting boolean for keep_locks keyword arg";
        bool keep_locks = args.getBoolean( name_keep_locks, true );
#endif
#if defined( PYSVN_HAS_CLIENT_COMMIT4 )
        type_error_message = "expecting recurse or depth keyword arg";
        svn_depth_t depth = args.getDepth( name_depth, name_recurse, svn_depth_infinity, svn_depth_infinity, svn_depth_files );

        bool keep_changelist = args.getBoolean( name_keep_changelist, false );
        apr_array_header_t *changelists = NULL;

        if( args.hasArg( name_changelists ) )
        {
            changelists = arrayOfStringsFromListOfStrings( args.getArg( name_changelists ), pool );
        }

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
#if defined( PYSVN_HAS_CLIENT_COMMIT5 )
        type_error_message = "expecting boolean for commit_as_operations keyword arg";
        bool commit_as_operations = args.getBoolean( name_commit_as_operations, false );
#endif
#if defined( PYSVN_HAS_CLIENT_COMMIT6 )
        type_error_message = "expecting boolean for include_file_externals keyword arg";
        bool include_file_externals = args.getBoolean( name_include_file_externals, false );

        type_error_message = "expecting boolean for include_dir_externals keyword arg";
        bool include_dir_externals = args.getBoolean( name_include_dir_externals, false );
#endif

        try
        {
            checkThreadPermission();

            PythonAllowThreads permission( m_context );

            m_context.setLogMessage( message );

#if defined( PYSVN_HAS_CLIENT_COMMIT6 )
            svn_error_t *error = svn_client_commit6
                (
                targets,
                depth,
                keep_locks,
                keep_changelist,
                commit_as_operations,
                include_file_externals,
                include_dir_externals,
                changelists,
                revprops,
                commit_info.callback(),
                commit_info.baton(),
                m_context,
                pool
                );
#elif defined( PYSVN_HAS_CLIENT_COMMIT5 )
            svn_error_t *error = svn_client_commit5
                (
                targets,
                depth,
                keep_locks,
                keep_changelist,
                commit_as_operations,
                changelists,
                revprops,
                commit_info.callback(),
                commit_info.baton(),
                m_context,
                pool
                );
#elif defined( PYSVN_HAS_CLIENT_COMMIT4 )
            svn_error_t *error = svn_client_commit4
                (
                &commit_info,
                targets,
                depth,
                keep_locks,
                keep_changelist,
                changelists,
                revprops,
                m_context,
                pool
                );
#elif defined( PYSVN_HAS_CLIENT_COMMIT3 )
            svn_error_t *error = svn_client_commit3
                (
                &commit_info,   // commit info type changed
                targets,
                recurse,
                keep_locks,
                m_context,
                pool
                );
#elif defined( PYSVN_HAS_CLIENT_COMMIT2 )
            svn_error_t *error = svn_client_commit2
                (
                &commit_info,
                targets,
                recurse,
                keep_locks,
                m_context,
                pool
                );
#else
            svn_error_t *error = svn_client_commit
                (
                &commit_info,
                targets,
                !recurse,       // non recursive
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

#if defined( PYSVN_HAS_CLIENT_COMMIT5 )
    return toObject( commit_info, m_wrapper_commit_info, m_commit_info_style );
#else
    return toObject( commit_info, m_commit_info_style );
#endif
}

Py::Object pysvn_client::cmd_checkout( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_url },
    { true,  name_path },
    { false, name_recurse },
    { false, name_revision },
#if defined( PYSVN_HAS_CLIENT_CHECKOUT2 )
    { false, name_peg_revision },
    { false, name_ignore_externals },
#endif
#if defined( PYSVN_HAS_CLIENT_CHECKOUT3 )
    { false, name_allow_unver_obstructions },
    { false, name_depth },
#endif
    { false, NULL }
    };
    FunctionArguments args( "checkout", args_desc, a_args, a_kws );
    args.check();

    std::string url( args.getUtf8String( name_url ) );
    std::string path( args.getUtf8String( name_path ) );
#if defined( PYSVN_HAS_CLIENT_CHECKOUT3 )
    bool allow_unver_obstructions = args.getBoolean( name_allow_unver_obstructions, false );
    svn_depth_t depth = args.getDepth( name_depth, name_recurse, svn_depth_infinity, svn_depth_infinity, svn_depth_files );
#else
    bool recurse = args.getBoolean( name_recurse, true );
#endif
    svn_opt_revision_t revision = args.getRevision( name_revision, svn_opt_revision_head );
#if defined( PYSVN_HAS_CLIENT_CHECKOUT2 )
    svn_opt_revision_t peg_revision = args.getRevision( name_peg_revision, revision );
    bool ignore_externals = args.getBoolean( name_ignore_externals, false );
#endif

    SvnPool pool( m_context );

    bool is_url = is_svn_url( path );
#if defined( PYSVN_HAS_CLIENT_CHECKOUT2 )
    revisionKindCompatibleCheck( is_url, peg_revision, name_peg_revision, name_url_or_path );
#endif
    revisionKindCompatibleCheck( is_url, revision, name_revision, name_url_or_path );

    svn_revnum_t revnum = 0;
    try
    {
        std::string norm_url( svnNormalisedIfPath( url, pool ) );
        std::string norm_path( svnNormalisedIfPath( path, pool ) );

        checkThreadPermission();

        PythonAllowThreads permission( m_context );

        svn_revnum_t revnum = 0;

#if defined( PYSVN_HAS_CLIENT_CHECKOUT3 )
        svn_error_t *error = svn_client_checkout3
            (
            &revnum,
            norm_url.c_str(),
            norm_path.c_str(),
            &peg_revision,
            &revision,
            depth,
            ignore_externals,
            allow_unver_obstructions,
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_CHECKOUT2 )
        svn_error_t *error = svn_client_checkout2
            (
            &revnum,
            norm_url.c_str(),
            norm_path.c_str(),
            &peg_revision,
            &revision,
            recurse,
            ignore_externals,
            m_context,
            pool
            );
#else
        svn_error_t *error = svn_client_checkout
            (
            &revnum,
            norm_url.c_str(),
            norm_path.c_str(),
            &revision,
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

    return Py::asObject( new pysvn_revision( svn_opt_revision_number, 0, revnum ) );
}

Py::Object pysvn_client::cmd_cleanup( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_path },
#if defined( PYSVN_HAS_CLIENT_CLEANUP2 )
    { false, name_break_locks },
    { false, name_fix_recorded_timestamps },
    { false, name_clear_dav_cache },
    { false, name_vacuum_pristines },
    { false, name_include_externals },
#endif
    { false, NULL }
    };
    FunctionArguments args( "cleanup", args_desc, a_args, a_kws );
    args.check();

    std::string path( args.getUtf8String( name_path ) );

#if defined( PYSVN_HAS_CLIENT_CLEANUP2 )
    bool break_locks = args.getBoolean( name_break_locks, true );
    bool fix_recorded_timestamps = args.getBoolean( name_fix_recorded_timestamps, true );
    bool clear_dav_cache = args.getBoolean( name_clear_dav_cache, true );
    bool vacuum_pristines = args.getBoolean( name_vacuum_pristines, true );
    bool include_externals = args.getBoolean( name_include_externals, false );
#endif

    SvnPool pool( m_context );

    try
    {
        std::string norm_path( svnNormalisedIfPath( path, pool ) );

        checkThreadPermission();

        PythonAllowThreads permission( m_context );

#if defined( PYSVN_HAS_CLIENT_CLEANUP2 )
        const char *abspath = NULL;
        svn_error_t *error = svn_dirent_get_absolute( &abspath, norm_path.c_str(), pool );

        if( error == NULL )
        {
            error = svn_client_cleanup2
                (
                abspath,
                break_locks,
                fix_recorded_timestamps,
                clear_dav_cache,
                vacuum_pristines,
                include_externals,
                m_context,
                pool
                );
        }
#else
        svn_error_t * error = svn_client_cleanup
            (
            norm_path.c_str(),
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

    return Py::None();
}
#if defined( PYSVN_HAS_CLIENT_VACUUM )
Py::Object pysvn_client::cmd_vacuum( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_path },
    { false, name_remove_unversioned_items },
    { false, name_remove_ignored_items },
    { false, name_fix_recorded_timestamps },
    { false, name_vacuum_pristines },
    { false, name_include_externals },
    { false, NULL }
    };
    FunctionArguments args( "vacuum", args_desc, a_args, a_kws );
    args.check();

    std::string path( args.getUtf8String( name_path ) );

    bool remove_unversioned_items = args.getBoolean( name_remove_unversioned_items, false );
    bool remove_ignored_items = args.getBoolean( name_remove_ignored_items, false );
    bool fix_recorded_timestamps = args.getBoolean( name_fix_recorded_timestamps, true );
    bool vacuum_pristines = args.getBoolean( name_vacuum_pristines, true );
    bool include_externals = args.getBoolean( name_include_externals, false );

    SvnPool pool( m_context );

    try
    {
        std::string norm_path( svnNormalisedIfPath( path, pool ) );

        checkThreadPermission();

        PythonAllowThreads permission( m_context );

        const char *abspath = NULL;
        svn_error_t *error = svn_dirent_get_absolute( &abspath, norm_path.c_str(), pool );

        if( error == NULL )
        {
            error = svn_client_vacuum
                (
                abspath,
                remove_unversioned_items,
                remove_ignored_items,
                fix_recorded_timestamps,
                vacuum_pristines,
                include_externals,
                m_context,
                pool
                );
        }

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

    return Py::None();
}
#endif

Py::Object pysvn_client::cmd_resolved( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_path },
    { false, name_recurse },
#if defined( PYSVN_HAS_CLIENT_RESOLVE )
    { false, name_depth },
    { false, name_conflict_choice },
#endif
    { false, NULL }
    };
    FunctionArguments args( "resolved", args_desc, a_args, a_kws );
    args.check();

    std::string path( args.getUtf8String( name_path ) );
#if defined( PYSVN_HAS_CLIENT_RESOLVE )
    svn_depth_t depth = args.getDepth( name_depth, name_recurse, svn_depth_files, svn_depth_infinity, svn_depth_files );
    svn_wc_conflict_choice_t conflict_choice = args.getWcConflictChoice(
                                name_conflict_choice, svn_wc_conflict_choose_merged );
#else
    bool recurse = args.getBoolean( name_recurse, false );
#endif

    SvnPool pool( m_context );

    try
    {
        std::string norm_path( svnNormalisedIfPath( path, pool ) );

        checkThreadPermission();

        PythonAllowThreads permission( m_context );
#if defined( PYSVN_HAS_CLIENT_RESOLVE )
        svn_error_t *error = svn_client_resolve
            (
            norm_path.c_str(),
            depth,
            conflict_choice,
            m_context,
            pool
            );
#else
        svn_error_t *error = svn_client_resolved
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
    catch( SvnException &e )
    {
        // use callback error over ClientException
        m_context.checkForError( m_module.client_error );

        throw_client_error( e );
    }

    return Py::None();
}

Py::Object pysvn_client::cmd_update( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_path },
    { false, name_recurse },
    { false, name_revision },
#if defined( PYSVN_HAS_CLIENT_UPDATE2 )
    { false, name_ignore_externals },
#endif
#if defined( PYSVN_HAS_CLIENT_UPDATE3 )
    { false, name_depth },
    { false, name_depth_is_sticky },
    { false, name_allow_unver_obstructions },
#endif
#if defined( PYSVN_HAS_CLIENT_UPDATE4 )
    { false, name_adds_as_modification },
    { false, name_make_parents },
#endif
    { false, NULL }
    };
    FunctionArguments args( "update", args_desc, a_args, a_kws );
    args.check();

    SvnPool pool( m_context );

    apr_array_header_t *targets = targetsFromStringOrList( args.getArg( name_path ), pool );

    svn_opt_revision_t revision = args.getRevision( name_revision, svn_opt_revision_head );
#if defined( PYSVN_HAS_CLIENT_UPDATE3 )
    svn_depth_t depth = args.getDepth( name_depth, name_recurse, svn_depth_unknown, svn_depth_unknown, svn_depth_files );
    bool depth_is_sticky = args.getBoolean( name_depth_is_sticky, false );
    bool allow_unver_obstructions = args.getBoolean( name_allow_unver_obstructions, false );
#else
    bool recurse = args.getBoolean( name_recurse, true );
#endif
#if defined( PYSVN_HAS_CLIENT_UPDATE2 )
    bool ignore_externals = args.getBoolean( name_ignore_externals, false );
#endif
#if defined( PYSVN_HAS_CLIENT_UPDATE4 )
    bool adds_as_modification = args.getBoolean( name_adds_as_modification, false );
    bool make_parents = args.getBoolean( name_make_parents, false );
#endif

    apr_array_header_t *result_revs = NULL;

    try
    {
        checkThreadPermission();

        PythonAllowThreads permission( m_context );

#if defined( PYSVN_HAS_CLIENT_UPDATE4 )
        svn_error_t *error = svn_client_update4
            (
            &result_revs,
            targets,
            &revision,
            depth,
            depth_is_sticky,
            ignore_externals,
            allow_unver_obstructions,
            adds_as_modification,
            make_parents,
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_UPDATE3 )
        svn_error_t *error = svn_client_update3
            (
            &result_revs,
            targets,
            &revision,
            depth,
            depth_is_sticky,
            ignore_externals,
            allow_unver_obstructions,
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_UPDATE2 )
        svn_error_t *error = svn_client_update2
            (
            &result_revs,
            targets,
            &revision,
            recurse,
            ignore_externals,
            m_context,
            pool
            );
#else
        svn_error_t *error = svn_client_update
            (
            &result_revs,
            targets,
            &revision,
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

    return revnumListToObject( result_revs, pool );
}

#if defined( PYSVN_HAS_CLIENT_UPGRADE )
Py::Object pysvn_client::cmd_upgrade( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_path },
    { false, NULL }
    };
    FunctionArguments args( "upgrade", args_desc, a_args, a_kws );
    args.check();

    SvnPool pool( m_context );

    std::string type_error_message;
    try
    {
        type_error_message = "expecting string for path keyword arg";
        std::string path( args.getUtf8String( name_path ) );
        std::string norm_path( svnNormalisedIfPath( path, pool ) );

        try
        {
            checkThreadPermission();

            PythonAllowThreads permission( m_context );

            svn_error_t *error = svn_client_upgrade(
                norm_path.c_str(),
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
