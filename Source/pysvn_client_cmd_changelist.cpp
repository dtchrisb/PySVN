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

#ifdef PYSVN_HAS_CLIENT_ADD_TO_CHANGELIST
Py::Object pysvn_client::cmd_add_to_changelist( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_path },
    { true,  name_changelist },
    { false, name_depth },
    { false, name_changelists },
    { false, NULL }
    };
    FunctionArguments args( "add_to_changelist", args_desc, a_args, a_kws );
    args.check();

    std::string type_error_message;

    SvnPool pool( m_context );

    try
    {
        apr_array_header_t *targets = targetsFromStringOrList( args.getArg( name_path ), pool );
        std::string changelist( args.getUtf8String( name_changelist ) );

        apr_array_header_t *changelists = NULL;
        if( args.hasArg( name_changelists ) )
        {
            changelists = arrayOfStringsFromListOfStrings( args.getArg( name_changelists ), pool );
        }

        svn_depth_t depth = args.getDepth( name_depth, svn_depth_files );

        try
        {
            checkThreadPermission();

            PythonAllowThreads permission( m_context );

            svn_error_t *error = svn_client_add_to_changelist
                (
                targets,
                changelist.c_str(),
                depth,
                changelists,
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

#ifdef PYSVN_HAS_CLIENT_GET_CHANGELIST
extern "C" svn_error_t *changelistReceiver
    (
    void *baton_,
    const char *path,
    const char *changelist,
    apr_pool_t *pool
    );
class ChangelistBaton
{
public:
    ChangelistBaton( PythonAllowThreads *permission, SvnPool &pool, Py::List &changelist_list )
        : m_permission( permission )
        , m_pool( pool )
        , m_changelist_list( changelist_list )
        {}
    ~ChangelistBaton()
        {}

    svn_changelist_receiver_t callback() { return &changelistReceiver; }
    void *baton() { return static_cast< void * >( this ); }
    static ChangelistBaton *castBaton( void *baton_ ) { return static_cast<ChangelistBaton *>( baton_ ); }


    PythonAllowThreads  *m_permission;
    SvnPool             &m_pool;
    Py::List            &m_changelist_list;
};

extern "C" svn_error_t *changelistReceiver
    (
    void *baton_,
    const char *path,
    const char *changelist,
    apr_pool_t *pool
    )
{
    ChangelistBaton *baton = ChangelistBaton::castBaton( baton_ );

    PythonDisallowThreads callback_permission( baton->m_permission );

    if( path == NULL || changelist == NULL )
    {
        return NULL;
    }

    Py::Tuple values( 2 );
    values[0] = Py::String( path );
    values[1] = Py::String( changelist );

    baton->m_changelist_list.append( values );

    return NULL;
}

Py::Object pysvn_client::cmd_get_changelist( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_path },
    { false, name_depth },
    { false, name_changelists },
    { false, NULL }
    };
    FunctionArguments args( "get_changelists", args_desc, a_args, a_kws );
    args.check();

    std::string type_error_message;

    SvnPool pool( m_context );

    try
    {
        std::string path( args.getUtf8String( name_path ) );
        std::string norm_path( svnNormalisedIfPath( path, pool ) );

        apr_array_header_t *changelists = NULL;
        if( args.hasArg( name_changelists ) )
        {
            changelists = arrayOfStringsFromListOfStrings( args.getArg( name_changelists ), pool );

            //for (int j = 0; j < changelists->nelts; ++j)
            //{
            //    const char *name = ((const char **)changelists->elts)[j];
            //    std::cout << "QQQ: get changelist=" << name << std::endl;
            //}
        }

        svn_depth_t depth = args.getDepth( name_depth, svn_depth_files );

        Py::List changelist_list;

        try
        {
            checkThreadPermission();

            PythonAllowThreads permission( m_context );

            ChangelistBaton baton( &permission, pool, changelist_list );

            svn_error_t *error = svn_client_get_changelists
                (
                norm_path.c_str(),
                changelists,
                depth,
                baton.callback(),
                baton.baton(),
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

        return changelist_list;
    }
    catch( Py::TypeError & )
    {
        throw Py::TypeError( type_error_message );
    }

    return Py::None();
}
#endif

#ifdef PYSVN_HAS_CLIENT_REMOVE_FROM_CHANGELISTS
Py::Object pysvn_client::cmd_remove_from_changelists( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_path },
    { false, name_depth },
    { false, name_changelists },
    { false, NULL }
    };
    FunctionArguments args( "remove_from_changelists", args_desc, a_args, a_kws );
    args.check();

    std::string type_error_message;

    SvnPool pool( m_context );

    try
    {
        apr_array_header_t *targets = targetsFromStringOrList( args.getArg( name_path ), pool );

        apr_array_header_t *changelists = NULL;
        if( args.hasArg( name_changelists ) )
        {
            changelists = arrayOfStringsFromListOfStrings( args.getArg( name_changelists ), pool );
        }

        svn_depth_t depth = args.getDepth( name_depth, svn_depth_files );

        try
        {
            checkThreadPermission();

            PythonAllowThreads permission( m_context );

            svn_error_t *error = svn_client_remove_from_changelists
                (
                targets,
                depth,
                changelists,
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
