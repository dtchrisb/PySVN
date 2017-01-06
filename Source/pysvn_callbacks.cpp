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
//  pysvn_callbacks.cpp
//
#if defined( _MSC_VER )
// disable warning C4786: symbol greater than 255 character,
// nessesary to ignore as <map> causes lots of warning
#pragma warning(disable: 4786)
#endif

#include "pysvn.hpp"
#include "pysvn_static_strings.hpp"

#if defined( PYSVN_TRACE_CALLBACK )
#define TRACE_CALLBACK( name ) do { std::cerr << "Debug: Callback " << #name << std::endl; } while( false )
#else
#define TRACE_CALLBACK( name ) do { } while( false )
#endif


static const char *g_utf_8 = "utf-8";

static bool get_string( Py::Object &fn, Py::Tuple &args, std::string &_msg );

pysvn_context::pysvn_context( const std::string &config_dir )
: SvnContext( config_dir )
, m_pyfn_GetLogin()
, m_pyfn_Notify()
#if defined( PYSVN_HAS_CONTEXT_PROGRESS )
, m_pyfn_Progress()
#endif
#if defined( PYSVN_HAS_SVN_CLIENT_CTX_T__CONFLICT_FUNC )
, m_pyfn_ConflictResolver()
#endif
, m_pyfn_Cancel()
, m_pyfn_GetLogMessage()
, m_pyfn_SslServerPrompt()
, m_pyfn_SslServerTrustPrompt()
, m_pyfn_SslClientCertPrompt()
, m_pyfn_SslClientCertPwPrompt()
, m_permission( NULL )
, m_error_message()
, m_log_message()
{
}


pysvn_context::~pysvn_context()
{
}

bool pysvn_context::hasPermission()
{
    return m_permission != NULL;
}

void pysvn_context::setPermission( PythonAllowThreads &_permission )
{
    assert( m_permission == NULL );
    m_permission = &_permission;
    m_error_message = "";
}

void pysvn_context::clearPermission()
{
    m_permission = NULL;
}

void pysvn_context::checkForError( Py::ExtensionExceptionType &exception_for_error )
{
    // see if any errors occurred in the callbacks
    if( !m_error_message.empty() )
    {
        throw Py::Exception( exception_for_error, m_error_message );
    }
}

//
// this method will be called to retrieve
// authentication information
//
// WORKAROUND FOR apr_xlate PROBLEM: 
// STRINGS ALREADY HAVE TO BE UTF8!!!
//
// @retval true continue
//
bool pysvn_context::contextGetLogin
    (
    const std::string & _realm,
    std::string & _username, 
    std::string & _password,
    bool &_may_save
    )
{
    TRACE_CALLBACK( contextGetLogin );

    PythonDisallowThreads callback_permission( m_permission );

    // make sure we can call the users object
    if( !m_pyfn_GetLogin.isCallable() )
    {
        m_error_message = "callback_get_login required";
        return false;
    }

    Py::Callable callback( m_pyfn_GetLogin );

    Py::Tuple args( 3 );
    args[0] = Py::String( _realm );
    args[1] = Py::String( _username );
    args[2] = Py::Int( (long)_may_save );

    // bool, username, password
    Py::Tuple results;
    Py::Int retcode;
    Py::String username;
    Py::String password;
    Py::Int may_save_out;

    try
    {
        results = callback.apply( args );
        retcode = results[0];
        username = results[1];
        password = results[2];
        may_save_out = results[3];

        // true returned
        if( long( retcode ) != 0 )
        {
            // copy out the answers
            _username = username.as_std_string( g_utf_8 );
            _password = password.as_std_string( g_utf_8 );
            _may_save = long( may_save_out ) != 0;

            return true;
        }
    }
    catch( Py::Exception &e )
    {
        PyErr_Print();
        e.clear();

        m_error_message = "unhandled exception in callback_get_login";

        return false;
    }

    return false;
}

#if defined( PYSVN_HAS_CONTEXT_PROGRESS )
void pysvn_context::contextProgress
    (
    apr_off_t progress,
    apr_off_t total
    )
{
    TRACE_CALLBACK( contextProgress );

    PythonDisallowThreads callback_permission( m_permission );

    // make sure we can call the users object
    if( !m_pyfn_Progress.isCallable() )
        return;

    Py::Callable callback( m_pyfn_Progress );

    Py::Tuple args( 2 );
    // on some platforms apr_off_t is int64
    args[0] = Py::Int( static_cast<long int>( progress ) );
    args[1] = Py::Int( static_cast<long int>( total ) );

    Py::Object results;

    try
    {
        results = callback.apply( args );
    }
    catch( Py::Exception &e )
    {
        PyErr_Print();
        e.clear();

        m_error_message = "unhandled exception in callback_progress";
    }
}
#endif


#if defined( PYSVN_HAS_SVN_CLIENT_CTX_T__CONFLICT_FUNC )

#if defined( PYSVN_HAS_SVN_CLIENT_CTX_T__CONFLICT_FUNC_1_6 )
Py::Object toConflictVersion( const svn_wc_conflict_version_t *version )
{
    if( version == NULL )
        return Py::None();

    Py::Dict ver;
    ver["repos_url"] = utf8_string_or_none( version->repos_url );
    ver["peg_rev"] = Py::asObject( new pysvn_revision( svn_opt_revision_number, 0, version->peg_rev ) );
    ver["path_in_repos"] = utf8_string_or_none( version->path_in_repos );
    ver["node_kind"] = toEnumValue( version->node_kind );

    return ver;
}
#endif

Py::Object toConflictDescription( const svn_wc_conflict_description_t *description, SvnPool &pool )
{
    if( description == NULL )
        return Py::None();

    Py::Dict desc;
    desc["path"] = Py::String( description->path );
    desc["node_kind"] = toEnumValue( description->node_kind );
    desc["kind"] = toEnumValue( description->kind );
    desc["property_name"] = utf8_string_or_none( description->property_name );
    desc["is_binary"] = Py::Boolean( description->is_binary != 0 );
    desc["mime_type"] = utf8_string_or_none( description->mime_type );
    desc["action"] = toEnumValue( description->action );
    desc["reason"] = toEnumValue( description->reason );
    desc["base_file"] = path_string_or_none( description->base_file, pool );
    desc["their_file"] = path_string_or_none( description->their_file, pool );
    desc["my_file"] = path_string_or_none( description->my_file, pool );
    desc["merged_file"] = path_string_or_none( description->merged_file, pool );

#if defined( PYSVN_HAS_SVN_CLIENT_CTX_T__CONFLICT_FUNC_1_6 )
    desc["operation"] = toEnumValue( description->operation );
    desc["src_left_version"] =  toConflictVersion( description->src_left_version );
    desc["src_right_version"] = toConflictVersion( description->src_right_version );
#endif

    return desc;
}

bool pysvn_context::contextConflictResolver
    (
    svn_wc_conflict_result_t **result,
    const svn_wc_conflict_description_t *description,
    apr_pool_t *conflict_resolver_pool
    )
{
    TRACE_CALLBACK( contextConflictResolver );

    PythonDisallowThreads callback_permission( m_permission );

    // make sure we can call the users object
    if( !m_pyfn_ConflictResolver.isCallable() )
        return false;

    Py::Callable callback( m_pyfn_ConflictResolver );

    SvnPool pool( *this );

    Py::Tuple args( 1 );
    args[0] = toConflictDescription( description, pool );

    try
    {
        Py::Tuple py_result( callback.apply( args ) );

        Py::ExtensionObject< pysvn_enum_value<svn_wc_conflict_choice_t> > py_kind( py_result[0] );
        svn_wc_conflict_choice_t choice( py_kind.extensionObject()->m_value );

        Py::Object py_merge_file( py_result[1] );
        const char *merged_file = NULL;
        if( !py_merge_file.isNone() )
        {
            Py::String pystr_merge_file( py_merge_file );
            std::string std_merged_file( pystr_merge_file.as_std_string( name_utf8 ) );
            merged_file = svn_string_ncreate( std_merged_file.data(), std_merged_file.length(), getContextPool() )->data;
        }

#if defined( PYSVN_HAS_SVN_WC_CONFLICT_RESULT_T__SAVE_MERGED )
        svn_boolean_t save_merged = py_result[2].isTrue() ? TRUE : FALSE;
#endif

        *result = svn_wc_create_conflict_result( choice, merged_file, conflict_resolver_pool );

#if defined( PYSVN_HAS_SVN_WC_CONFLICT_RESULT_T__SAVE_MERGED )
        (*result)->save_merged = save_merged;
#endif

        return true;
    }
    catch( Py::Exception &e )
    {
        PyErr_Print();
        e.clear();

        m_error_message = "unhandled exception in callback_conflict_resolver";
        return false;
    }
}
#endif


// 
// this method will be called to notify about
// the progress of an ongoing action
//

#if defined( PYSVN_HAS_CONTEXT_NOTIFY2 )
void pysvn_context::contextNotify2
    (
    const svn_wc_notify_t *notify,
    apr_pool_t *pool
    )
{
    TRACE_CALLBACK( contextNotify2 );

    PythonDisallowThreads callback_permission( m_permission );

    // make sure we can call the users object
    if( !m_pyfn_Notify.isCallable() )
        return;

    Py::Callable callback( m_pyfn_Notify );

    Py::Tuple args( 1 );
    Py::Dict info;
    args[0] = info;

    info["path"] = Py::String( notify->path );
    info["action"] = toEnumValue( notify->action );
    info["kind"] = toEnumValue( notify->kind );
    info["mime_type"] = utf8_string_or_none( notify->mime_type );
    info["content_state"] = toEnumValue( notify->content_state );
    info["prop_state"] = toEnumValue( notify->prop_state );
    info["revision"] = Py::asObject( new pysvn_revision( svn_opt_revision_number, 0, notify->revision ) );
    if( notify->err != NULL )
    {
        SvnException error( notify->err );
        info["error"] = error.pythonExceptionArg( 1 );
    }
    else
    {
        info["error"] = Py::None();
    }

    Py::Object results;

    try
    {
        results = callback.apply( args );
    }
    catch( Py::Exception &e )
    {
        PyErr_Print();
        e.clear();

        m_error_message = "unhandled exception in callback_notify";
    }
}
#else
void pysvn_context::contextNotify
    (
    const char *path,
    svn_wc_notify_action_t action,
    svn_node_kind_t kind,
    const char *mime_type,
    svn_wc_notify_state_t content_state,
    svn_wc_notify_state_t prop_state,
    svn_revnum_t revnum
    )
{
    TRACE_CALLBACK( contextNotify );

    PythonDisallowThreads callback_permission( m_permission );

    // make sure we can call the users object
    if( !m_pyfn_Notify.isCallable() )
        return;

    Py::Callable callback( m_pyfn_Notify );

    Py::Tuple args( 1 );
    Py::Dict info;
    args[0] = info;

    info["path"] = Py::String( path );
    info["action"] = toEnumValue( action );
    info["kind"] = toEnumValue( kind );
    if( mime_type == NULL )
        info["mime_type"] = Py::None();
    else
        info["mime_type"] = Py::String( mime_type );
    info["content_state"] = toEnumValue( content_state );
    info["prop_state"] = toEnumValue( prop_state );
    info["revision"] = Py::asObject( new pysvn_revision( svn_opt_revision_number, 0, revnum ) );

    Py::Object results;

    try
    {
        results = callback.apply( args );
    }
    catch( Py::Exception &e )
    {
        PyErr_Print();
        e.clear();

        m_error_message = "unhandled exception in callback_notify";
    }
}
#endif

//
//    Return true to cancel a long running operation
//
bool pysvn_context::contextCancel()
{
    TRACE_CALLBACK( contextCancel );

    PythonDisallowThreads callback_permission( m_permission );

    // make sure we can call the users object
    if( !m_pyfn_Cancel.isCallable() )
        return false;

    Py::Callable callback( m_pyfn_Cancel );

    Py::Tuple args( 0 );

    // bool
    Py::Object result;

    Py::Int retcode;

    try
    {
        result = callback.apply( args );
        retcode = result;

        return long( retcode ) != 0;
    }
    catch( Py::Exception &e )
    {
        PyErr_Print();
        e.clear();

        m_error_message = "unhandled exception in callback_cancel";

        return false;
    }
}

void pysvn_context::setLogMessage( const std::string & a_msg )
{
    m_log_message = a_msg;
}

//
// this method will be called to retrieve
// a log message
//
bool pysvn_context::contextGetLogMessage( std::string & a_msg )
{
    TRACE_CALLBACK( contextGetLogMessage );

    if( !m_log_message.empty() )
    {
        a_msg = m_log_message;
        m_log_message.erase();

        return true;
    }

    PythonDisallowThreads callback_permission( m_permission );

    if( !m_pyfn_GetLogMessage.isCallable() )
    {
        m_error_message = "callback_get_log_message required";
        return false;
    }

    Py::Tuple args( 0 );
    try
    {
        return get_string( m_pyfn_GetLogMessage, args, a_msg );
    }
    catch( Py::Exception &e )
    {
        PyErr_Print();
        e.clear();

        m_error_message = "unhandled exception in callback_get_log_message";
    }

    return false;
}

//
// this method is called if there is ssl server
// information, that has to be confirmed by the user
//
// @param data 
// @return @a SslServerTrustAnswer
//
bool pysvn_context::contextSslServerTrustPrompt
        ( 
        const svn_auth_ssl_server_cert_info_t &info, 
        const std::string &realm,
        apr_uint32_t &a_accepted_failures,
        bool &accept_permanent
        )
{
    TRACE_CALLBACK( contextSslServerTrustPrompt );

    PythonDisallowThreads callback_permission( m_permission );

    // make sure we can call the users object
    if( !m_pyfn_SslServerTrustPrompt.isCallable() )
    {
        m_error_message = "callback_ssl_server_trust_prompt required";

        return false;
    }

    Py::Callable callback( m_pyfn_SslServerTrustPrompt );

    Py::Dict trust_info;
    trust_info[Py::String("failures")] = Py::Int( long( a_accepted_failures ) );
    trust_info[Py::String("hostname")] = Py::String( info.hostname );
    trust_info[Py::String("finger_print")] = Py::String( info.fingerprint );
    trust_info[Py::String("valid_from")] = Py::String( info.valid_from );
    trust_info[Py::String("valid_until")] = Py::String( info.valid_until );
    trust_info[Py::String("issuer_dname")] = Py::String( info.issuer_dname );
    trust_info[Py::String("realm")] = Py::String( realm );

    Py::Tuple args( 1 );
    args[0] = trust_info;

    Py::Tuple result_tuple;
    Py::Int retcode;
    Py::Int accepted_failures;
    Py::Int may_save;

    try
    {
        result_tuple = callback.apply( args );
        retcode = result_tuple[0];
        accepted_failures = result_tuple[1];
        may_save = result_tuple[2];

        a_accepted_failures = long( accepted_failures );
        if( long( retcode ) != 0 )
        {
            accept_permanent = long( may_save ) != 0;
            return true;
        }
        else
            return false;
    }
    catch( Py::Exception &e )
    {
        PyErr_Print();
        e.clear();

        m_error_message = "unhandled exception in callback_ssl_server_trust_prompt";
    }

    return false;
}

//
// this method is called to retrieve client side
// information
//
bool pysvn_context::contextSslClientCertPrompt( std::string &_cert_file, const std::string &_realm, bool &_may_save )
{
    TRACE_CALLBACK( contextSslClientCertPrompt );

    PythonDisallowThreads callback_permission( m_permission );

    if( !m_pyfn_SslClientCertPrompt.isCallable() )
    {
        m_error_message = "callback_ssl_client_cert_prompt required";

        return false;
    }

    Py::Callable callback( m_pyfn_SslClientCertPrompt );

    Py::Tuple args( 2 );
    args[0] = Py::String( _realm );
    args[1] = Py::Int( _may_save );

    Py::Tuple results;
    Py::Int retcode;
    Py::String cert_file;
    Py::Int may_save_out;

    try
    {
        results = callback.apply( args );
        retcode = results[0];
        cert_file = results[1];
        may_save_out = results[2];

        if( long( retcode ) != 0 )
        {
            _cert_file = cert_file.as_std_string( g_utf_8 );
            _may_save = long( may_save_out ) != 0;

            return true;
        }
    }
    catch( Py::Exception &e )
    {
        PyErr_Print();
        e.clear();

        m_error_message = "unhandled exception in callback_ssl_client_cert_prompt";

        return false;
    }

    return false;
}

//
// this method is called to retrieve the password
// for the certificate
//
// @param password
//
bool pysvn_context::contextSslClientCertPwPrompt
    (
    std::string &_password, 
    const std::string &_realm,
    bool &_may_save
    )
{
    TRACE_CALLBACK( contextSslClientCertPwPrompt );

    PythonDisallowThreads callback_permission( m_permission );

    // make sure we can call the users object
    if( !m_pyfn_SslClientCertPwPrompt.isCallable() )
    {
        m_error_message = "callback_ssl_client_cert_password_prompt required";

        return false;
    }

    Py::Callable callback( m_pyfn_SslClientCertPwPrompt );

    Py::Tuple args( 2 );
    args[0] = Py::String( _realm );
    args[1] = Py::Int( (long)_may_save );

    // bool, username, password
    Py::Tuple results;
    Py::Int retcode;
    Py::String username;
    Py::String password;
    Py::Int may_save_out;

    try
    {
        results = callback.apply( args );
        retcode = results[0];
        password = results[1];
        may_save_out = results[2];

        // true returned
        if( long( retcode ) != 0 )
        {
            // copy out the answers
            _password = password.as_std_string( g_utf_8 );
            _may_save = long( may_save_out ) != 0;

            return true;
        }
    }
    catch( Py::Exception &e )
    {
        PyErr_Print();
        e.clear();

        m_error_message = "unhandled exception in callback_ssl_client_cert_password_prompt";

        return false;
    }

    return false;
}

// common get a string implementation
static bool get_string( Py::Object &fn, Py::Tuple &args, std::string &msg )
{
    // make sure we can call the users object
    if( !fn.isCallable() )
        return false;

    Py::Callable callback( fn );

    Py::Tuple results;
    Py::Int retcode;
    Py::String message;

    results = callback.apply( args );
    retcode = results[0];
    message = results[1];

    // true returned
    if( long( retcode ) != 0 )
    {
        // copy out the answers
        msg = message.as_std_string( g_utf_8 );

        return true;
    }

    return false;
}
