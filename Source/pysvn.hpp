//
// ====================================================================
// Copyright (c) 2003-2016 Barry A Scott.  All rights reserved.
//
// This software is licensed as described in the file LICENSE.txt,
// which you should have received as part of this distribution.
//
// ====================================================================
//
#include "Python.h"
#include "CXX/Version.hxx"

#include "CXX/Objects.hxx"
#include "CXX/Extensions.hxx"
#include <iostream>

#include "pysvn_svnenv.hpp"

#include <string>
#include <list>
#include <map>

extern "C" int pysvn_breakpoint();

//--------------------------------------------------------------------------------
class pysvn_module : public Py::ExtensionModule<pysvn_module>
{
public:
    pysvn_module();
    virtual ~pysvn_module();
private:// functions
    Py::Object new_client( const Py::Tuple &a_args, const Py::Dict &a_kws );
    Py::Object new_transaction( const Py::Tuple &a_args, const Py::Dict &a_kws );
    Py::Object new_revision( const Py::Tuple &a_args, const Py::Dict &a_kws );

public:// variables
    Py::ExtensionExceptionType client_error;
};


//--------------------------------------------------------------------------------
class PythonAllowThreads;

class pysvn_context : public SvnContext
{
public:
    pysvn_context( const std::string &config_dir );
    virtual ~pysvn_context();

    // return true if the callbacks are being used on some thread
    bool hasPermission();
    // give the callbacks permission to run
    void setPermission( PythonAllowThreads &_permission );
    // revoke permission
    void clearPermission();

    void checkForError( Py::ExtensionExceptionType &exception_for_error );

public:     // data

    //
    // Python class that has implements the callback methods
    //
    Py::Object  m_pyfn_GetLogin;
    Py::Object  m_pyfn_Notify;
#if defined( PYSVN_HAS_CONTEXT_PROGRESS )
    Py::Object  m_pyfn_Progress;
#endif
#if defined( PYSVN_HAS_SVN_CLIENT_CTX_T__CONFLICT_FUNC )
    Py::Object  m_pyfn_ConflictResolver;
#endif

    Py::Object  m_pyfn_Cancel;
    Py::Object  m_pyfn_GetLogMessage;
    Py::Object  m_pyfn_SslServerPrompt;
    Py::Object  m_pyfn_SslServerTrustPrompt;
    Py::Object  m_pyfn_SslClientCertPrompt;
    Py::Object  m_pyfn_SslClientCertPwPrompt;

    std::string m_default_username;
    std::string m_default_password;

    void setLogMessage( const std::string &message );

private:    // methods

    //
    // this method will be called to retrieve
    // authentication information
    //
    bool contextGetLogin 
        (
        const std::string &realm,
        std::string &username, 
        std::string &password,
        bool &may_save
        );

    // 
    // this method will be called to notify about
    // the progress of an ongoing action
    //
#if defined( PYSVN_HAS_CONTEXT_NOTIFY2 )
    void contextNotify2
        (
        const svn_wc_notify_t *notify,
        apr_pool_t *pool
        );
#else
    void contextNotify
        (
        const char *path,
        svn_wc_notify_action_t action,
        svn_node_kind_t kind,
        const char *mime_type,
        svn_wc_notify_state_t content_state,
        svn_wc_notify_state_t prop_state,
        svn_revnum_t revision
        );
#endif

#if defined( PYSVN_HAS_CONTEXT_PROGRESS )
    void contextProgress
        (
        apr_off_t progress,
        apr_off_t total
        );
#endif

#if defined( PYSVN_HAS_SVN_CLIENT_CTX_T__CONFLICT_FUNC )
    bool contextConflictResolver
        (
        svn_wc_conflict_result_t **result,
        const svn_wc_conflict_description_t *description,
        apr_pool_t *pool
        );
#endif


    //
    // this method will be called periodically to allow
    // the app to cancel long running operations
    //
    // @return cancel action?
    // @retval true cancel
    //
    bool contextCancel();

    //
    // this method will be called to retrieve
    // a log message
    //
    bool contextGetLogMessage( std::string & msg );

    //
    // this method is called if there is ssl server
    // information, that has to be confirmed by the user
    //
    // @param data 
    // @return @a SslServerTrustAnswer
    //
    bool contextSslServerTrustPrompt
        (
        const svn_auth_ssl_server_cert_info_t &info, 
        const std::string &realm,
        apr_uint32_t &a_accepted_failures,
        bool &accept_permanent
        );
    //
    // this method is called to retrieve client side
    // information
    //
    bool contextSslClientCertPrompt( std::string &certFile, const std::string &realm, bool &may_save );

    //
    // this method is called to retrieve the password
    // for the certificate
    //
    // @param password
    //
    bool contextSslClientCertPwPrompt
        (
        std::string & password,
        const std::string &realm,
        bool &may_save
        );

private:// vaiables

    PythonAllowThreads  *m_permission;
    std::string         m_error_message;
    std::string         m_log_message;

};

class DictWrapper
{
public:
    DictWrapper( Py::Dict result_wrappers, const std::string &wrapper_name );
    ~DictWrapper();

    Py::Object wrapDict( Py::Dict result ) const;

private:
    const std::string m_wrapper_name;
    bool m_have_wrapper;
    Py::Callable m_wrapper;
};

class FunctionArguments;

class pysvn_client : public Py::PythonExtension<pysvn_client>
{
public:
    pysvn_client( pysvn_module &module, const std::string &config_dir, Py::Dict result_wrappers );
    virtual ~pysvn_client();

    // override functions from PythonExtension
    virtual Py::Object getattr( const char *name );
    virtual int setattr( const char *name, const Py::Object &value );
    static void init_type( void );

    // SVN commands
    Py::Object cmd_add( const Py::Tuple& args, const Py::Dict &kws );
#ifdef PYSVN_HAS_CLIENT_ADD_TO_CHANGELIST
    Py::Object cmd_add_to_changelist( const Py::Tuple& args, const Py::Dict &kws );
#endif
    Py::Object cmd_annotate( const Py::Tuple& args, const Py::Dict &kws );
#if defined( PYSVN_HAS_CLIENT_ANNOTATE5 )
    Py::Object cmd_annotate2( const Py::Tuple& args, const Py::Dict &kws );
#endif
    Py::Object cmd_cat( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object cmd_checkout( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object cmd_cleanup( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object cmd_checkin( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object cmd_copy( const Py::Tuple& args, const Py::Dict &kws );
#if defined( PYSVN_HAS_CLIENT_COPY4 )
    Py::Object cmd_copy2( const Py::Tuple& args, const Py::Dict &kws );
#endif
    Py::Object cmd_diff( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object cmd_diff_peg( const Py::Tuple& args, const Py::Dict &kws );
#if defined( PYSVN_HAS_CLIENT_DIFF_SUMMARIZE )
    Py::Object cmd_diff_summarize( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object cmd_diff_summarize_peg( const Py::Tuple& args, const Py::Dict &kws );
#endif
    Py::Object cmd_export( const Py::Tuple& args, const Py::Dict &kws );
#ifdef PYSVN_HAS_CLIENT_GET_CHANGELIST
    Py::Object cmd_get_changelist( const Py::Tuple& args, const Py::Dict &kws );
#endif
    Py::Object cmd_info( const Py::Tuple& args, const Py::Dict &kws );
#if defined( PYSVN_HAS_CLIENT_INFO )
    Py::Object cmd_info2( const Py::Tuple& args, const Py::Dict &kws );
#endif
    Py::Object cmd_import( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object cmd_log( const Py::Tuple& args, const Py::Dict &kws );
#if defined( PYSVN_HAS_CLIENT_LOCK )
    Py::Object cmd_lock( const Py::Tuple& args, const Py::Dict &kws );
#endif
#if defined( PYSVN_HAS_CLIENT_LIST )
    Py::Object cmd_list( const Py::Tuple& args, const Py::Dict &kws );
#endif
    Py::Object cmd_ls( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object cmd_merge( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object cmd_merge_peg( const Py::Tuple& args, const Py::Dict &kws );
#if defined( PYSVN_HAS_CLIENT_MERGE_PEG3 )
    Py::Object cmd_merge_peg2( const Py::Tuple& args, const Py::Dict &kws );
#endif
#if defined( PYSVN_HAS_CLIENT_MERGE_REINTEGRATE )
    Py::Object cmd_merge_reintegrate( const Py::Tuple& args, const Py::Dict &kws );
#endif
    Py::Object cmd_mkdir( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object cmd_move( const Py::Tuple& args, const Py::Dict &kws );
#if defined( PYSVN_HAS_CLIENT_MOVE4 )
    Py::Object cmd_move2( const Py::Tuple& args, const Py::Dict &kws );
#endif
#if defined( PYSVN_HAS_CLIENT_PATCH )
    Py::Object cmd_patch( const Py::Tuple& args, const Py::Dict &kws );
#endif
#if defined( PYSVN_HAS_CLIENT_PROPSET_LOCAL )
    Py::Object cmd_propdel_local( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object cmd_propset_local( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object common_propset_local( FunctionArguments &args, bool is_set );
#endif
#if defined( PYSVN_HAS_CLIENT_PROPSET_REMOTE )
    Py::Object cmd_propdel_remote( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object cmd_propset_remote( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object common_propset_remote( FunctionArguments &args, bool is_set );
#endif
    Py::Object cmd_propdel( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object cmd_propset( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object common_propset( FunctionArguments &args, bool is_set );
    Py::Object cmd_propget( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object cmd_proplist( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object cmd_relocate( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object cmd_remove( const Py::Tuple& args, const Py::Dict &kws );
#ifdef PYSVN_HAS_CLIENT_REMOVE_FROM_CHANGELISTS
    Py::Object cmd_remove_from_changelists( const Py::Tuple& args, const Py::Dict &kws );
#endif
    Py::Object cmd_resolved( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object cmd_revert( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object cmd_revpropdel( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object cmd_revpropset( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object common_revpropset( FunctionArguments &args, bool is_set );
    Py::Object cmd_revpropget( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object cmd_revproplist( const Py::Tuple& args, const Py::Dict &kws );
#if defined( PYSVN_HAS_CLIENT_STATUS5 )
    Py::Object cmd_status2( const Py::Tuple& args, const Py::Dict &kws );
#endif
    Py::Object cmd_status( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object cmd_switch( const Py::Tuple& args, const Py::Dict &kws );
#if defined( PYSVN_HAS_CLIENT_LOCK )
    Py::Object cmd_unlock( const Py::Tuple& args, const Py::Dict &kws );
#endif
#if defined( PYSVN_HAS_CLIENT_UPGRADE )
    Py::Object cmd_upgrade( const Py::Tuple& args, const Py::Dict &kws );
#endif
    Py::Object cmd_update( const Py::Tuple& args, const Py::Dict &kws );
#if defined( PYSVN_HAS_CLIENT_VACUUM )
    Py::Object cmd_vacuum( const Py::Tuple& args, const Py::Dict &kws );
#endif
    // SVN commands
    Py::Object is_url( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object get_auth_cache( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object set_auth_cache( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object get_auto_props( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object set_auto_props( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object get_interactive( const Py::Tuple &a_args, const Py::Dict &a_kws );
    Py::Object set_interactive( const Py::Tuple &a_args, const Py::Dict &a_kws );
    Py::Object get_default_username( const Py::Tuple &a_args, const Py::Dict &a_kws );
    Py::Object set_default_username( const Py::Tuple &a_args, const Py::Dict &a_kws );
    Py::Object get_default_password( const Py::Tuple &a_args, const Py::Dict &a_kws );
    Py::Object set_default_password( const Py::Tuple &a_args, const Py::Dict &a_kws );
    Py::Object get_store_passwords( const Py::Tuple &a_args, const Py::Dict &a_kws );
    Py::Object set_store_passwords( const Py::Tuple &a_args, const Py::Dict &a_kws );

#if defined( PYSVN_HAS_WC_ADM_DIR )
    Py::Object is_adm_dir( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object get_adm_dir( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object set_adm_dir( const Py::Tuple& args, const Py::Dict &kws );
#endif

#if defined( PYSVN_HAS_CLIENT_ROOT_URL_FROM_PATH )
    Py::Object cmd_root_url_from_path( const Py::Tuple& args, const Py::Dict &kws );
#endif

    // check that we are not in use on another thread
    void checkThreadPermission();
private:
    // helper function that wraps up the logic to throw a client error exception
    void throw_client_error( SvnException & );
private:
    Py::Object helper_boolean_auth_set( FunctionArguments &a_args, const char *a_arg_name, const char *a_param_name );
    Py::Object helper_boolean_auth_get( FunctionArguments &a_args, const char *a_param_name );
    Py::Object helper_string_auth_set( FunctionArguments &a_args,
                                        const char *a_arg_name, const char *a_param_name, std::string &ctx_str );
    Py::Object helper_string_auth_get( FunctionArguments &a_args, const char *a_param_name );

    pysvn_module        &m_module;
    Py::Dict            m_result_wrappers;
    pysvn_context       m_context;
    int                 m_exception_style;
    int                 m_commit_info_style;

#if defined( PYSVN_HAS_CLIENT_STATUS_T )
    DictWrapper         m_wrapper_status2;
#endif
    DictWrapper         m_wrapper_status;
    DictWrapper         m_wrapper_entry;
    DictWrapper         m_wrapper_info;
    DictWrapper         m_wrapper_lock;
    DictWrapper         m_wrapper_list;
    DictWrapper         m_wrapper_log;
    DictWrapper         m_wrapper_log_changed_path;
    DictWrapper         m_wrapper_dirent;
    DictWrapper         m_wrapper_wc_info;
    DictWrapper         m_wrapper_diff_summary;
    DictWrapper         m_wrapper_commit_info;
};

class pysvn_transaction : public Py::PythonExtension<pysvn_transaction>
{
public:
    pysvn_transaction( pysvn_module &module, Py::Dict result_wrappers );
    virtual ~pysvn_transaction();
    void init( const std::string &repos_path,
        const std::string &transaction_name, bool is_revision );

    // override functions from PythonExtension
    virtual Py::Object getattr( const char *name );
    virtual int setattr( const char *name, const Py::Object &value );
    static void init_type( void );

    // SVN Transaction commands
    Py::Object cmd_cat( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object cmd_changed( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object cmd_list( const Py::Tuple& args, const Py::Dict &kws );

    Py::Object cmd_propdel( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object cmd_propget( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object cmd_proplist( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object cmd_propset( const Py::Tuple& args, const Py::Dict &kws );

    Py::Object cmd_revpropdel( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object cmd_revpropget( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object cmd_revproplist( const Py::Tuple& args, const Py::Dict &kws );
    Py::Object cmd_revpropset( const Py::Tuple& args, const Py::Dict &kws );

    // check that we are not in use on another thread
    void checkThreadPermission();
private:
    // helper function that wraps up the logic to throw a client error exception
    void throw_client_error( SvnException & );
private:
    pysvn_module        &m_module;
    Py::Dict            m_result_wrappers;
    SvnTransaction      m_transaction;
    int                 m_exception_style;
};

class pysvn_revision : public Py::PythonExtension<pysvn_revision>
{
public:
    pysvn_revision( svn_opt_revision_kind kind, double date=0.0, int revnum=0 );

    virtual ~pysvn_revision();

    virtual Py::Object getattr( const char *name );
    virtual int setattr( const char *name, const Py::Object &value );
    virtual Py::Object repr();

    const svn_opt_revision_t &getSvnRevision() const;

    static void init_type(void);

private:
    pysvn_revision();    // not defined
    svn_opt_revision_t m_svn_revision;
};

// help functions to check the arguments pass and throw AttributeError
struct argument_description
{
    bool m_required;            // true if this argument must be present
    const char *m_arg_name;     // the name of this arg to lookup in the keywords dictionary
};

class FunctionArguments
{
public:
    FunctionArguments
        (
        const char *function_name,
        const argument_description *arg_info,
        const Py::Tuple &args,
        const Py::Dict &kws
        );
    ~FunctionArguments();
    void check();

    bool hasArg( const char *arg_name );
    bool hasArgNotNone( const char *arg_name );
    Py::Object getArg( const char *arg_name );

    bool getBoolean( const char *name );
    bool getBoolean( const char *name, bool default_value );
    int getInteger( const char *name );
    int getInteger( const char *name, int default_value );
    long getLong( const char *name );
    long getLong( const char *name, long default_value );
    std::string getUtf8String( const char *name );
    std::string getUtf8String( const char *name, const std::string &default_value );
    std::string getBytes( const char *name, const std::string &default_value );
    std::string getBytes( const char *name );
    svn_opt_revision_t getRevision( const char *name );
    svn_opt_revision_t getRevision( const char *name, svn_opt_revision_kind default_value );
    svn_opt_revision_t getRevision( const char *name, svn_opt_revision_t default_value );

#if defined( PYSVN_HAS_SVN__DEPTH_PARAMETER )
    svn_depth_t getDepth( const char *depth_name, const char *recursive_name, svn_depth_t default_value, svn_depth_t recursive_true_value, svn_depth_t recursive_false_value );
    svn_depth_t getDepth( const char *depth_name, svn_depth_t default_value );
    svn_depth_t getDepth( const char *depth_name );
#endif
#if defined( PYSVN_HAS_SVN_WC_CONFLICT_CHOICE_T )
    svn_wc_conflict_choice_t getWcConflictChoice( const char *choice_name, svn_wc_conflict_choice_t default_value );
    svn_wc_conflict_choice_t getWcConflictChoice( const char *choice_name );
#endif

public:
    const std::string           m_function_name;
private:
    const argument_description  *m_arg_desc;
    const Py::Tuple             &m_args;
    const Py::Dict              &m_kws;

    Py::Dict                    m_checked_args;
    Py::Tuple::size_type        m_min_args;
    Py::Tuple::size_type        m_max_args;
};

//------------------------------------------------------------
#if defined( PYSVN_HAS_COMMIT_CALLBACK2_T )
extern "C" svn_error_t *CommitInfoResult_callback( const svn_commit_info_t *commit_info, void *baton, apr_pool_t *pool );

class CommitInfoResult
{
    friend svn_error_t *CommitInfoResult_callback( const svn_commit_info_t *commit_info, void *baton, apr_pool_t *pool );

public:
    CommitInfoResult( SvnPool &pool );
    ~CommitInfoResult();

    // needed to call svn API
    svn_commit_callback2_t callback() { return &CommitInfoResult_callback; }
    void *baton() { return static_cast< void * >( this ); }

    // convert to python
    int count();
    const svn_commit_info_t *result( int index );

private:
    static CommitInfoResult *castBaton( void *baton_ ) { return static_cast<CommitInfoResult *>( baton_ ); }

    apr_array_header_t  *m_all_results;
    SvnPool             &m_pool;
};
#endif

//------------------------------------------------------------
template<TEMPLATE_TYPENAME T>
class EnumString
{
public:
    EnumString();
    ~EnumString() {}

    const std::string &toTypeName( T )
    {
        return m_type_name;
    }

    const std::string &toString( T value )
    {
        static std::string not_found( "-unknown-" );
        EXPLICIT_TYPENAME std::map<T,std::string>::iterator it = m_enum_to_string.find( value );
        if( it != m_enum_to_string.end() )
            return (*it).second;

        not_found = "-unknown (";
        int u1000 = value/1000 % 10;
        int u100 = value/100 % 10;
        int u10 = value/10 % 10;
        int u1 = value % 10;
        not_found += char( '0' + u1000 );
        not_found += char( '0' + u100 );
        not_found += char( '0' + u10 );
        not_found += char( '0' + u1 );
        not_found += ")-";
        return not_found;
    }

    bool toEnum( const std::string &string, T &value )
    {
        EXPLICIT_TYPENAME std::map<std::string,T>::iterator it = m_string_to_enum.find( string );
        if( it != m_string_to_enum.end() )
        {
            value = (*it).second;
            return true;
        }

        return false;
    }

    EXPLICIT_TYPENAME std::map<std::string,T>::iterator begin()
    {
        return m_string_to_enum.begin();
    }

    EXPLICIT_TYPENAME std::map<std::string,T>::iterator end()
    {
        return m_string_to_enum.end();
    }

private:
    void add( T value, std::string string )
    {
        m_string_to_enum[string] = value;
        m_enum_to_string[value] = string;
    }
 
    std::string             m_type_name;
    std::map<std::string,T> m_string_to_enum;
    std::map<T,std::string> m_enum_to_string;
};

template<TEMPLATE_TYPENAME T>
const std::string &toTypeName( T value )
{
    static EnumString< T > enum_map;

    return enum_map.toTypeName( value );
}

template<TEMPLATE_TYPENAME T>
const std::string &toString( T value )
{
    static EnumString< T > enum_map;

    return enum_map.toString( value );
}

template<TEMPLATE_TYPENAME T>
bool toEnum( const std::string &string, T &value )
{
    static EnumString< T > enum_map;

    return enum_map.toEnum( string, value );
}

template<TEMPLATE_TYPENAME T>
Py::List memberList( T value )
{
    static EnumString< T > enum_map;

    Py::List members;

    EXPLICIT_TYPENAME std::map<std::string,T>::iterator it = enum_map.begin();
    while( it != enum_map.end() )
    {
        members.append( Py::String( (*it).first ) );
        ++it;
    }
    
    return members;
}

//------------------------------------------------------------
template<TEMPLATE_TYPENAME T>
class pysvn_enum_value : public Py::PythonExtension< EXPLICIT_CLASS pysvn_enum_value<T> >
{
public:
    pysvn_enum_value( T _value)
        : Py::PythonExtension<pysvn_enum_value>()
        , m_value( _value )
    { }

    virtual ~pysvn_enum_value()
    { }

    virtual int compare( const Py::Object &other )
    {
        if( pysvn_enum_value::check( other ) )
        {
            pysvn_enum_value<T> *other_value = static_cast<pysvn_enum_value *>( other.ptr() );
            if( m_value == other_value->m_value )
                return 0;

            if( m_value > other_value->m_value )
                return 1;
            else
                return -1;
        }
        else
        {
            std::string msg( "expecting " );
            msg += toTypeName( m_value );
            msg += " object for compare ";
            throw Py::AttributeError( msg );
        }
    }

    virtual Py::Object rich_compare( const Py::Object &other, int op )
    {
        if( pysvn_enum_value::check( other ) )
        {
            pysvn_enum_value<T> *other_value = static_cast<pysvn_enum_value *>( other.ptr() );
            switch( op )
            {
            case Py_EQ:
                return Py::Boolean( m_value == other_value->m_value );
            case Py_NE:
                return Py::Boolean( m_value != other_value->m_value );
            case Py_LT:
                return Py::Boolean( m_value <  other_value->m_value );
            case Py_LE:
                return Py::Boolean( m_value <= other_value->m_value );
            case Py_GT:
                return Py::Boolean( m_value >  other_value->m_value );
            case Py_GE:
                return Py::Boolean( m_value >= other_value->m_value );
            default:
                throw Py::RuntimeError( "rich_compare bad op" );
            }
        }
        else
        {
            std::string msg( "expecting " );
            msg += toTypeName( m_value );
            msg += " object for rich compare ";
            throw Py::NotImplementedError( msg );
        }
    }

    virtual Py::Object repr()
    {
        std::string s("<");
        s += toTypeName( m_value );
        s += ".";
        s += toString( m_value );
        s += ">";

        return Py::String( s );
    }

    virtual Py::Object str()
    {
        return Py::String( toString( m_value ) );
    }

    // need a hash so that the enums can go into a map
    virtual long hash()
    {
        static Py::String type_name( toTypeName( m_value ) );

        // use the m_value plus the hash of the type name
        return long( m_value ) + type_name.hashValue();
    }

    static void init_type(void);

public:
    T m_value;
};

//------------------------------------------------------------
template<TEMPLATE_TYPENAME T>
class pysvn_enum : public Py::PythonExtension< EXPLICIT_CLASS pysvn_enum<T> >
{
public:
    pysvn_enum()
        : Py::PythonExtension<pysvn_enum>()
    { }

    virtual ~pysvn_enum()
    { }

    virtual Py::Object getattr( const char *_name )
    {
        std::string name( _name );
        T value;

        if( name == "__methods__" )
        {
            return Py::List();
        }

        if( name == "__members__" )
        {
            return memberList( static_cast<T>( 0 ) );
        }

        if( toEnum( name, value ) )
        {
            return Py::asObject( new pysvn_enum_value<T>( value ) );
        }

        return this->getattr_methods( _name );    
    }

    static void init_type(void);
};

template<TEMPLATE_TYPENAME T> Py::Object toEnumValue( const T &value )
{
    return Py::asObject( new pysvn_enum_value<T>( value ) );
}

// true if path exists and is a directory
extern bool is_path_dir( const std::string &path );

// true if the path_or_url is a svn url
extern bool is_svn_url( const std::string &path_or_url );

// convert a path to what SVN likes only if its not a URL
extern std::string svnNormalisedIfPath( const std::string &unnormalised, SvnPool &pool );
// convert a URL to what SVN likes
extern std::string svnNormalisedUrl( const std::string &unnormalised, SvnPool &pool );
// convert a path to what SVN likes
extern std::string svnNormalisedPath( const std::string &unnormalised, SvnPool &pool );

// convert a path to what the native OS likes
extern std::string osNormalisedPath( const std::string &unnormalised, SvnPool &pool );

// need a type to convert from apr_time_t to signed_int64 to double
#if defined(_MSC_VER)
typedef signed __int64 signed_int64;
#else
typedef signed long long signed_int64;
#endif



//--------------------------------------------------------------------------------
//
//    PythonAllowThreads provides a exception safe
//    wrapper for the C idiom:
//
//    Py_BEGIN_ALLOW_THREADS
//    ...Do some blocking I/O operation...
//    Py_END_ALLOW_THREADS
//
//    IN C++ use PythonAllowThreads in main code:
//{
//    PythonAllowThreads main_permission;
//    ...Do some blocking I/O operation that may throw
//} // allow d'tor grabs the lock
//
//    In C++ use PythonDisallowThreads in callback code:
//{
//    PythonDisallowThreads permission( main_permission );
//    ... Python operations that may throw
//} // allow d'tor to release the lock
//
//--------------------------------------------------------------------------------
class PythonAllowThreads
{
public:
    // calls allowOtherThreads()
    PythonAllowThreads( pysvn_context &_context );
    // calls allowThisThread() if necessary
    ~PythonAllowThreads();

    void allowOtherThreads();
    void allowThisThread();
private:
    pysvn_context            &m_callbacks;
    PyThreadState            *m_save;
};

class PythonDisallowThreads
{
public:
    // calls allowThisThread()
    PythonDisallowThreads( PythonAllowThreads *_permission );
    // calls allowOtherThreads() if necessary
    ~PythonDisallowThreads();
private:
    PythonAllowThreads *m_permission;
};

//--------------------------------------------------------------------------------
extern Py::Object utf8_string_or_none( const char *str );
extern Py::Object utf8_string_or_none( const std::string &str );

extern Py::Object path_string_or_none( const char *str, SvnPool &pool );
extern Py::Object path_string_or_none( const std::string &str, SvnPool &pool );

extern apr_time_t convertStringToTime( const std::string &text, apr_time_t now, SvnPool &pool );

extern Py::Object propsToObject( apr_hash_t *props, SvnPool &pool );
#if defined( PYSVN_HAS_CLIENT_PROPGET5 )
extern Py::Object inheritedPropsToObject( apr_array_header_t *inherited_props, SvnPool &pool );
#endif
extern Py::Object revnumListToObject( apr_array_header_t *revs, SvnPool &pool );
extern void proplistToObject( Py::List &py_path_propmap_list, apr_array_header_t *props, SvnPool &pool );
extern Py::Bytes asUtf8Bytes( Py::Object obj );
extern apr_array_header_t *targetsFromStringOrList( Py::Object arg, SvnPool &pool );
extern Py::List toListOfStrings( Py::Object obj );
extern apr_array_header_t *arrayOfStringsFromListOfStrings( Py::Object arg, SvnPool &pool );
extern apr_hash_t *hashOfStringsFromDictOfStrings( Py::Object arg, SvnPool &pool );
extern Py::Object direntsToObject( apr_hash_t *props, SvnPool &pool );

Py::Object toObject( apr_time_t t );
Py::Object toObject( pysvn_commit_info_t *commit_info, int commit_style );

#if defined( PYSVN_HAS_SVN_COMMIT_INFO_T )
Py::Object toObject( CommitInfoResult &commit_info, const DictWrapper &wrapper_commit_info, int commit_style );
#endif

#if defined( PYSVN_HAS_CLIENT_STATUS_T )
extern Py::Object toObject
    (
    Py::String path,
    svn_client_status_t &svn_status,
    SvnPool &pool,
    const DictWrapper &wrapper_status2,
    const DictWrapper &wrapper_lock
    );
#endif

extern Py::Object toObject
    (
    Py::String path,
    pysvn_wc_status_t &svn_status,
    SvnPool &pool,
    const DictWrapper &wrapper_status,
    const DictWrapper &wrapper_entry,
    const DictWrapper &wrapper_lock
    );

extern Py::Object toObject
    (
    const svn_wc_entry_t &svn_entry,
    SvnPool &pool,
    const DictWrapper &wrapper_entry
    );

#if defined( PYSVN_HAS_CLIENT_INFO )
extern Py::Object toObject
    (
    const svn_info_t &info,
    const DictWrapper &wrapper_info,
    const DictWrapper &wrapper_lock,
    const DictWrapper &wrapper_wc_info
    );
#endif
#if defined( PYSVN_HAS_CLIENT_INFO3 )
extern Py::Object toObject
    (
    const svn_client_info2_t &info,
    SvnPool &pool,
    const DictWrapper &wrapper_info,
    const DictWrapper &wrapper_lock,
    const DictWrapper &wrapper_wc_info
    );
#endif
#if defined( PYSVN_HAS_CLIENT_LOCK )
extern Py::Object toObject
    (
    const svn_lock_t &lock,
    const DictWrapper &wrapper_lock
    );
#endif

extern void revisionKindCompatibleCheck
    (
    bool is_url,
    const svn_opt_revision_t &revision,
    const char *revision_name,
    const char *url_or_path_name
    );

//--------------------------------------------------------------------------------
