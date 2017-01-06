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
#include "svn_path.h"
#include "svn_config.h"
#include "pysvn_static_strings.hpp"

Py::Object pysvn_client::cmd_ls( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_url_or_path },
    { false, name_revision },
    { false, name_recurse },
#if defined( PYSVN_HAS_CLIENT_LS2 )
    { false, name_peg_revision },
#endif
    { false, NULL }
    };
    FunctionArguments args( "ls", args_desc, a_args, a_kws );
    args.check();

    std::string path( args.getUtf8String( name_url_or_path ) );
    bool recurse = args.getBoolean( name_recurse, false );
    svn_opt_revision_t revision = args.getRevision( name_revision, svn_opt_revision_head );

    SvnPool pool( m_context );
    apr_hash_t *dirents = NULL;
    std::string norm_path( svnNormalisedIfPath( path, pool ) );
#if defined( PYSVN_HAS_CLIENT_LS2 )
    svn_opt_revision_t peg_revision = args.getRevision( name_peg_revision, revision );
#endif

    bool is_url = is_svn_url( path );
#if defined( PYSVN_HAS_CLIENT_LS2 )
    revisionKindCompatibleCheck( is_url, peg_revision, name_peg_revision, name_url_or_path );
#endif
    revisionKindCompatibleCheck( is_url, revision, name_revision, name_url_or_path );

    try
    {
        checkThreadPermission();

        PythonAllowThreads permission( m_context );

#if defined( PYSVN_HAS_CLIENT_LS3 )
        svn_error_t *error = svn_client_ls3 // ignore deprecation warning - support old Client().ls()
            (
            &dirents,
            NULL,
            norm_path.c_str(),
            &peg_revision,
            &revision,
            recurse,
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_LS2 )
        svn_error_t *error = svn_client_ls2
            (
            &dirents,
            norm_path.c_str(),
            &peg_revision,
            &revision,
            recurse,
            m_context,
            pool
            );
#else
        svn_error_t *error = svn_client_ls
            (
            &dirents,
            norm_path.c_str(),
            &revision,
            recurse,
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

    std::string base_path;
    if( !norm_path.empty() )
    {
        base_path = norm_path;
        base_path += '/';
    }

    // convert the entries into python objects
    Py::List entries_list;

    for( apr_hash_index_t *hi = apr_hash_first( pool, dirents );
            hi != NULL;
                hi = apr_hash_next( hi ) )
    {
        const void *key;
        void *val;
        apr_hash_this( hi, &key, NULL, &val );

        const char *utf8_entryname = static_cast<const char *>( key );
        svn_dirent_t *dirent = static_cast<svn_dirent_t *>( val );

        std::string full_name( base_path );
        full_name += utf8_entryname;

        Py::Dict entry_dict;
        entry_dict[ *py_name_name ] = Py::String( full_name, name_utf8 );
        entry_dict[ *py_name_kind ] = toEnumValue( dirent->kind );
        entry_dict[ *py_name_has_props ] = Py::Int( dirent->has_props );
        entry_dict[ *py_name_size ] = Py::Long( Py::Float( double( static_cast<signed_int64>( dirent->size ) ) ) );
        entry_dict[ *py_name_created_rev ] = Py::asObject( new pysvn_revision( svn_opt_revision_number, 0, dirent->created_rev ) );
        entry_dict[ *py_name_time ] = toObject( dirent->time );
        entry_dict[ *py_name_last_author ] = utf8_string_or_none( dirent->last_author );

        entries_list.append( m_wrapper_dirent.wrapDict( entry_dict ) );
    }

    return entries_list;
}

#if defined( PYSVN_HAS_CLIENT_LIST )
extern "C" svn_error_t *list_receiver_c
    (
    void *baton_,
    const char *path,
    const svn_dirent_t *dirent,
    const svn_lock_t *lock,
    const char *abs_path,
#if defined( PYSVN_HAS_CLIENT_LIST3 )
    const char *external_parent_url,
    const char *external_target,
#endif
    apr_pool_t *pool
    );

class ListReceiveBaton
{
public:
    ListReceiveBaton( PythonAllowThreads *permission, Py::List &list_list, SvnPool &pool )
        : m_permission( permission )
        , m_include_externals( false )
        , m_list_list( list_list )
        , m_pool( pool )
        {}
    ~ListReceiveBaton()
        {}

#if defined( PYSVN_HAS_CLIENT_LIST3 )
    svn_client_list_func2_t callback() { return &list_receiver_c; }
#else
    svn_client_list_func_t callback() { return &list_receiver_c; }
#endif
    void *baton() { return static_cast<void *>( this ); }
    static ListReceiveBaton *castBaton( void *baton_ ) { return static_cast<ListReceiveBaton *>( baton_ ); }

    PythonAllowThreads  *m_permission;

    apr_uint32_t        m_dirent_fields;
    bool                m_fetch_locks;
    bool                m_include_externals;
    bool                m_is_url;
    std::string         m_url_or_path;
    DictWrapper         *m_wrapper_lock;
    DictWrapper         *m_wrapper_list;

    Py::List            &m_list_list;
    SvnPool             &m_pool;
};

extern "C" svn_error_t *list_receiver_c
    (
    void *baton_,
    const char *path,
    const svn_dirent_t *dirent,
    const svn_lock_t *lock,
    const char *abs_path,
#if defined( PYSVN_HAS_CLIENT_LIST3 )
    const char *external_parent_url,
    const char *external_target,
#endif
    apr_pool_t *pool
    )
{
    ListReceiveBaton *baton = ListReceiveBaton::castBaton( baton_ );

    PythonDisallowThreads callback_permission( baton->m_permission );

    std::string full_path( baton->m_url_or_path );
    std::string full_repos_path( abs_path );
    if( strlen( path ) != 0 )
    {
        full_path += "/";
        full_path += path;

        full_repos_path += "/";
        full_repos_path += path;
    }

    Py::Tuple py_tuple( baton->m_include_externals ? 4 : 2 );

    Py::Dict entry_dict;
    entry_dict[ *py_name_path ] = Py::String( full_path, name_utf8 );
    entry_dict[ *py_name_repos_path ] = Py::String( full_repos_path, name_utf8 );

    if( dirent != NULL )
    {
        if( baton->m_dirent_fields&SVN_DIRENT_KIND )
        {
            entry_dict[ *py_name_kind ] = toEnumValue( dirent->kind );
        }
        if( baton->m_dirent_fields&SVN_DIRENT_SIZE )
        {
            entry_dict[ *py_name_size ] = Py::Long( Py::Float( double( static_cast<signed_int64>( dirent->size ) ) ) );
        }
        if( baton->m_dirent_fields&SVN_DIRENT_CREATED_REV )
        {
            entry_dict[ *py_name_created_rev ] = Py::asObject( new pysvn_revision( svn_opt_revision_number, 0, dirent->created_rev ) );
        }
        if( baton->m_dirent_fields&SVN_DIRENT_TIME )
        {
            entry_dict[ *py_name_time ] = toObject( dirent->time );
        }
        if( baton->m_dirent_fields&SVN_DIRENT_HAS_PROPS )
        {
            entry_dict[ *py_name_has_props ] = Py::Int( dirent->has_props );
        }
        if( baton->m_dirent_fields&SVN_DIRENT_LAST_AUTHOR )
        {
            entry_dict[ *py_name_last_author ] = utf8_string_or_none( dirent->last_author );
        }
    }
    py_tuple[0] = baton->m_wrapper_list->wrapDict( entry_dict );

    if( lock == NULL )
    {
        py_tuple[1] = Py::None();
    }
    else
    {
        py_tuple[1] = toObject( *lock, *baton->m_wrapper_lock );
    }

#if defined( PYSVN_HAS_CLIENT_LIST3 )
    if( baton->m_include_externals )
    {
        py_tuple[2] = path_string_or_none( external_parent_url, baton->m_pool );
        py_tuple[3] = path_string_or_none( external_target, baton->m_pool );
    }
#endif

    baton->m_list_list.append( py_tuple );

    return SVN_NO_ERROR;
}

Py::Object pysvn_client::cmd_list( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_url_or_path },
    { false, name_peg_revision },
    { false, name_revision },
    { false, name_recurse },
    { false, name_dirent_fields },
    { false, name_fetch_locks },
#if defined( PYSVN_HAS_CLIENT_LIST2 )
    { false, name_depth },
#endif
#if defined( PYSVN_HAS_CLIENT_LIST3 )
    { false, name_include_externals },
#endif
    { false, NULL }
    };
    FunctionArguments args( "list", args_desc, a_args, a_kws );
    args.check();

    std::string path( args.getUtf8String( name_url_or_path ) );
    svn_opt_revision_t peg_revision = args.getRevision( name_peg_revision, svn_opt_revision_unspecified );
    bool is_url = is_svn_url( path );
    svn_opt_revision_t revision;
    if( is_url )
         revision = args.getRevision( name_revision, svn_opt_revision_head );
    else
         revision = args.getRevision( name_revision, svn_opt_revision_working );
#if defined( PYSVN_HAS_CLIENT_LIST2 )
    svn_depth_t depth = args.getDepth( name_depth, name_recurse, svn_depth_immediates, svn_depth_infinity, svn_depth_immediates );
#else
    bool recurse = args.getBoolean( name_recurse, false );
#endif
    apr_uint32_t dirent_fields = args.getLong( name_dirent_fields, SVN_DIRENT_ALL );
    bool fetch_locks = args.getBoolean( name_fetch_locks, false );

#if defined( PYSVN_HAS_CLIENT_LIST3 )
    bool include_externals = args.getBoolean( name_include_externals, false );
#endif

    revisionKindCompatibleCheck( is_url, peg_revision, name_peg_revision, name_url_or_path );
    revisionKindCompatibleCheck( is_url, revision, name_revision, name_url_or_path );

    SvnPool pool( m_context );
    std::string norm_path( svnNormalisedIfPath( path, pool ) );

    Py::List list_list;

    try
    {
        checkThreadPermission();

        PythonAllowThreads permission( m_context );

        ListReceiveBaton list_baton( &permission, list_list, pool );
        list_baton.m_dirent_fields = dirent_fields;
        list_baton.m_is_url = is_url;
        list_baton.m_fetch_locks = fetch_locks;
        list_baton.m_url_or_path = norm_path;
        list_baton.m_wrapper_lock = &m_wrapper_lock;
        list_baton.m_wrapper_list = &m_wrapper_list;
#if defined( PYSVN_HAS_CLIENT_LIST3 )
        list_baton.m_include_externals = include_externals;
#endif

#if defined( PYSVN_HAS_CLIENT_LIST3 )
        svn_error_t *error = svn_client_list3
            (
            norm_path.c_str(),
            &peg_revision,
            &revision,
            depth,
            dirent_fields,
            fetch_locks,
            include_externals,
            list_baton.callback(),
            list_baton.baton(),
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_LIST2 )
        svn_error_t *error = svn_client_list2
            (
            norm_path.c_str(),
            &peg_revision,
            &revision,
            depth,
            dirent_fields,
            fetch_locks,
            list_baton.callback(),
            list_baton.baton(),
            m_context,
            pool
            );
#else
        svn_error_t *error = svn_client_list
            (
            norm_path.c_str(),
            &peg_revision,
            &revision,
            recurse,
            dirent_fields,
            fetch_locks,
            list_baton.callback(),
            list_baton.baton(),
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

    return list_list;
}
#endif
