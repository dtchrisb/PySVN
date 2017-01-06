//
// ====================================================================
// (c) 2003-2009 Barry A Scott.  All rights reserved.
//
// This software is licensed as described in the file LICENSE.txt,
// which you should have received as part of this distribution.
//
// ====================================================================
//
//
//  pysvn_client.cpp
//

#if defined( _MSC_VER )
// disable warning C4786: symbol greater than 255 character,
// nessesary to ignore as <map> causes lots of warning
#pragma warning(disable: 4786)
#endif

#include "pysvn.hpp"
#include "pysvn_docs.hpp"
#include "pysvn_svnenv.hpp"
#include "svn_path.h"
#include "svn_config.h"
#include "svn_sorts.h"
#include "pysvn_static_strings.hpp"

static const char *g_utf_8 = "utf-8";

static void init_py_names()
{
    static bool init_done = false;
    if( init_done )
    {
        return;
    }

    py_name_callback_cancel = new Py::String( name_callback_cancel );
    py_name_callback_conflict_resolver = new Py::String( name_callback_conflict_resolver );
    py_name_callback_get_log_message = new Py::String( name_callback_get_log_message );
    py_name_callback_get_login = new Py::String( name_callback_get_login );
    py_name_callback_notify = new Py::String( name_callback_notify );
    py_name_callback_ssl_client_cert_password_prompt = new Py::String( name_callback_ssl_client_cert_password_prompt );
    py_name_callback_ssl_client_cert_prompt = new Py::String( name_callback_ssl_client_cert_prompt );
    py_name_callback_ssl_server_prompt = new Py::String( name_callback_ssl_server_prompt );
    py_name_callback_ssl_server_trust_prompt = new Py::String( name_callback_ssl_server_trust_prompt );
    py_name_commit_info_style = new Py::String( name_commit_info_style );
    py_name_created_rev = new Py::String( name_created_rev );
    py_name_exception_style = new Py::String( name_exception_style );
    py_name_has_props = new Py::String( name_has_props );
    py_name_kind = new Py::String( name_kind );
    py_name_last_author = new Py::String( name_last_author );
    py_name_lock = new Py::String( name_lock );
    py_name_name = new Py::String( name_name );
    py_name_node_kind = new Py::String( name_node_kind );
    py_name_path = new Py::String( name_path );
    py_name_prop_changed = new Py::String( name_prop_changed );
    py_name_repos_path = new Py::String( name_repos_path );
    py_name_size = new Py::String( name_size );
    py_name_summarize_kind = new Py::String( name_summarize_kind );
    py_name_time = new Py::String( name_time );

    init_done = true;
}

//--------------------------------------------------------------------------------
#if defined( PYSVN_HAS_CLIENT_STATUS_T )
std::string name_wrapper_status2("PysvnStatus2");
#endif
std::string name_wrapper_status("PysvnStatus");
std::string name_wrapper_entry("PysvnEntry");
std::string name_wrapper_info("PysvnInfo");
std::string name_wrapper_lock("PysvnLock");
std::string name_wrapper_list("PysvnList");
std::string name_wrapper_log("PysvnLog");
std::string name_wrapper_log_changed_path("PysvnLogChangedPath");
std::string name_wrapper_dirent("PysvnDirent");
std::string name_wrapper_wc_info("PysvnWcInfo");
std::string name_wrapper_diff_summary("PysvnDiffSummary");
std::string name_wrapper_commit_info("PysvnCommitInfo");

pysvn_client::pysvn_client
    (
    pysvn_module &_module,
    const std::string &config_dir,
    Py::Dict result_wrappers
    )
: m_module( _module )
, m_result_wrappers( result_wrappers )
, m_context( config_dir )
, m_exception_style( 0 )
, m_commit_info_style( 0 )
#if defined( PYSVN_HAS_CLIENT_STATUS_T )
, m_wrapper_status2( result_wrappers, name_wrapper_status2 )
#endif
, m_wrapper_status( result_wrappers, name_wrapper_status )
, m_wrapper_entry( result_wrappers, name_wrapper_entry )
, m_wrapper_info( result_wrappers, name_wrapper_info )
, m_wrapper_lock( result_wrappers, name_wrapper_lock )
, m_wrapper_list( result_wrappers, name_wrapper_list )
, m_wrapper_log( result_wrappers, name_wrapper_log )
, m_wrapper_log_changed_path( result_wrappers, name_wrapper_log_changed_path )
, m_wrapper_dirent( result_wrappers, name_wrapper_dirent )
, m_wrapper_wc_info( result_wrappers, name_wrapper_wc_info )
, m_wrapper_diff_summary( result_wrappers, name_wrapper_diff_summary )
, m_wrapper_commit_info( result_wrappers, name_wrapper_commit_info )
{
    init_py_names();
}

pysvn_client::~pysvn_client()
{
}

Py::Object pysvn_client::getattr( const char *_name )
{
    std::string name( _name );

    // std::cout << "getattr( " << name << " )" << std::endl << std::flush;
    
    if( name == name___members__ )
    {
        Py::List members;

        members.append( *py_name_callback_get_login );
        members.append( *py_name_callback_notify );
        members.append( *py_name_callback_cancel );
        members.append( *py_name_callback_conflict_resolver );
        members.append( *py_name_callback_get_log_message );
        members.append( *py_name_callback_ssl_server_prompt );
        members.append( *py_name_callback_ssl_server_trust_prompt );
        members.append( *py_name_callback_ssl_client_cert_prompt );
        members.append( *py_name_callback_ssl_client_cert_password_prompt );

        members.append( *py_name_exception_style );
        members.append( *py_name_commit_info_style );

        return members;
    }

    if( name == name_callback_get_login )
        return m_context.m_pyfn_GetLogin;

    if( name == name_callback_notify )
        return m_context.m_pyfn_Notify;

#if defined( PYSVN_HAS_CONTEXT_PROGRESS )
    if( name == name_callback_progress )
        return m_context.m_pyfn_Progress;
#endif

#if defined( PYSVN_HAS_SVN_CLIENT_CTX_T__CONFLICT_FUNC )
    if( name == name_callback_conflict_resolver )
        return m_context.m_pyfn_ConflictResolver;
#endif

    if( name == name_callback_cancel )
        return m_context.m_pyfn_Cancel;

    if( name == name_callback_get_log_message )
        return m_context.m_pyfn_GetLogMessage;

    if( name == name_callback_ssl_server_prompt )
        return m_context.m_pyfn_SslServerPrompt;

    if( name == name_callback_ssl_server_trust_prompt )
        return m_context.m_pyfn_SslServerTrustPrompt;

    if( name == name_callback_ssl_client_cert_prompt )
        return m_context.m_pyfn_SslClientCertPrompt;

    if( name == name_callback_ssl_client_cert_password_prompt )
        return m_context.m_pyfn_SslClientCertPwPrompt;

    if( name == name_callback_ssl_client_cert_password_prompt )
        return m_context.m_pyfn_SslClientCertPwPrompt;

    if( name == name_exception_style )
        return Py::Int( m_exception_style );

    if( name == name_commit_info_style )
        return Py::Int( m_commit_info_style );

    return getattr_default( _name );
}

static bool set_callable( Py::Object &callback, const Py::Object &value )
{
    if( value.isCallable() )
    {
        callback = value;
        return true;
    }

    else
    if( value.is( Py::None() ) )
    {
        callback = value;
        return false;
    }

    else
    {
        throw Py::AttributeError( "expecting None or a callable object" );
    }
}

int pysvn_client::setattr( const char *_name, const Py::Object &value )
{
    std::string name( _name );
    if( name == name_callback_get_login )
        set_callable( m_context.m_pyfn_GetLogin, value );

    else if( name == name_callback_notify )
        m_context.installNotify( set_callable( m_context.m_pyfn_Notify, value ) );

#if defined( PYSVN_HAS_CONTEXT_PROGRESS )
    else if( name == name_callback_progress )
        m_context.installProgress( set_callable( m_context.m_pyfn_Progress, value ) );
#endif

#if defined( PYSVN_HAS_SVN_CLIENT_CTX_T__CONFLICT_FUNC )
    else if( name == name_callback_conflict_resolver )
        m_context.installConflictResolver( set_callable( m_context.m_pyfn_ConflictResolver, value ) );
#endif

    else if( name == name_callback_cancel )
        m_context.installCancel( set_callable( m_context.m_pyfn_Cancel, value ) );

    else if( name == name_callback_get_log_message )
        set_callable( m_context.m_pyfn_GetLogMessage, value );

    else if( name == name_callback_ssl_server_prompt )
        set_callable( m_context.m_pyfn_SslServerPrompt, value );

    else if( name == name_callback_ssl_server_trust_prompt )
        set_callable( m_context.m_pyfn_SslServerTrustPrompt, value );

    else if( name == name_callback_ssl_client_cert_prompt )
        set_callable( m_context.m_pyfn_SslClientCertPrompt, value );

    else if( name == name_callback_ssl_client_cert_password_prompt )
        set_callable( m_context.m_pyfn_SslClientCertPwPrompt, value );

    else if( name == name_exception_style )
    {
        Py::Int style( value );
        if( style == 0l || style == 1l )
        {
            m_exception_style = style;
        }
            else
        {
            throw Py::AttributeError( "exception_style value must be 0 or 1" );
        }
    }

    else if( name == name_commit_info_style )
    {
        Py::Int style( value );
        if( style == 0l || style == 1l || style == 2l )
        {
            m_commit_info_style = style;
        }
            else
        {
            throw Py::AttributeError( "commit_info_style value must be 0, 1 or 2" );
        }
    }

    else
    {
        std::string msg( "Unknown attribute: " );
        msg += name;
        throw Py::AttributeError( msg );
    }

    return 0;
}


Py::Object pysvn_client::helper_boolean_auth_set( FunctionArguments &a_args, const char *a_arg_name, const char *a_param_name )
{
    a_args.check();

    bool enable( a_args.getBoolean( a_arg_name ) );
    try
    {
        void *param = 0;
        if( !enable )
            param = (void *)"1";

        svn_auth_set_parameter
            (
            m_context.ctx()->auth_baton,
            a_param_name,
            param
            );
    }
    catch( SvnException &e )
    {
        // use callback error over ClientException
        m_context.checkForError( m_module.client_error );

        throw_client_error( e );
    }

    return Py::None();
}

Py::Object pysvn_client::helper_boolean_auth_get( FunctionArguments &a_args, const char *a_param_name )
{
    a_args.check();

    char *param = NULL;
    try
    {
        param = (char *)svn_auth_get_parameter
            (
            m_context.ctx()->auth_baton,
            a_param_name
            );
    }
    catch( SvnException &e )
    {
        // use callback error over ClientException
        m_context.checkForError( m_module.client_error );

        throw_client_error( e );
    }

    bool not_set = param != NULL && param[0] == '1';
    if( not_set )
        return Py::Int( 0 );
    return Py::Int( 1 );
}

Py::Object pysvn_client::helper_string_auth_set
    (
    FunctionArguments &a_args,
    const char *a_arg_name,
    const char *a_param_name,
    std::string &ctx_str
    )
{
    a_args.check();

    const char *param = NULL;
    Py::Object param_obj( a_args.getArg( a_arg_name ) );
    if( !param_obj.is( Py::None() ) )
    {
        Py::String param_str( param_obj );
        ctx_str = param_str.as_std_string( g_utf_8 );
        param = ctx_str.c_str();
    }

    try
    {
        svn_auth_set_parameter
            (
            m_context.ctx()->auth_baton,
            a_param_name,
            param
            );
    }
    catch( SvnException &e )
    {
        // use callback error over ClientException
        m_context.checkForError( m_module.client_error );

        throw_client_error( e );
    }

    return Py::None();
}

Py::Object pysvn_client::helper_string_auth_get( FunctionArguments &a_args, const char *a_param_name )
{
    a_args.check();

    char *param = NULL;
    try
    {
        param = (char *)svn_auth_get_parameter
            (
            m_context.ctx()->auth_baton,
            a_param_name
            );
    }
    catch( SvnException &e )
    {
        // use callback error over ClientException
        m_context.checkForError( m_module.client_error );

        throw_client_error( e );
    }

    if( param != NULL )
        return Py::String( param );

    return Py::None();
}

#if defined( PYSVN_HAS_WC_ADM_DIR )
Py::Object pysvn_client::get_adm_dir( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { false, NULL }
    };
    FunctionArguments args( "get_adm_dir", args_desc, a_args, a_kws );
    args.check();

    const char *name = NULL;

    try
    {
        name = svn_wc_get_adm_dir
            (
            m_context.getContextPool()
            );
    }
    catch( SvnException &e )
    {
        // use callback error over ClientException
        m_context.checkForError( m_module.client_error );

        throw_client_error( e );
    }

    return Py::String( name );
}
#endif

Py::Object pysvn_client::get_auth_cache( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { false, NULL }
    };
    FunctionArguments args( "get_auth_cache", args_desc, a_args, a_kws );

    return helper_boolean_auth_get( args, SVN_AUTH_PARAM_NO_AUTH_CACHE );
}

Py::Object pysvn_client::get_interactive( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { false, NULL }
    };
    FunctionArguments args( "get_interactive", args_desc, a_args, a_kws );

    return helper_boolean_auth_get( args, SVN_AUTH_PARAM_NON_INTERACTIVE );
}

Py::Object pysvn_client::get_store_passwords( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { false, NULL }
    };
    FunctionArguments args( "get_store_passwords", args_desc, a_args, a_kws );

    return helper_boolean_auth_get( args, SVN_AUTH_PARAM_DONT_STORE_PASSWORDS );
}

Py::Object pysvn_client::get_default_username( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { false, NULL }
    };
    FunctionArguments args( "get_default_username", args_desc, a_args, a_kws );

    return helper_string_auth_get( args, SVN_AUTH_PARAM_DEFAULT_USERNAME );
}

Py::Object pysvn_client::get_default_password( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { false, NULL }
    };
    FunctionArguments args( "get_default_password", args_desc, a_args, a_kws );

    return helper_string_auth_get( args, SVN_AUTH_PARAM_DEFAULT_PASSWORD );
}

#if defined( PYSVN_HAS_WC_ADM_DIR )
Py::Object pysvn_client::set_adm_dir( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_name },
    { false, NULL }
    };
    FunctionArguments args( "set_adm_dir", args_desc, a_args, a_kws );

    args.check();

    std::string name( args.getBytes( name_name ) );

    try
    {
        svn_wc_set_adm_dir
            (
            name.c_str(),
            m_context.getContextPool()
            );
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

Py::Object pysvn_client::set_auth_cache( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_enable },
    { false, NULL }
    };
    FunctionArguments args( "set_auth_cache", args_desc, a_args, a_kws );

    return helper_boolean_auth_set( args, name_enable, SVN_AUTH_PARAM_NO_AUTH_CACHE );
}

Py::Object pysvn_client::set_interactive( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_enable },
    { false, NULL }
    };
    FunctionArguments args( "set_interactive", args_desc, a_args, a_kws );

    return helper_boolean_auth_set( args, name_enable, SVN_AUTH_PARAM_NON_INTERACTIVE );
}

Py::Object pysvn_client::set_store_passwords( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_enable },
    { false, NULL }
    };
    FunctionArguments args( "set_store_passwords", args_desc, a_args, a_kws );

    return helper_boolean_auth_set( args, name_enable, SVN_AUTH_PARAM_DONT_STORE_PASSWORDS );
}

Py::Object pysvn_client::set_default_username( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_username },
    { false, NULL }
    };
    FunctionArguments args( "set_default_username", args_desc, a_args, a_kws );

    return helper_string_auth_set( args, name_username, SVN_AUTH_PARAM_DEFAULT_USERNAME, m_context.m_default_username );
}

Py::Object pysvn_client::set_default_password( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_password },
    { false, NULL }
    };
    FunctionArguments args( "set_default_password", args_desc, a_args, a_kws );

    return helper_string_auth_set( args, name_password, SVN_AUTH_PARAM_DEFAULT_PASSWORD, m_context.m_default_password );
}

Py::Object pysvn_client::get_auto_props( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { false, NULL }
    };
    FunctionArguments args( "get_auto_props", args_desc, a_args, a_kws );
    args.check();

    svn_boolean_t enable = false;
    try
    {
        svn_config_t *cfg = (svn_config_t *)apr_hash_get
            (
            m_context.ctx()->config,
            SVN_CONFIG_CATEGORY_CONFIG,
            APR_HASH_KEY_STRING
            );
        svn_error_t *error = svn_config_get_bool
            (
            cfg,
            &enable,
            SVN_CONFIG_SECTION_MISCELLANY,
            SVN_CONFIG_OPTION_ENABLE_AUTO_PROPS,
            enable
            );
        if( error != NULL )
            throw SvnException( error );
    }
    catch( SvnException &e )
    {
        // use callback error over ClientException
        m_context.checkForError( m_module.client_error );

        throw_client_error( e );
    }

    return Py::Int( enable );
}

Py::Object pysvn_client::set_auto_props( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_enable },
    { false, NULL }
    };
    FunctionArguments args( "set_auto_props", args_desc, a_args, a_kws );
    args.check();

    bool enable( args.getBoolean( name_enable ) );
    try
    {
        svn_config_t *cfg = (svn_config_t *)apr_hash_get
            (
            m_context.ctx()->config,
            SVN_CONFIG_CATEGORY_CONFIG,
            APR_HASH_KEY_STRING
            );
        svn_config_set_bool
            (
            cfg,
            SVN_CONFIG_SECTION_MISCELLANY,
            SVN_CONFIG_OPTION_ENABLE_AUTO_PROPS,
            enable
            );
    }
    catch( SvnException &e )
    {
        // use callback error over ClientException
        m_context.checkForError( m_module.client_error );

        throw_client_error( e );
    }

    return Py::None();
}

#if defined( PYSVN_HAS_WC_ADM_DIR )
Py::Object pysvn_client::is_adm_dir( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_name },
    { false, NULL }
    };
    FunctionArguments args( "is_adm_dir", args_desc, a_args, a_kws );

    args.check();

    std::string name( args.getBytes( name_name ) );

    svn_boolean_t name_is_adm_dir = 0;
    try
    {
        name_is_adm_dir = svn_wc_is_adm_dir
            (
            name.c_str(),
            m_context.getContextPool()
            );
    }
    catch( SvnException &e )
    {
        // use callback error over ClientException
        m_context.checkForError( m_module.client_error );

        throw_client_error( e );
    }

    return Py::Int( name_is_adm_dir );
}
#endif

Py::Object pysvn_client::is_url( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_url },
    { false, NULL }
    };
    FunctionArguments args( "is_url", args_desc, a_args, a_kws );
    args.check();

    Py::String path( args.getUtf8String( name_url ) );

    Py::Int result( is_svn_url( path ) );
    return result;
}

#if defined( PYSVN_HAS_CLIENT_ROOT_URL_FROM_PATH )
Py::Object pysvn_client::cmd_root_url_from_path( const Py::Tuple& a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_url_or_path },
    { false, NULL }
    };
    FunctionArguments args( "root_url_from_path", args_desc, a_args, a_kws );

    args.check();

    std::string path( args.getUtf8String( name_url_or_path ) );

    SvnPool pool( m_context );
    const char *root_url = NULL;
    try
    {
        std::string norm_path( svnNormalisedIfPath( path, pool ) );

        checkThreadPermission();

        PythonAllowThreads permission( m_context );

        // known to call back into pysvn
#if defined( PYSVN_HAS_CLIENT_GET_REPOS_ROOT )
        const char *repos_uuid = NULL;
        svn_error_t *error = svn_client_get_repos_root
            (
            &root_url,
            &repos_uuid,
            norm_path.c_str(),
            m_context,
            pool,
            pool
            );
#else
        svn_error_t *error = svn_client_root_url_from_path
            (
            &root_url,
            norm_path.c_str(),
            m_context,
            pool
            );
#endif
        if( error != NULL )
            throw SvnException( error );
    }
    catch( SvnException &e )
    {
        // use callback error over ClientException
        m_context.checkForError( m_module.client_error );

        throw_client_error( e );
    }

    return Py::String( root_url );
}
#endif

// check that we are not in use on another thread
void pysvn_client::checkThreadPermission()
{
    if( m_context.hasPermission() )
    {
        throw Py::Exception( m_module.client_error,
            "client in use on another thread" );
    }
}

void pysvn_client::throw_client_error( SvnException &e )
{
    throw Py::Exception(
        m_module.client_error,
        e.pythonExceptionArg( m_exception_style ) );
}

void revisionKindCompatibleCheck
    (
    bool is_url,
    const svn_opt_revision_t &revision,
    const char *revision_name,
    const char *url_or_path_name
    )
{
    std::string message;
    if( is_url )
    {
        // URL compatibility
        switch( revision.kind )
        {
        case svn_opt_revision_number:
        case svn_opt_revision_date:
        case svn_opt_revision_committed:
        case svn_opt_revision_previous:
        case svn_opt_revision_head:
        case svn_opt_revision_unspecified:
            break;

        case svn_opt_revision_working:
        case svn_opt_revision_base:
        default:
            message += revision_name;
            message += " is not compatible with URL ";
            message += url_or_path_name;
            throw Py::AttributeError( message );
        }
    }
#if defined( there_are_any_checks_for_path )
    else
    {
        // PATH compatibility
        switch( revision.kind )
        {
        case svn_opt_revision_working:
        case svn_opt_revision_base:
        case svn_opt_revision_unspecified:
            break;

        case svn_opt_revision_number:
        case svn_opt_revision_date:
        case svn_opt_revision_committed:
        case svn_opt_revision_previous:
        case svn_opt_revision_head:
        default:
            message += revision_name;
            message += " is not compatible with path ";
            message += url_or_path_name;
            throw Py::AttributeError( message );
        }
    }
#endif
}

void pysvn_client::init_type()
{
    behaviors().name("Client");
    behaviors().doc( pysvn_client_doc );
    behaviors().supportGetattr();
    behaviors().supportSetattr();

    add_keyword_method("add", &pysvn_client::cmd_add, pysvn_client_add_doc );
#if defined( PYSVN_HAS_CLIENT_ADD_TO_CHANGELIST )
    add_keyword_method("add_to_changelist", &pysvn_client::cmd_add_to_changelist, pysvn_client_add_to_changelist_doc );
#endif
    add_keyword_method("annotate", &pysvn_client::cmd_annotate, pysvn_client_annotate_doc );
#if defined( PYSVN_HAS_CLIENT_ANNOTATE5 )
    add_keyword_method("annotate2", &pysvn_client::cmd_annotate2, pysvn_client_annotate2_doc );
#endif
    add_keyword_method("cat", &pysvn_client::cmd_cat, pysvn_client_cat_doc );
    add_keyword_method("checkin", &pysvn_client::cmd_checkin, pysvn_client_checkin_doc );
    add_keyword_method("checkout", &pysvn_client::cmd_checkout, pysvn_client_checkout_doc );
    add_keyword_method("cleanup", &pysvn_client::cmd_cleanup, pysvn_client_cleanup_doc );
    add_keyword_method("copy", &pysvn_client::cmd_copy, pysvn_client_copy_doc );
#if defined( PYSVN_HAS_CLIENT_COPY4 )
    add_keyword_method("copy2", &pysvn_client::cmd_copy2, pysvn_client_copy2_doc );
#endif
    add_keyword_method("diff", &pysvn_client::cmd_diff, pysvn_client_diff_doc );
#if defined( PYSVN_HAS_CLIENT_DIFF_PEG )
    add_keyword_method("diff_peg", &pysvn_client::cmd_diff_peg, pysvn_client_diff_peg_doc );
#endif
#if defined( PYSVN_HAS_CLIENT_DIFF_SUMMARIZE )
    add_keyword_method("diff_summarize", &pysvn_client::cmd_diff_summarize, pysvn_client_diff_summarize_doc );
    add_keyword_method("diff_summarize_peg", &pysvn_client::cmd_diff_summarize_peg, pysvn_client_diff_summarize_peg_doc );
#endif
    add_keyword_method("export", &pysvn_client::cmd_export, pysvn_client_export_doc );
#if defined( PYSVN_HAS_CLIENT_GET_CHANGELIST )
    add_keyword_method("get_changelist", &pysvn_client::cmd_get_changelist, pysvn_client_get_changelist_doc );
#endif
#if defined( PYSVN_HAS_WC_ADM_DIR )
    add_keyword_method("get_adm_dir", &pysvn_client::get_adm_dir, pysvn_client_get_adm_dir_doc );
#endif
    add_keyword_method("get_auth_cache", &pysvn_client::get_auth_cache, pysvn_client_get_auth_cache_doc );
    add_keyword_method("get_auto_props", &pysvn_client::get_auto_props, pysvn_client_get_auto_props_doc );
    add_keyword_method("get_default_password", &pysvn_client::get_default_password, pysvn_client_get_default_password_doc );
    add_keyword_method("get_default_username", &pysvn_client::get_default_username, pysvn_client_get_default_username_doc );
    add_keyword_method("get_interactive", &pysvn_client::get_interactive, pysvn_client_get_interactive_doc );
    add_keyword_method("get_store_passwords", &pysvn_client::get_store_passwords, pysvn_client_get_store_passwords_doc );
    add_keyword_method("import_", &pysvn_client::cmd_import, pysvn_client_import__doc );
    add_keyword_method("info", &pysvn_client::cmd_info, pysvn_client_info_doc );
#if defined( PYSVN_HAS_CLIENT_INFO )
    add_keyword_method("info2", &pysvn_client::cmd_info2, pysvn_client_info2_doc );
#endif
#if defined( PYSVN_HAS_WC_ADM_DIR )
    add_keyword_method("is_adm_dir", &pysvn_client::is_adm_dir, pysvn_client_is_adm_dir_doc );
#endif
    add_keyword_method("is_url", &pysvn_client::is_url, pysvn_client_is_url_doc );
#if defined( PYSVN_HAS_CLIENT_LOCK )
    add_keyword_method("lock", &pysvn_client::cmd_lock, pysvn_client_lock_doc );
#endif
    add_keyword_method("log", &pysvn_client::cmd_log, pysvn_client_log_doc );
#if defined( PYSVN_HAS_CLIENT_LIST )
    add_keyword_method("list", &pysvn_client::cmd_list, pysvn_client_list_doc );
#endif
    add_keyword_method("ls", &pysvn_client::cmd_ls, pysvn_client_ls_doc );
    add_keyword_method("merge", &pysvn_client::cmd_merge, pysvn_client_merge_doc );
#if defined( PYSVN_HAS_CLIENT_MERGE_PEG )
    add_keyword_method("merge_peg", &pysvn_client::cmd_merge_peg, pysvn_client_merge_peg_doc );
#endif
#if defined( PYSVN_HAS_CLIENT_MERGE_PEG3 )
    add_keyword_method("merge_peg2", &pysvn_client::cmd_merge_peg2, pysvn_client_merge_peg2_doc );
#endif
#if defined( PYSVN_HAS_CLIENT_MERGE_REINTEGRATE )
    add_keyword_method("merge_reintegrate", &pysvn_client::cmd_merge_reintegrate, pysvn_client_merge_reintegrate_doc );
#endif
    add_keyword_method("mkdir", &pysvn_client::cmd_mkdir, pysvn_client_mkdir_doc );
#if defined( PYSVN_HAS_CLIENT_MOVE5 )
    add_keyword_method("move2", &pysvn_client::cmd_move2, pysvn_client_move2_doc );
#endif
    add_keyword_method("move", &pysvn_client::cmd_move, pysvn_client_move_doc );
#if defined( PYSVN_HAS_CLIENT_PATCH )
    add_keyword_method("patch", &pysvn_client::cmd_patch, pysvn_client_patch_doc );
#endif
    add_keyword_method("propdel", &pysvn_client::cmd_propdel, pysvn_client_propdel_doc );
    add_keyword_method("propget", &pysvn_client::cmd_propget, pysvn_client_propget_doc );
    add_keyword_method("proplist", &pysvn_client::cmd_proplist, pysvn_client_proplist_doc );
    add_keyword_method("propset", &pysvn_client::cmd_propset, pysvn_client_propset_doc );
#if defined( PYSVN_HAS_CLIENT_PROPSET_LOCAL )
    add_keyword_method("propdel_local", &pysvn_client::cmd_propdel_local, pysvn_client_propdel_local_doc );
    add_keyword_method("propset_local", &pysvn_client::cmd_propset_local, pysvn_client_propset_local_doc );
#endif
#if defined( PYSVN_HAS_CLIENT_PROPSET_REMOTE )
    add_keyword_method("propdel_remote", &pysvn_client::cmd_propdel_remote, pysvn_client_propdel_remote_doc );
    add_keyword_method("propset_remote", &pysvn_client::cmd_propset_remote, pysvn_client_propset_remote_doc );
#endif
    add_keyword_method("relocate", &pysvn_client::cmd_relocate, pysvn_client_relocate_doc );
    add_keyword_method("remove", &pysvn_client::cmd_remove, pysvn_client_remove_doc );
#if defined( PYSVN_HAS_CLIENT_REMOVE_FROM_CHANGELISTS )
    add_keyword_method("remove_from_changelists", &pysvn_client::cmd_remove_from_changelists, pysvn_client_remove_from_changelists_doc );
#endif
    add_keyword_method("resolved", &pysvn_client::cmd_resolved, pysvn_client_resolved_doc );
    add_keyword_method("revert", &pysvn_client::cmd_revert, pysvn_client_revert_doc );
    add_keyword_method("revpropdel", &pysvn_client::cmd_revpropdel, pysvn_client_revpropdel_doc );
    add_keyword_method("revpropget", &pysvn_client::cmd_revpropget, pysvn_client_revpropget_doc );
    add_keyword_method("revproplist", &pysvn_client::cmd_revproplist, pysvn_client_revproplist_doc );
    add_keyword_method("revpropset", &pysvn_client::cmd_revpropset, pysvn_client_revpropset_doc );
#if defined( PYSVN_HAS_CLIENT_ROOT_URL_FROM_PATH )
    add_keyword_method("root_url_from_path", &pysvn_client::cmd_root_url_from_path, pysvn_client_root_url_from_path_doc );
#endif
#if defined( PYSVN_HAS_WC_ADM_DIR )
    add_keyword_method("set_adm_dir", &pysvn_client::set_adm_dir, pysvn_client_set_adm_dir_doc );
#endif
    add_keyword_method("set_auth_cache", &pysvn_client::set_auth_cache, pysvn_client_set_auth_cache_doc );
    add_keyword_method("set_auto_props", &pysvn_client::set_auto_props, pysvn_client_set_auto_props_doc );
    add_keyword_method("set_default_password", &pysvn_client::set_default_password, pysvn_client_set_default_password_doc );
    add_keyword_method("set_default_username", &pysvn_client::set_default_username, pysvn_client_set_default_username_doc );
    add_keyword_method("set_interactive", &pysvn_client::set_interactive, pysvn_client_set_interactive_doc );
    add_keyword_method("set_store_passwords", &pysvn_client::set_store_passwords, pysvn_client_set_store_passwords_doc );
#if defined( PYSVN_HAS_CLIENT_STATUS5 )
    add_keyword_method("status2", &pysvn_client::cmd_status2, pysvn_client_status2_doc );
#endif
    add_keyword_method("status", &pysvn_client::cmd_status, pysvn_client_status_doc );
    add_keyword_method("switch", &pysvn_client::cmd_switch, pysvn_client_switch_doc );
#if defined( PYSVN_HAS_CLIENT_LOCK )
    add_keyword_method("unlock", &pysvn_client::cmd_unlock, pysvn_client_unlock_doc );
#endif
#if defined( PYSVN_HAS_CLIENT_UPGRADE )
    add_keyword_method("upgrade", &pysvn_client::cmd_upgrade, pysvn_client_upgrade_doc );
#endif
    add_keyword_method("update", &pysvn_client::cmd_update, pysvn_client_update_doc );
#if defined( PYSVN_HAS_CLIENT_VACUUM )
    add_keyword_method("vacuum", &pysvn_client::cmd_vacuum, pysvn_client_vacuum_doc );
#endif
}
