//
// ====================================================================
// Copyright (c) 2003-2009 Barry A Scott.  All rights reserved.
//
// This software is licensed as described in the file LICENSE.txt,
// which you should have received as part of this distribution.
//
// ====================================================================
//
#include "pysvn_svnenv.hpp"
#include "svn_config.h"
#include "svn_pools.h"
#include "CXX/Objects.hxx"

//--------------------------------------------------------------------------------
//
//        SvnException
//
//--------------------------------------------------------------------------------
SvnException::SvnException( svn_error_t *error )
: m_message()
, m_exception_arg()
{
    std::string whole_message;

    // set the error to be a list of (code, message) tuples
    Py::List error_list;
    while( error != NULL )
    {
        Py::Tuple t( 2 );

        if( !whole_message.empty() )
        {
            whole_message += "\n";
        }

        if( error->message != NULL )
        {
            t[0] = Py::String( error->message );
            whole_message += error->message;
        }
        else
        {
            char buffer[256];
            buffer[0] = '\0';

            svn_strerror( error->apr_err, buffer, sizeof( buffer ) );
            whole_message += buffer;
            t[0] = Py::String( buffer );
        }
        t[1] = Py::Int( error->apr_err );
        error_list.append( t );

        error = error->child;
    }
    m_message = Py::String( whole_message );
    Py::Tuple arg_list(2);
    arg_list[0] = m_message;
    arg_list[1] = error_list;

    m_exception_arg = arg_list;

    svn_error_clear( error );
}

SvnException::SvnException( const SvnException &other )
: m_code( other.m_code )
, m_message( other.m_message )
, m_exception_arg( other.m_exception_arg )
{ }

SvnException::~SvnException()
{ }

Py::String &SvnException::message()
{
    return m_message;
}

apr_status_t SvnException::code()
{
    return m_code;
}

Py::Object &SvnException::pythonExceptionArg( int style )
{
    if( style == 1 )
    {
        return m_exception_arg;
    }
    else
    {
        return m_message;
    }
}

//--------------------------------------------------------------------------------
//
//        SvnContext
//
//--------------------------------------------------------------------------------
#if defined( PYSVN_HAS_CONTEXT_LOG_MSG2 )
extern "C" svn_error_t *handlerLogMsg2
    (
    const char **log_msg,
    const char **tmp_file,
    const apr_array_header_t *commit_items,
    void *baton,
    apr_pool_t *pool
    )
{
    SvnContext *context = SvnContext::castBaton( baton );

    std::string msg;

    if (!context->contextGetLogMessage( msg ) )
        return svn_error_create( SVN_ERR_CANCELLED, NULL, "" );

    *log_msg = svn_string_ncreate( msg.data(), msg.length(), pool )->data;
    *tmp_file = NULL;

    return SVN_NO_ERROR;
}
#else
extern "C" svn_error_t *handlerLogMsg
    (
    const char **log_msg,
    const char **tmp_file,
    apr_array_header_t *commit_items,
    void *baton,
    apr_pool_t *pool
    )
{
    SvnContext *context = SvnContext::castBaton( baton );

    std::string msg;

    if (!context->contextGetLogMessage( msg ) )
        return svn_error_create( SVN_ERR_CANCELLED, NULL, "" );

    *log_msg = svn_string_ncreate( msg.data(), msg.length(), pool )->data;
    *tmp_file = NULL;

    return SVN_NO_ERROR;
}
#endif

#if defined( PYSVN_HAS_CONTEXT_NOTIFY2 )
extern "C" void handlerNotify2
    (
    void *baton,
    const svn_wc_notify_t *notify,
    apr_pool_t *pool
    )
{
    SvnContext *context = SvnContext::castBaton( baton );

    context->contextNotify2( notify, pool );
}
#else
extern "C" void handlerNotify
    (
    void * baton,
    const char *path,
    svn_wc_notify_action_t action,
    svn_node_kind_t kind,
    const char *mime_type,
    svn_wc_notify_state_t content_state,
    svn_wc_notify_state_t prop_state,
    svn_revnum_t revision
    )
{
    pysvn_bpt();

    SvnContext *context = SvnContext::castBaton( baton );

    context->contextNotify( path, action, kind, mime_type, content_state, prop_state, revision );
}
#endif

#if defined( PYSVN_HAS_CONTEXT_PROGRESS )
extern "C" void handlerProgress
    (
    apr_off_t progress,
    apr_off_t total,
    void *baton,
    apr_pool_t *pool
    )
{
    SvnContext *context = SvnContext::castBaton( baton );

    context->contextProgress( progress, total );
}
#endif

#if defined( PYSVN_HAS_SVN_CLIENT_CTX_T__CONFLICT_FUNC )
extern "C" svn_error_t *handlerConflictResolver
    (
    svn_wc_conflict_result_t **result,
    const svn_wc_conflict_description_t *description,
    void *baton,
    apr_pool_t *pool
    )
{
    SvnContext *context = SvnContext::castBaton( baton );

    if( context->contextConflictResolver( result, description, pool ) )
        return SVN_NO_ERROR;
    else
        return svn_error_create( SVN_ERR_CANCELLED, NULL, "cancelled by user" );
}
#endif

extern "C" svn_error_t *handlerCancel
    (
    void * baton
    )
{
    SvnContext *context = SvnContext::castBaton( baton );

    if( context->contextCancel() )
        return svn_error_create( SVN_ERR_CANCELLED, NULL, "cancelled by user" );
    else
        return SVN_NO_ERROR;
}

extern "C" svn_error_t *handlerSimplePrompt
    (
    svn_auth_cred_simple_t **cred,
    void *baton,
    const char *a_realm,
    const char *a_username, 
    svn_boolean_t a_may_save,
    apr_pool_t *pool
    )
{
    SvnContext *context = SvnContext::castBaton( baton );

    bool may_save = a_may_save != 0;

    if( a_realm == NULL )
        a_realm = "";
    if( a_username == NULL )
        a_username = "";
    std::string realm( a_realm );
    std::string username( a_username );
    std::string password;

    if( !context->contextGetLogin( realm, username, password, may_save ) )
        return svn_error_create( SVN_ERR_CANCELLED, NULL, "" );

    svn_auth_cred_simple_t *lcred = (svn_auth_cred_simple_t *)apr_palloc( pool, sizeof( svn_auth_cred_simple_t ) );
    lcred->username = svn_string_ncreate( username.data(), username.length(), pool )->data;
    lcred->password = svn_string_ncreate( password.data(), password.length(), pool )->data;

    // tell svn if the credentials need to be saved
    lcred->may_save = may_save;
    *cred = lcred;

    return SVN_NO_ERROR;
}

extern "C" svn_error_t *handlerSslServerTrustPrompt 
    (
    svn_auth_cred_ssl_server_trust_t **cred, 
    void *baton,
    const char *a_realm,
    apr_uint32_t failures,
    const svn_auth_ssl_server_cert_info_t *info,
    svn_boolean_t may_save,
    apr_pool_t *pool
    )
{
    SvnContext *context = SvnContext::castBaton( baton );

    apr_uint32_t accepted_failures = failures;
    bool accept_permanently = true;

    if( a_realm == NULL )
        a_realm = "";
    std::string realm( a_realm );
    if( !context->contextSslServerTrustPrompt( *info, realm, accepted_failures, accept_permanently ) )
    {
        *cred = NULL;

        return SVN_NO_ERROR;
    }

    svn_auth_cred_ssl_server_trust_t *new_cred = (svn_auth_cred_ssl_server_trust_t *)
            apr_palloc( pool, sizeof (svn_auth_cred_ssl_server_trust_t) );

    if( accept_permanently )
    {
        new_cred->may_save = 1;
    }

    new_cred->accepted_failures = accepted_failures;

    *cred = new_cred;

    return SVN_NO_ERROR;
}

extern "C" svn_error_t *handlerSslClientCertPrompt 
    (
    svn_auth_cred_ssl_client_cert_t **cred, 
    void *baton, 
    const char *a_realm,
    svn_boolean_t a_may_save,
    apr_pool_t *pool
    )
{
    SvnContext *context = SvnContext::castBaton( baton );

    if( a_realm == NULL )
        a_realm = "";
    std::string realm( a_realm );
    bool may_save = a_may_save != 0;
    std::string cert_file;
    if( !context->contextSslClientCertPrompt( cert_file, realm, may_save ) )
        return svn_error_create (SVN_ERR_CANCELLED, NULL, "");

    svn_auth_cred_ssl_client_cert_t *new_cred = (svn_auth_cred_ssl_client_cert_t*)
        apr_palloc (pool, sizeof (svn_auth_cred_ssl_client_cert_t));

    new_cred->cert_file = svn_string_ncreate( cert_file.data(), cert_file.length(), pool )->data;
    new_cred->may_save = may_save;

    *cred = new_cred;

    return SVN_NO_ERROR;
}

extern "C" svn_error_t *handlerSslClientCertPwPrompt
    (
    svn_auth_cred_ssl_client_cert_pw_t **cred, 
    void *baton, 
    const char *a_realm,
    svn_boolean_t a_may_save,
    apr_pool_t *pool
    )
{
    SvnContext *context = SvnContext::castBaton( baton );

    if( a_realm == NULL )
        a_realm = "";
    std::string realm( a_realm );

    std::string password;
    bool may_save = a_may_save != 0;
    if( !context->contextSslClientCertPwPrompt( password, realm, may_save ) )
        return svn_error_create( SVN_ERR_CANCELLED, NULL, "" );

    svn_auth_cred_ssl_client_cert_pw_t *new_cred = (svn_auth_cred_ssl_client_cert_pw_t *)
        apr_palloc (pool, sizeof (svn_auth_cred_ssl_client_cert_pw_t));

    new_cred->password = svn_string_ncreate( password.data(), password.length(), pool )->data;
    new_cred->may_save = may_save;

    *cred = new_cred;

    return SVN_NO_ERROR;
}

SvnContext::SvnContext( const std::string &config_dir_str )
: m_pool( NULL )
, m_context( NULL )
, m_config_dir( NULL )
{
    memset( &m_context, 0, sizeof( m_context ) );

    apr_pool_create( &m_pool, NULL );

#if defined( PYSVN_HAS_CLIENT_CREATE_CONTEXT2 )
    svn_client_create_context2( &m_context, NULL, m_pool );
#else
    svn_client_create_context( &m_context, m_pool );
#endif

    if( !config_dir_str.empty() )
    {
        m_config_dir = svn_dirent_canonicalize( config_dir_str.c_str(), m_pool );
    }

    svn_config_ensure( m_config_dir, m_pool );

#if defined( PYSVN_HAS_SVN_AUTH_PROVIDERS )
    apr_array_header_t *providers = apr_array_make( m_pool, 11, sizeof( svn_auth_provider_object_t * ) );

    // simple providers
    svn_auth_provider_object_t *provider = NULL;
#if defined( WIN32 )
    svn_auth_get_windows_simple_provider(&provider, m_pool);
    *(svn_auth_provider_object_t **)apr_array_push( providers ) = provider;
#endif
#if defined( DARWIN )
    svn_auth_get_keychain_simple_provider(&provider, m_pool);
    *(svn_auth_provider_object_t **)apr_array_push( providers ) = provider;
#endif
#if defined( PYSVN_HAS_AUTH_GET_SIMPLE_PROVIDER2 )
    svn_auth_get_simple_provider2( &provider, NULL, NULL, m_pool );
#else
    svn_auth_get_simple_provider( &provider, m_pool );
#endif
    *(svn_auth_provider_object_t **)apr_array_push( providers ) = provider;

    svn_auth_get_username_provider( &provider, m_pool );
    *(svn_auth_provider_object_t **)apr_array_push( providers ) = provider;

    svn_auth_get_simple_prompt_provider( &provider, handlerSimplePrompt, this, 1000000, m_pool );
    *(svn_auth_provider_object_t **)apr_array_push( providers ) = provider;

    // ssl providers

    // order is important - file first then prompt providers
    svn_auth_get_ssl_server_trust_file_provider( &provider, m_pool );
    *(svn_auth_provider_object_t **)apr_array_push( providers ) = provider;

    svn_auth_get_ssl_client_cert_file_provider( &provider, m_pool );
    *(svn_auth_provider_object_t **)apr_array_push( providers ) = provider;

#if defined( PYSVN_HAS_AUTH_GET_SSL_CLIENT_CERT_PW_FILE_PROVIDER2 )
    svn_auth_get_ssl_client_cert_pw_file_provider2( &provider, NULL, NULL, m_pool );
#else
    svn_auth_get_ssl_client_cert_pw_file_provider( &provider, m_pool );
#endif
    *(svn_auth_provider_object_t **)apr_array_push( providers ) = provider;

    svn_auth_get_ssl_server_trust_prompt_provider( &provider, handlerSslServerTrustPrompt, this, m_pool );
    *(svn_auth_provider_object_t **)apr_array_push (providers) = provider;

    svn_auth_get_ssl_client_cert_prompt_provider( &provider, handlerSslClientCertPrompt, this, 3, m_pool );
    *(svn_auth_provider_object_t **)apr_array_push (providers) = provider;

    svn_auth_get_ssl_client_cert_pw_prompt_provider( &provider, handlerSslClientCertPwPrompt, this, 3, m_pool );
    *(svn_auth_provider_object_t **)apr_array_push (providers) = provider;
#else
    // Pre 1.4.0 version
    apr_array_header_t *providers = apr_array_make( m_pool, 8, sizeof( svn_auth_provider_object_t * ) );

    // simple providers
    svn_auth_provider_object_t *provider = NULL;
    svn_client_get_simple_provider( &provider, m_pool );
    *(svn_auth_provider_object_t **)apr_array_push( providers ) = provider;

    svn_client_get_username_provider( &provider, m_pool );
    *(svn_auth_provider_object_t **)apr_array_push( providers ) = provider;

    svn_client_get_simple_prompt_provider( &provider, handlerSimplePrompt, this, 3, m_pool );
    *(svn_auth_provider_object_t **)apr_array_push( providers ) = provider;

    // ssl providers

    // order is important - file first then prompt providers
    svn_client_get_ssl_server_trust_file_provider( &provider, m_pool );
    *(svn_auth_provider_object_t **)apr_array_push( providers ) = provider;

    svn_client_get_ssl_client_cert_file_provider( &provider, m_pool );
    *(svn_auth_provider_object_t **)apr_array_push( providers ) = provider;

    svn_client_get_ssl_client_cert_pw_file_provider( &provider, m_pool );
    *(svn_auth_provider_object_t **)apr_array_push( providers ) = provider;

    svn_client_get_ssl_server_trust_prompt_provider( &provider, handlerSslServerTrustPrompt, this, m_pool );
    *(svn_auth_provider_object_t **)apr_array_push (providers) = provider;

    svn_client_get_ssl_client_cert_prompt_provider( &provider, handlerSslClientCertPrompt, this, 3, m_pool );
    *(svn_auth_provider_object_t **)apr_array_push (providers) = provider;

    svn_client_get_ssl_client_cert_pw_prompt_provider( &provider, handlerSslClientCertPwPrompt, this, 3, m_pool );
    *(svn_auth_provider_object_t **)apr_array_push (providers) = provider;
#endif

    svn_auth_baton_t *auth_baton = NULL;
    svn_auth_open( &auth_baton, providers, m_pool );

    // get the config based on the config dir passed in
    svn_config_get_config( &m_context->config, m_config_dir, m_pool );

    // tell the auth functions where the config dir is
    svn_auth_set_parameter( auth_baton, SVN_AUTH_PARAM_CONFIG_DIR, m_config_dir );

    m_context->auth_baton = auth_baton;

#if defined( PYSVN_HAS_CONTEXT_LOG_MSG2 )
    m_context->log_msg_func2 = handlerLogMsg2;
    m_context->log_msg_baton2 = this;
#else
    m_context->log_msg_func = handlerLogMsg;
    m_context->log_msg_baton = this;
#endif
}

void SvnContext::installCancel( bool install )
{
    if( install )
    {
        m_context->cancel_func = handlerCancel;
        m_context->cancel_baton = this;
    }
    else
    {
        m_context->cancel_func = NULL;
        m_context->cancel_baton = NULL;
    }
}

void SvnContext::installNotify( bool install )
{
    if( install )
    {
#if defined( PYSVN_HAS_CONTEXT_NOTIFY2 )
        m_context->notify_func2 = handlerNotify2;
        m_context->notify_baton2 = this;
#else
        m_context->notify_func = handlerNotify;
        m_context->notify_baton = this;
#endif
    }
    else
    {
#if defined( PYSVN_HAS_CONTEXT_NOTIFY2 )
        m_context->notify_func2 = NULL;
        m_context->notify_baton2 = NULL;
#else
        m_context->notify_func = NULL;
        m_context->notify_baton = NULL;
#endif
    }
}

#if defined( PYSVN_HAS_CONTEXT_PROGRESS )
void SvnContext::installProgress( bool install )
{
    if( install )
    {
        m_context->progress_func = handlerProgress;
        m_context->progress_baton = this;
    }
    else
    {
        m_context->progress_func = handlerProgress;
        m_context->progress_baton = this;
    }
}
#endif

#if defined( PYSVN_HAS_SVN_CLIENT_CTX_T__CONFLICT_FUNC )
void SvnContext::installConflictResolver( bool install )
{
    if( install )
    {
        m_context->conflict_func = handlerConflictResolver;
        m_context->conflict_baton = this;
    }
    else
    {
        m_context->conflict_func = NULL;
        m_context->conflict_baton = NULL;
    }
}
#endif

SvnContext::~SvnContext()
{
    if( m_pool )
    {
        apr_pool_destroy( m_pool );
    }
}

SvnContext::operator svn_client_ctx_t *()
{
    return m_context;
}

svn_client_ctx_t *SvnContext::ctx()
{
    return m_context;
}

// only use this pool for data that has a life time
// that matches the life time of the context
apr_pool_t *SvnContext::getContextPool()
{
    return m_pool;
}

//--------------------------------------------------------------------------------
//
//        SvnTransaction
//
//--------------------------------------------------------------------------------
SvnTransaction::SvnTransaction()
: m_pool( NULL )
, m_repos( NULL )
, m_fs( NULL )
, m_txn( NULL )
, m_txn_name( NULL )
, m_rev_id( SVN_INVALID_REVNUM )
{
    apr_pool_create( &m_pool, NULL );
}

svn_error_t *SvnTransaction::init( const std::string &repos_path,
    const std::string &transaction_name, bool is_revision )
{
    svn_error_t *error;

    SvnPool scratch_pool( *this );

#if defined( PYSVN_HAS_REPOS_OPEN3 )
    error = svn_repos_open3( &m_repos, repos_path.c_str(), NULL, m_pool, scratch_pool );

#elif defined( PYSNV_HAS_REPOS_OPEN2 )
    error = svn_repos_open2( &m_repos, repos_path.c_str(), NULL, m_pool );

#else
    error = svn_repos_open( &m_repos, repos_path.c_str(), m_pool );
#endif
    if( error != NULL )
        return error;

    m_fs = svn_repos_fs( m_repos );
    // what is a warning function?
    // svn_fs_set_warning_func (m_fs, warning_func, NULL);

    if( is_revision )
    {
        Py::String rev_name( transaction_name );
        Py::Long long_val( rev_name );
        m_rev_id = (long)long_val;
        if (! SVN_IS_VALID_REVNUM( m_rev_id ))
            return svn_error_create( SVN_ERR_CL_ARG_PARSING_ERROR, NULL,
                "invalid revision number supplied" );
    }
    else
    {
        m_txn_name = apr_pstrdup( m_pool, transaction_name.c_str() );
        error = svn_fs_open_txn( &m_txn, m_fs, m_txn_name, m_pool );
    }

    return error;
}

SvnTransaction::~SvnTransaction()
{
}

svn_error_t *SvnTransaction::root( svn_fs_root_t **root, apr_pool_t *pool )
{
    if( is_revision() )
        return svn_fs_revision_root( root, m_fs, m_rev_id, pool );
    else
        return svn_fs_txn_root( root, m_txn, pool );
}

SvnTransaction::operator svn_fs_txn_t *()
{
    return m_txn;
}

SvnTransaction::operator svn_fs_t *()
{
    return m_fs;
}

SvnTransaction::operator svn_repos_t *()
{
    return m_repos;
}


svn_fs_txn_t *SvnTransaction::transaction()
{
    return m_txn;
}

svn_revnum_t SvnTransaction::revision()
{
    return m_rev_id;
}

//--------------------------------------------------------------------------------
//
//        Pool
//
//--------------------------------------------------------------------------------
SvnPool::SvnPool( SvnContext &ctx )
: m_pool( NULL )
{
    m_pool = svn_pool_create( NULL );
}

SvnPool::SvnPool( SvnTransaction &txn )
: m_pool( NULL )
{
    m_pool = svn_pool_create( NULL );
}

SvnPool::~SvnPool()
{
    if( m_pool != NULL )
    {
        svn_pool_destroy( m_pool );
    }
}

SvnPool::operator apr_pool_t *() const 
{
    return m_pool;
}

#if 0
// keep around as it useful for debugging pysvn
static const char *toHex( unsigned int num )
{
    static char buffer[9];
    for( int i=0; i<8; i++ )
    {
        buffer[i] = "0123456789abcdef"[ (num >> (32-(i+1)*4)) & 0x0f ];
    }
    buffer[8] = '\0';
    return buffer;
}
#endif
