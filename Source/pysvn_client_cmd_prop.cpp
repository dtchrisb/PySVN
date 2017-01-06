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
#include "pysvn_static_strings.hpp"

#if defined( PYSVN_HAS_CLIENT_PROPSET_LOCAL )
Py::Object pysvn_client::cmd_propdel_local( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_prop_name },
    { true,  name_path },
    { false, name_depth },
    { false, name_changelists },
    { false, NULL }
    };
    FunctionArguments args( "propdel_local", args_desc, a_args, a_kws );
    args.check();

    return common_propset_local( args, false );
}

Py::Object pysvn_client::cmd_propset_local( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_prop_name },
    { true,  name_prop_value },
    { true,  name_path },
    { false, name_depth },
    { false, name_skip_checks },
    { false, name_changelists },
    { false, NULL }
    };
    FunctionArguments args( "propset_local", args_desc, a_args, a_kws );
    args.check();

    return common_propset_local( args, true );
}

Py::Object pysvn_client::common_propset_local( FunctionArguments &args, bool is_set )
{
    SvnPool pool( m_context );

    std::string propname( args.getUtf8String( name_prop_name ) );
    std::string propval;
    if( is_set )
    {
        propval = args.getUtf8String( name_prop_value );
    }

    // path
    apr_array_header_t *targets = targetsFromStringOrList( args.getArg( name_path ), pool );

    // depth
    svn_depth_t depth = args.getDepth( name_depth, svn_depth_empty );

    // skip_check
    bool skip_checks = false;
    if( is_set )
    {
        skip_checks = args.getBoolean( name_skip_checks, false );
    }

    // changelist
    apr_array_header_t *changelists = NULL;
    if( args.hasArg( name_changelists ) )
    {
        changelists = arrayOfStringsFromListOfStrings( args.getArg( name_changelists ), pool );
    }

    try
    {
        checkThreadPermission();

        PythonAllowThreads permission( m_context );

        const svn_string_t *svn_propval = NULL;
        if( is_set )
        {
            svn_propval = svn_string_ncreate( propval.c_str(), propval.size(), pool );
        }

        svn_error_t *error = svn_client_propset_local
            (
            propname.c_str(),
            svn_propval,
            targets,
            depth,
            skip_checks,
            changelists,
            m_context.ctx(),
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

    return Py::None();
}
#endif

#if defined( PYSVN_HAS_CLIENT_PROPSET_REMOTE )
Py::Object pysvn_client::cmd_propdel_remote( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_prop_name },
    { true,  name_url },
    { false, name_base_revision_for_url },
    { false, NULL }
    };
    FunctionArguments args( "propdel_remote", args_desc, a_args, a_kws );
    args.check();

    return common_propset_remote( args, false );
}

Py::Object pysvn_client::cmd_propset_remote( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_prop_name },
    { true,  name_prop_value },
    { true,  name_url },
    { false, name_skip_checks },
    { false, name_base_revision_for_url },
    { false, name_revprops },
    { false, NULL }
    };
    FunctionArguments args( "propset_remote", args_desc, a_args, a_kws );
    args.check();

    return common_propset_remote( args, true );
}

Py::Object pysvn_client::common_propset_remote( FunctionArguments &args, bool is_set )
{
    SvnPool pool( m_context );

    std::string propname( args.getUtf8String( name_prop_name ) );
    std::string propval;
    if( is_set )
    {
        propval = args.getUtf8String( name_prop_value );
    }

    // url
    std::string url( args.getUtf8String( name_url ) );

    // skip_check
    bool skip_checks = false;
    if( is_set )
    {
        skip_checks = args.getBoolean( name_skip_checks, false );
    }

    // base_revision_for_url
    svn_revnum_t base_revision_for_url = SVN_INVALID_REVNUM;
    if( args.hasArg( name_base_revision_for_url ) )
    {
        svn_opt_revision_t revision = args.getRevision( name_base_revision_for_url );
        if( revision.kind == svn_opt_revision_number )
        {
            base_revision_for_url = revision.value.number;
        }
        else
        {
            std::string msg = args.m_function_name;
            msg += "() expects ";
            msg += name_base_revision_for_url;
            msg += " to be a number kind revision";
            throw Py::TypeError( msg );
        }
    }

    // revprops
    apr_hash_t *revprops = NULL;
    if( is_set && args.hasArg( name_revprops ) )
    {
        Py::Object py_revprop = args.getArg( name_revprops );
        if( !py_revprop.isNone() )
        {
            revprops = hashOfStringsFromDictOfStrings( py_revprop, pool );
        }
    }

    CommitInfoResult commit_baton( pool );

    try
    {
        checkThreadPermission();

        PythonAllowThreads permission( m_context );

        const svn_string_t *svn_propval = NULL;
        if( is_set )
        {
            svn_propval = svn_string_ncreate( propval.c_str(), propval.size(), pool );
        }

        svn_error_t *error = svn_client_propset_remote
            (
            propname.c_str(),
            svn_propval,
            url.c_str(),
            skip_checks,
            base_revision_for_url,
            revprops,
            commit_baton.callback(),
            commit_baton.baton(),
            m_context.ctx(),
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

    return toObject( commit_baton, m_wrapper_commit_info, m_commit_info_style );
}
#endif

Py::Object pysvn_client::cmd_propdel( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_prop_name },
    { true,  name_url_or_path },
    { false, name_revision },
    { false, name_recurse },
#if defined( PYSVN_HAS_CLIENT_PROPSET2 )
    { false, name_skip_checks },
#endif
#if defined( PYSVN_HAS_CLIENT_PROPSET3 )
    { false, name_depth },
    { false, name_base_revision_for_url },
    { false, name_changelists },
    { false, name_revprops },
#endif
    { false, NULL }
    };
    FunctionArguments args( "propdel", args_desc, a_args, a_kws );
    args.check();

    return common_propset( args, false );
}

Py::Object pysvn_client::cmd_propset( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_prop_name },
    { true,  name_prop_value },
    { true,  name_url_or_path },
    { false, name_revision },
    { false, name_recurse },
#if defined( PYSVN_HAS_CLIENT_PROPSET2 )
    { false, name_skip_checks },
#endif
#if defined( PYSVN_HAS_CLIENT_PROPSET3 )
    { false, name_depth },
    { false, name_base_revision_for_url },
    { false, name_changelists },
    { false, name_revprops },
#endif
    { false, NULL }
    };
    FunctionArguments args( "propset", args_desc, a_args, a_kws );
    args.check();

    return common_propset( args, true );
}

Py::Object pysvn_client::common_propset( FunctionArguments &args, bool is_set )
{
    std::string propname( args.getUtf8String( name_prop_name ) );
    std::string path( args.getUtf8String( name_url_or_path ) );
    std::string propval;
    if( is_set )
    {
        propval = args.getUtf8String( name_prop_value );
    }

    svn_opt_revision_t revision;
    if( is_svn_url( path ) )
        revision = args.getRevision( name_revision, svn_opt_revision_head );
    else
        revision = args.getRevision( name_revision, svn_opt_revision_working );

    SvnPool pool( m_context );

#if defined( PYSVN_HAS_CLIENT_PROPSET3 )
    apr_array_header_t *changelists = NULL;

    if( args.hasArg( name_changelists ) )
    {
        changelists = arrayOfStringsFromListOfStrings( args.getArg( name_changelists ), pool );
    }

    svn_revnum_t base_revision_for_url = SVN_INVALID_REVNUM;
    if( args.hasArg( name_base_revision_for_url ) )
    {
        svn_opt_revision_t revision = args.getRevision( name_base_revision_for_url );
        if( revision.kind == svn_opt_revision_number )
        {
            base_revision_for_url = revision.value.number;
        }
        else
        {
            std::string msg = args.m_function_name;
            msg += "() expects ";
            msg += name_base_revision_for_url;
            msg += " to be a number kind revision";
            throw Py::TypeError( msg );
        }
    }

    svn_depth_t depth = args.getDepth( name_depth, name_recurse, svn_depth_empty, svn_depth_infinity, svn_depth_empty );

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
    bool recurse = args.getBoolean( name_recurse, false );
#endif
#if defined( PYSVN_HAS_CLIENT_PROPSET2 )
    bool skip_checks = args.getBoolean( name_skip_checks, false );
#endif

#if defined( PYSVN_HAS_CLIENT_PROPSET_REMOTE )
    CommitInfoResult commit_info( pool );

#elif defined( PYSVN_HAS_CLIENT_PROPSET3 )
    pysvn_commit_info_t *commit_info = NULL;
#endif

    try
    {
        std::string norm_path( svnNormalisedIfPath( path, pool ) );

        checkThreadPermission();

        PythonAllowThreads permission( m_context );

        const svn_string_t *svn_propval = NULL;
        if( is_set )
        {
            svn_propval = svn_string_ncreate( propval.c_str(), propval.size(), pool );
        }
#if defined( PYSVN_HAS_CLIENT_PROPSET_LOCAL ) && defined( PYSVN_HAS_CLIENT_PROPSET_REMOTE )
        svn_error_t *error;
        if( is_svn_url( norm_path ) )
        {
            error = svn_client_propset_remote
                (
                propname.c_str(),
                svn_propval,
                norm_path.c_str(),
                skip_checks,
                base_revision_for_url,
                revprops,
                commit_info.callback(),
                commit_info.baton(),
                m_context.ctx(),
                pool
                );
        }
        else
        {
            apr_array_header_t *targets = apr_array_make( pool, 11, sizeof( const char * ) );
            APR_ARRAY_PUSH( targets, const char * ) = apr_pstrdup( pool, norm_path.c_str() );

            error = svn_client_propset_local
                (
                propname.c_str(),
                svn_propval,
                targets,
                depth,
                skip_checks,
                changelists,
                m_context.ctx(),
                pool
                );
        }
#elif defined( PYSVN_HAS_CLIENT_PROPSET3 )
        svn_error_t *error = svn_client_propset3
            (
            &commit_info,
            propname.c_str(),
            svn_propval,
            norm_path.c_str(),
            depth,
            skip_checks,
            base_revision_for_url,
            changelists,
            revprops,
            m_context.ctx(),
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_PROPSET2 )
        svn_error_t *error = svn_client_propset2
            (
            propname.c_str(),
            svn_propval,
            norm_path.c_str(),
            recurse,
            skip_checks,
            m_context.ctx(),
            pool
            );
#else
        svn_error_t *error = svn_client_propset
            (
            propname.c_str(),
            svn_propval,
            norm_path.c_str(),
            recurse,
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

#if defined( PYSVN_HAS_CLIENT_PROPSET_REMOTE )
    return toObject( commit_info, m_wrapper_commit_info, m_commit_info_style );

#elif defined( PYSVN_HAS_CLIENT_PROPSET3 )
    return toObject( commit_info, m_commit_info_style );

#else
    return Py::None();
#endif
}

Py::Object pysvn_client::cmd_propget( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_prop_name },
    { true,  name_url_or_path },
    { false, name_revision },
    { false, name_recurse },
#if defined( PYSVN_HAS_CLIENT_PROPGET2 )
    { false, name_peg_revision },
#endif
#if defined( PYSVN_HAS_CLIENT_PROPGET3 )
    { false, name_depth },
    { false, name_changelists },
#endif
#if defined( PYSVN_HAS_CLIENT_PROPGET5 )
    { false, name_get_inherited_props },
#endif
    { false, NULL }
    };
    FunctionArguments args( "propget", args_desc, a_args, a_kws );
    args.check();

    std::string propname( args.getUtf8String( name_prop_name ) );
    std::string path( args.getUtf8String( name_url_or_path ) );

    SvnPool pool( m_context );

#if defined( PYSVN_HAS_CLIENT_PROPGET3 )
    apr_array_header_t *changelists = NULL;

    if( args.hasArg( name_changelists ) )
    {
        changelists = arrayOfStringsFromListOfStrings( args.getArg( name_changelists ), pool );
    }

    svn_depth_t depth = args.getDepth( name_depth, name_recurse, svn_depth_files, svn_depth_infinity, svn_depth_empty );
#else
    bool recurse = args.getBoolean( name_recurse, false );
#endif
    svn_opt_revision_t revision;
    if( is_svn_url( path ) )
        revision = args.getRevision( name_revision, svn_opt_revision_head );
    else
        revision = args.getRevision( name_revision, svn_opt_revision_working );
#if defined( PYSVN_HAS_CLIENT_PROPGET2 )
    svn_opt_revision_t peg_revision = args.getRevision( name_peg_revision, revision );
#endif

    bool is_url = is_svn_url( path );
#if defined( PYSVN_HAS_CLIENT_PROPGET2 )
    revisionKindCompatibleCheck( is_url, peg_revision, name_peg_revision, name_url_or_path );
#endif
    revisionKindCompatibleCheck( is_url, revision, name_revision, name_url_or_path );

    apr_hash_t *props = NULL;

#if defined( PYSVN_HAS_CLIENT_PROPGET3 )
    svn_revnum_t actual_revnum = 0;
#endif

#if defined( PYSVN_HAS_CLIENT_PROPGET5 )
    bool get_inherited_props = args.getBoolean( name_get_inherited_props, false );
    apr_array_header_t *inherited_props = NULL;
#endif

    try
    {
        std::string norm_path( svnNormalisedIfPath( path, pool ) );

        checkThreadPermission();

        PythonAllowThreads permission( m_context );

#if defined( PYSVN_HAS_CLIENT_PROPGET5 )
        svn_error_t *error = SVN_NO_ERROR;
        const char *abspath_or_url = NULL;
        if( !svn_path_is_url( norm_path.c_str() )
        &&  !svn_dirent_is_absolute( norm_path.c_str() ) )
        {
            error = svn_dirent_get_absolute( &abspath_or_url, norm_path.c_str(), pool );
        }
        else
        {
            abspath_or_url = norm_path.c_str();
        }

        if( error == SVN_NO_ERROR )
        {
            error = svn_client_propget5
                (
                &props,
                &inherited_props,
                propname.c_str(),
                abspath_or_url, // svn asserts if not absolute
                &peg_revision,
                &revision,
                &actual_revnum,
                depth,
                changelists,
                m_context,
                pool,
                pool
                );
        }
#elif defined( PYSVN_HAS_CLIENT_PROPGET4 )
        svn_error_t *error = SVN_NO_ERROR;
        const char *abspath_or_url = NULL;
        if( !svn_path_is_url( norm_path.c_str() )
        &&  !svn_dirent_is_absolute( norm_path.c_str() ) )
        {
            error = svn_dirent_get_absolute( &abspath_or_url, norm_path.c_str(), pool );
        }
        else
        {
            abspath_or_url = norm_path.c_str();
        }

        if( error == SVN_NO_ERROR )
        {
            error = svn_client_propget4
                (
                &props,
                propname.c_str(),
                abspath_or_url, // svn asserts if not absolute
                &peg_revision,
                &revision,
                &actual_revnum,
                depth,
                changelists,
                m_context,
                pool,
                pool
                );
        }
#elif defined( PYSVN_HAS_CLIENT_PROPGET3 )
        svn_error_t *error = svn_client_propget3
            (
            &props,
            propname.c_str(),
            norm_path.c_str(),
            &peg_revision,
            &revision,
            &actual_revnum,
            depth,
            changelists,
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_PROPGET2 )
        svn_error_t *error = svn_client_propget2
            (
            &props,
            propname.c_str(),
            norm_path.c_str(),
            &peg_revision,
            &revision,
            recurse,
            m_context,
            pool
            );
#else
        svn_error_t *error = svn_client_propget
            (
            &props,
            propname.c_str(),
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

#if defined( PYSVN_HAS_CLIENT_PROPGET5 )
    if( get_inherited_props )
    {
        Py::Tuple result( 2 );

        result[0] = propsToObject( props, pool );
        result[1] = inheritedPropsToObject( inherited_props, pool );
        return result;
    }
    else
#endif
    {
        return propsToObject( props, pool );
    }
}

#if defined( PYSVN_HAS_CLIENT_PROPLIST3 ) || defined( PYSVN_HAS_CLIENT_PROPLIST4 )
extern "C" svn_error_t *proplist_receiver_c
    (
    void *baton_,
    const char *path,
    apr_hash_t *prop_hash,
#if defined( PYSVN_HAS_CLIENT_PROPLIST4 )
    apr_array_header_t *inherited_props,
#endif
    apr_pool_t *pool
    );
class ProplistReceiveBaton
{
public:
    ProplistReceiveBaton( PythonAllowThreads *permission, SvnPool &pool, Py::List &prop_list, bool get_inherited_props )
        : m_permission( permission )
        , m_pool( pool )
        , m_get_inherited_props( get_inherited_props )
        , m_prop_list( prop_list )
        {}
    ~ProplistReceiveBaton()
        {}
#if defined( PYSVN_HAS_CLIENT_PROPLIST4 )
    svn_proplist_receiver2_t callback() { return &proplist_receiver_c; }
#else
    svn_proplist_receiver_t callback() { return &proplist_receiver_c; }
#endif
    void *baton() { return static_cast< void * >( this ); }
    static ProplistReceiveBaton *castBaton( void *baton_ ) { return static_cast<ProplistReceiveBaton *>( baton_ ); }

    PythonAllowThreads  *m_permission;
    SvnPool             &m_pool;
    bool                m_get_inherited_props;

    Py::List            &m_prop_list;
};

extern "C" svn_error_t *proplist_receiver_c
    (
    void *baton_,
    const char *path,
    apr_hash_t *prop_hash,
#if defined( PYSVN_HAS_CLIENT_PROPLIST4 )
    apr_array_header_t *inherited_props,
#endif
    apr_pool_t *pool
    )
{
    ProplistReceiveBaton *baton = ProplistReceiveBaton::castBaton( baton_ );

    PythonDisallowThreads callback_permission( baton->m_permission );

    Py::Dict prop_dict;

#if defined( PYSVN_HAS_CLIENT_PROPLIST4 )
    if( baton->m_get_inherited_props )
    {
        Py::Tuple py_tuple( 2 );
        py_tuple[0] = Py::String( path );
        py_tuple[1] = propsToObject( prop_hash, baton->m_pool );
        py_tuple[2] = inheritedPropsToObject( inherited_props, baton->m_pool );

        baton->m_prop_list.append( py_tuple );
    }
    else
#endif
    {
        Py::Tuple py_tuple( 2 );
        py_tuple[0] = Py::String( path );
        py_tuple[1] = propsToObject( prop_hash, baton->m_pool );

        baton->m_prop_list.append( py_tuple );
    }

    return SVN_NO_ERROR;
}
#endif

Py::Object pysvn_client::cmd_proplist( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_url_or_path },
    { false, name_revision },
    { false, name_recurse },
#if defined( PYSVN_HAS_CLIENT_PROPLIST2 )
    { false, name_peg_revision },
#endif
#if defined( PYSVN_HAS_CLIENT_PROPLIST3 )
    { false, name_depth },
    { false, name_changelists },
#endif
#if defined( PYSVN_HAS_CLIENT_PROPLIST4 )
    { false, name_get_inherited_props },
#endif
    { false, NULL }
    };
    FunctionArguments args( "proplist", args_desc, a_args, a_kws );
    args.check();

    Py::List path_list( toListOfStrings( args.getArg( name_url_or_path ) ) );

    SvnPool pool( m_context );

#if defined( PYSVN_HAS_CLIENT_PROPLIST3 )
    apr_array_header_t *changelists = NULL;

    if( args.hasArg( name_changelists ) )
    {
        changelists = arrayOfStringsFromListOfStrings( args.getArg( name_changelists ), pool );
    }

    svn_depth_t depth = args.getDepth( name_depth, name_recurse, svn_depth_empty, svn_depth_infinity, svn_depth_empty );
#else
    bool recurse = args.getBoolean( name_recurse, false );
#endif
    bool is_revision_setup = false;
    bool is_url = false;

    svn_opt_revision_t revision_url;
    svn_opt_revision_t revision_file;
    if( args.hasArg( name_revision ) )
    {
        revision_url = args.getRevision( name_revision );
        revision_file = revision_url;
    }
    else
    {
        revision_url.kind = svn_opt_revision_head;
        revision_file.kind = svn_opt_revision_working;
    }

#if defined( PYSVN_HAS_CLIENT_PROPLIST2 )
    svn_opt_revision_t peg_revision_url;
    svn_opt_revision_t peg_revision_file;
    if( args.hasArg( name_peg_revision ) )
    {
        peg_revision_url = args.getRevision( name_peg_revision );
        peg_revision_file = peg_revision_url;
    }
    else
    {
        peg_revision_url = revision_url;
        peg_revision_file = revision_file;
    }
#endif

#if defined( PYSVN_HAS_CLIENT_PROPLIST4 )
    bool get_inherited_props = args.getBoolean( name_get_inherited_props, false );
#endif

    Py::List list_of_proplists;

    for( Py::List::size_type i=0; i<path_list.length(); i++ )
    {
        Py::Bytes path_str( asUtf8Bytes( path_list[i] ) );
        std::string path( path_str.as_std_string() );
        std::string norm_path( svnNormalisedIfPath( path, pool ) );

        svn_opt_revision_t revision;
        svn_opt_revision_t peg_revision;
        if( !is_revision_setup )
            if( is_svn_url( path ) )
            {
                revision = revision_url;
#if defined( PYSVN_HAS_CLIENT_PROPLIST2 )
                peg_revision = peg_revision_url;
#endif
                is_url = true;
            }
            else
            {
                revision = revision_file;
#if defined( PYSVN_HAS_CLIENT_PROPLIST2 )
                peg_revision = peg_revision_file;
#endif
            }
        else
            if( is_svn_url( path ) && !is_url )
            {
                throw Py::AttributeError( "cannot mix URL and PATH in name_path" );
            }

        try
        {
            const char *norm_path_c_str= norm_path.c_str();
            checkThreadPermission();

            PythonAllowThreads permission( m_context );

#if defined( PYSVN_HAS_CLIENT_PROPLIST4 )
            ProplistReceiveBaton proplist_baton( &permission, pool, list_of_proplists, get_inherited_props );
            svn_error_t *error = svn_client_proplist4
                (
                norm_path_c_str,
                &peg_revision,
                &revision,
                depth,
                changelists,
                get_inherited_props,
                proplist_baton.callback(),
                proplist_baton.baton(),
                m_context,
                pool
                );
#elif defined( PYSVN_HAS_CLIENT_PROPLIST3 )
            ProplistReceiveBaton proplist_baton( &permission, pool, list_of_proplists );
            svn_error_t *error = svn_client_proplist3
                (
                norm_path_c_str,
                &peg_revision,
                &revision,
                depth,
                changelists,
                proplist_baton.callback(),
                proplist_baton.baton(),
                m_context,
                pool
                );
#elif defined( PYSVN_HAS_CLIENT_PROPLIST2 )
            apr_array_header_t *props = NULL;
            svn_error_t *error = svn_client_proplist2
                (
                &props,
                norm_path_c_str,
                &peg_revision,
                &revision,
                recurse,
                m_context,
                pool
                );
#else
            apr_array_header_t *props = NULL;
            svn_error_t *error = svn_client_proplist
                (
                &props,
                norm_path_c_str,
                &revision,
                recurse,
                m_context,
                pool
                );
#endif
            permission.allowThisThread();
            if( error != NULL )
                throw SvnException( error );

#if !defined( PYSVN_HAS_CLIENT_PROPLIST3 )
            proplistToObject( list_of_proplists, props, pool );
#endif
        }
        catch( SvnException &e )
        {
            // use callback error over ClientException
            m_context.checkForError( m_module.client_error );

            throw_client_error( e );
        }
    }
    
    return list_of_proplists;
}
