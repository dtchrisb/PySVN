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

Py::Object pysvn_client::cmd_revpropdel( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_prop_name },
    { true,  name_url },
#if defined( PYSVN_HAS_CLIENT_REVPROP_SET2 )
    { false, name_original_prop_value },
#endif
    { false, name_revision },
    { false, name_force },
    { false, NULL }
    };
    FunctionArguments args( "revpropdel", args_desc, a_args, a_kws );
    args.check();

    return common_revpropset( args, false );
}

Py::Object pysvn_client::cmd_revpropset( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_prop_name },
    { true,  name_prop_value },
    { true,  name_url },
#if defined( PYSVN_HAS_CLIENT_REVPROP_SET2 )
    { false, name_original_prop_value },
#endif
    { false, name_revision },
    { false, name_force },
    { false, NULL }
    };
    FunctionArguments args( "revpropset", args_desc, a_args, a_kws );
    args.check();

    return common_revpropset( args, true );
}

Py::Object pysvn_client::common_revpropset( FunctionArguments &args, bool is_set )
{
    std::string propname( args.getUtf8String( name_prop_name ) );
    std::string propval;
    if( is_set )
    {
        propval = args.getUtf8String( name_prop_value );
    }
    std::string original_propval;
    bool has_original_propval = args.hasArgNotNone( name_original_prop_value );
    if( has_original_propval )
    {
        original_propval = args.getUtf8String( name_original_prop_value );
    }

    std::string path( args.getUtf8String( name_url ) );
    svn_opt_revision_t revision = args.getRevision( name_revision, svn_opt_revision_head );

    bool force = args.getBoolean( name_force, false );

    SvnPool pool( m_context );

    svn_revnum_t revnum = 0;
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

#if defined( PYSVN_HAS_CLIENT_REVPROP_SET2 )
        const svn_string_t *svn_original_propval = NULL;
        if( has_original_propval )
        {
            svn_original_propval = svn_string_ncreate( original_propval.c_str(), original_propval.size(), pool );
        }

        svn_error_t *error = svn_client_revprop_set2
            (
            propname.c_str(),
            svn_propval,
            svn_original_propval,
            norm_path.c_str(),
            &revision,
            &revnum,
            force,
            m_context,
            pool
            );
#else
        svn_error_t *error = svn_client_revprop_set
            (
            propname.c_str(),
            svn_propval,
            norm_path.c_str(),
            &revision,
            &revnum,
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

Py::Object pysvn_client::cmd_revpropget( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_prop_name },
    { true,  name_url },
    { false, name_revision },
    { false, NULL }
    };
    FunctionArguments args( "revpropget", args_desc, a_args, a_kws );
    args.check();

    std::string propname( args.getUtf8String( name_prop_name ) );
    std::string path( args.getUtf8String( name_url ) );
    svn_opt_revision_t revision = args.getRevision( name_revision, svn_opt_revision_head );

    SvnPool pool( m_context );

    svn_string_t *propval = NULL;
    svn_revnum_t revnum = 0;

    try
    {
        std::string norm_path( svnNormalisedIfPath( path, pool ) );

        checkThreadPermission();

        PythonAllowThreads permission( m_context );

        svn_error_t * error = svn_client_revprop_get
            (
            propname.c_str(),
            &propval,
            norm_path.c_str(),
            &revision,
            &revnum,
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

    Py::Tuple result(2);
    result[0] = Py::asObject( new pysvn_revision( svn_opt_revision_number, 0, revnum ) );
    // prop_name that is not in this rev returns a NULL value
    if( propval == NULL )
    {
        result[1] = Py::None();
    }
    else
    {
        result[1] = Py::String( propval->data, propval->len, name_utf8 );
    }

    return result;
}

Py::Object pysvn_client::cmd_revproplist( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_url },
    { false, name_revision },
    { false, NULL }
    };
    FunctionArguments args( "revproplist", args_desc, a_args, a_kws );
    args.check();

    std::string path( args.getUtf8String( name_url ) );
    svn_opt_revision_t revision = args.getRevision( name_revision, svn_opt_revision_head );

    SvnPool pool( m_context );

    apr_hash_t *props = NULL;
    svn_revnum_t revnum = 0;

    try
    {
        std::string norm_path( svnNormalisedIfPath( path, pool ) );

        checkThreadPermission();

        PythonAllowThreads permission( m_context );

        svn_error_t *error = svn_client_revprop_list
            (
            &props,
            norm_path.c_str(),
            &revision,
            &revnum,
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


    Py::Tuple result(2);
    result[0] = Py::asObject( new pysvn_revision( svn_opt_revision_number, 0, revnum ) );
    result[1] = propsToObject( props, pool );

    return result;
}
