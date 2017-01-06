//
// ====================================================================
// Copyright (c) 2003-2011 Barry A Scott.  All rights reserved.
//
// This software is licensed as described in the file LICENSE.txt,
// which you should have received as part of this distribution.
//
// ====================================================================
//
//
//  pysvn.cpp
//

#if defined( _MSC_VER )
// disable warning C4786: symbol greater than 255 character,
// nessesary to ignore as <map> causes lots of warning
#pragma warning(disable: 4786)
#endif

#include "pysvn.hpp"
#include "pysvn_docs.hpp"
#include "pysvn_version.hpp"
#include "svn_version.h"

#include "pysvn_static_strings.hpp"

#ifdef _WIN32
// include to cause the windows manifest file to be created for pysvn.pyd
// as it contains the pragma with linker instructions for the C runtime libs
#include <use_ansi.h>
#endif

extern "C" int pysvn_breakpoint()
{
    return 0;
}

pysvn_module::pysvn_module()
: Py::ExtensionModule<pysvn_module>( "pysvn" )
, client_error()
{
    // init APR once globally - rather then on demand
    // to avoid life time issues with pools
    apr_initialize();
    apr_pool_initialize();

    client_error.init( *this, "ClientError" );

    pysvn_client::init_type();
    pysvn_transaction::init_type();
    pysvn_revision::init_type();

    pysvn_enum< svn_opt_revision_kind >::init_type();
    pysvn_enum_value< svn_opt_revision_kind >::init_type();

    pysvn_enum< svn_wc_notify_action_t >::init_type();
    pysvn_enum_value< svn_wc_notify_action_t >::init_type();

    pysvn_enum< svn_wc_status_kind >::init_type();
    pysvn_enum_value< svn_wc_status_kind >::init_type();

    pysvn_enum< svn_wc_schedule_t >::init_type();
    pysvn_enum_value< svn_wc_schedule_t >::init_type();

    pysvn_enum< svn_wc_merge_outcome_t >::init_type();
    pysvn_enum_value< svn_wc_merge_outcome_t >::init_type();

    pysvn_enum< svn_wc_notify_state_t >::init_type();
    pysvn_enum_value< svn_wc_notify_state_t >::init_type();

    pysvn_enum< svn_node_kind_t >::init_type();
    pysvn_enum_value< svn_node_kind_t >::init_type();

#if defined( PYSVN_HAS_DIFF_FILE_IGNORE_SPACE )
    pysvn_enum< svn_diff_file_ignore_space_t >::init_type();
    pysvn_enum_value< svn_diff_file_ignore_space_t >::init_type();
#endif

#if defined( PYSVN_HAS_CLIENT_DIFF_SUMMARIZE )
    pysvn_enum< svn_client_diff_summarize_kind_t >::init_type();
    pysvn_enum_value< svn_client_diff_summarize_kind_t >::init_type();
#endif
#ifdef QQQ
    pysvn_enum< svn_wc_conflict_action_t >::init_type();
    pysvn_enum_value< svn_wc_conflict_action_t >::init_type();
#endif
#if defined( PYSVN_HAS_SVN__DEPTH_PARAMETER )
    pysvn_enum< svn_depth_t >::init_type();
    pysvn_enum_value< svn_depth_t >::init_type();
#endif
#if defined( PYSVN_HAS_SVN_WC_CONFLICT_CHOICE_T )
    pysvn_enum< svn_wc_conflict_choice_t >::init_type();
    pysvn_enum_value< svn_wc_conflict_choice_t >::init_type();

    pysvn_enum< svn_wc_conflict_action_t >::init_type();
    pysvn_enum_value< svn_wc_conflict_action_t >::init_type();

    pysvn_enum< svn_wc_conflict_kind_t >::init_type();
    pysvn_enum_value< svn_wc_conflict_kind_t >::init_type();

    pysvn_enum< svn_wc_conflict_reason_t >::init_type();
    pysvn_enum_value< svn_wc_conflict_reason_t >::init_type();
#endif

#if defined( PYSVN_HAS_SVN_WC_OPERATION_T )
    pysvn_enum< svn_wc_operation_t >::init_type();
    pysvn_enum_value< svn_wc_operation_t >::init_type();
#endif

    add_keyword_method( "_Client", &pysvn_module::new_client, pysvn_client_doc );
    add_keyword_method( "Revision", &pysvn_module::new_revision, pysvn_revision_doc );
    add_keyword_method( "_Transaction", &pysvn_module::new_transaction, pysvn_transaction_doc );

    initialize( pysvn_module_doc );

    Py::Dict d( moduleDictionary() );

    d["ClientError"] = client_error;

    d["copyright"] = Py::String( copyright_doc );
    Py::Tuple version(4);
    version[0] = Py::Int( version_major );
    version[1] = Py::Int( version_minor );
    version[2] = Py::Int( version_patch );
    version[3] = Py::Int( version_build );

    d["version"] = version;

    Py::Tuple svn_api_version(4);
    svn_api_version[0] = Py::Int( SVN_VER_MAJOR );
    svn_api_version[1] = Py::Int( SVN_VER_MINOR );
    svn_api_version[2] = Py::Int( SVN_VER_MICRO );
    svn_api_version[3] = Py::String( SVN_VER_TAG );

#if defined( PYSVN_HAS_CLIENT_VERSION )
    const svn_version_t *client_version = svn_client_version();
    Py::Tuple svn_version(4);
    svn_version[0] = Py::Int( client_version->major );
    svn_version[1] = Py::Int( client_version->minor );
    svn_version[2] = Py::Int( client_version->patch );
    svn_version[3] = Py::String( client_version->tag );
    d["svn_version"] = svn_version;
    d["svn_api_version"] = svn_api_version;
#else
    d["svn_version"] = svn_api_version;
    d["svn_api_version"] = svn_api_version;
#endif

    d["opt_revision_kind"] = Py::asObject( new pysvn_enum< svn_opt_revision_kind >() );
    d["wc_notify_action"] = Py::asObject( new pysvn_enum< svn_wc_notify_action_t >() );
    d["wc_status_kind"] = Py::asObject( new pysvn_enum< svn_wc_status_kind >() );
    d["wc_schedule"] = Py::asObject( new pysvn_enum< svn_wc_schedule_t >() );
    d["wc_merge_outcome"] = Py::asObject( new pysvn_enum< svn_wc_merge_outcome_t >() );
    d["wc_notify_state"] = Py::asObject( new pysvn_enum< svn_wc_notify_state_t >() );
    d["node_kind"] = Py::asObject( new pysvn_enum< svn_node_kind_t >() );
#if defined( PYSVN_HAS_CLIENT_DIFF_SUMMARIZE )
    d["diff_summarize_kind"] = Py::asObject( new pysvn_enum< svn_client_diff_summarize_kind_t >() );
#endif
#if defined( PYSVN_HAS_SVN__DEPTH_PARAMETER )
    d["depth"] = Py::asObject( new pysvn_enum< svn_depth_t >() );
#endif
#if defined( PYSVN_HAS_SVN_WC_CONFLICT_CHOICE_T )
    d["wc_conflict_choice"] = Py::asObject( new pysvn_enum< svn_wc_conflict_choice_t >() );
    d["wc_conflict_action"] = Py::asObject( new pysvn_enum< svn_wc_conflict_action_t >() );
    d["wc_conflict_kind"] = Py::asObject( new pysvn_enum< svn_wc_conflict_kind_t >() );
    d["wc_conflict_reason"] = Py::asObject( new pysvn_enum< svn_wc_conflict_reason_t >() );
#endif
#if defined( PYSVN_HAS_SVN_WC_OPERATION_T )
    d["wc_operation"] = Py::asObject( new pysvn_enum< svn_wc_operation_t >() );
#endif
}

pysvn_module::~pysvn_module()
{
}


Py::Object pysvn_module::new_client( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { false, name_config_dir },
    { false, name_result_wrappers },
    { false, NULL }
    };

    FunctionArguments args( "Client", args_desc, a_args, a_kws );
    args.check();

    std::string config_dir = args.getUtf8String( name_config_dir, "" );

    Py::Dict result_wrappers_dict;
    if( args.hasArg( name_result_wrappers ) )
        result_wrappers_dict = args.getArg( name_result_wrappers );

    return Py::asObject( new pysvn_client( *this, config_dir, result_wrappers_dict ) );
}



Py::Object pysvn_module::new_transaction( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true, name_repos_path },
    { true, name_transaction_name },
    { false, name_is_revision },
    { false, name_result_wrappers },
    { false, NULL }
    };

    FunctionArguments args( "Transaction", args_desc, a_args, a_kws );
    args.check();

    std::string repos_path = args.getUtf8String( name_repos_path );
    std::string transaction_name = args.getUtf8String( name_transaction_name );
    bool is_revision = args.getBoolean( name_is_revision, false );

    Py::Dict result_wrappers_dict;
    if( args.hasArg( name_result_wrappers ) )
        result_wrappers_dict = args.getArg( name_result_wrappers );

    pysvn_transaction *t = new pysvn_transaction( *this, result_wrappers_dict );
    Py::Object result( Py::asObject( t ) );
    t->init( repos_path, transaction_name, is_revision );
    return result;
}


Py::Object pysvn_module::new_revision( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    //
    // support only one of the following:
    // revision( kind )
    // revision( kind, number )
    // revision( kind, date )
    //
    static argument_description args_desc_generic[] =
    {
    { true, name_kind },
    { false, name_date },
    { false, name_number },
    { false, NULL }
    };
    FunctionArguments args_generic( "Revision", args_desc_generic, a_args, a_kws );
    args_generic.check();

    Py::ExtensionObject< pysvn_enum_value<svn_opt_revision_kind> > py_kind( args_generic.getArg( name_kind ) );

    svn_opt_revision_kind kind = svn_opt_revision_kind( py_kind.extensionObject()->m_value );

    pysvn_revision *rev = NULL;
    switch( kind )
    {
        case svn_opt_revision_date:
        {
            static argument_description args_desc_date[] =
            {
            { true, name_kind },
            { true, name_date },
            { false, NULL }
            };
            FunctionArguments args_date( "Revision", args_desc_date, a_args, a_kws );
            args_date.check();
            Py::Float date( args_date.getArg( name_date ) );

            rev = new pysvn_revision( kind, double(date) );
            break;
        }

        case svn_opt_revision_number:
        {
            static argument_description args_desc_number[] =
            {
            { true, name_kind },
            { true, name_number },
            { false, NULL }
            };
            FunctionArguments args_number( "Revision", args_desc_number, a_args, a_kws );
            args_number.check();
            Py::Int revnum( args_number.getArg( name_number ) );

            rev = new pysvn_revision( kind, 0, long( revnum ) );
            break;
        }

        default:
        {
            static argument_description args_desc_other[] =
            {
            { true, name_kind },
            { false, NULL }
            };
            FunctionArguments args_other( "Revision", args_desc_other, a_args, a_kws );
            args_other.check();

            rev = new pysvn_revision( kind );
        }
    }

    return Py::asObject( rev );
}


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
//    In C++ use PythonDisallowTheads in callback code:
//{
//    PythonDisallowTheads permission( main_permission );
//    ... Python operations that may throw
//} // allow d'tor to release the lock
//
//--------------------------------------------------------------------------------
PythonAllowThreads::PythonAllowThreads( pysvn_context &_callbacks )
: m_callbacks( _callbacks )
, m_save( NULL )
{
    m_callbacks.setPermission( *this );
    allowOtherThreads();
}

PythonAllowThreads::~PythonAllowThreads()
{
    if( m_save != NULL )
        allowThisThread();

    m_callbacks.clearPermission();
}

void PythonAllowThreads::allowOtherThreads()
{
#if defined( WITH_THREAD )
    assert( m_save == NULL );

    m_save = PyEval_SaveThread();
    assert( m_save != NULL );
#endif
}

void PythonAllowThreads::allowThisThread()
{
#if defined( WITH_THREAD )
    assert( m_save != NULL );

    PyEval_RestoreThread( m_save );
    m_save = NULL;
#endif
}

PythonDisallowThreads::PythonDisallowThreads( PythonAllowThreads *_permission )
: m_permission( _permission )
{
    m_permission->allowThisThread();
}

PythonDisallowThreads::~PythonDisallowThreads()
{
    m_permission->allowOtherThreads();
}

//--------------------------------------------------------------------------------
#if defined(WIN32)
#define EXPORT_SYMBOL __declspec( dllexport )
#else
#define EXPORT_SYMBOL
#endif


static pysvn_module* the_pysvn_module = NULL;
#if PY_MAJOR_VERSION >= 3
extern "C" EXPORT_SYMBOL PyObject *PyInit__pysvn()
{
    the_pysvn_module = new pysvn_module;
    return the_pysvn_module->module().ptr();
}

// symbol required for the debug version
extern "C" EXPORT_SYMBOL PyObject *PyInit__pysvn_d()
{ 
    return PyInit__pysvn();
}

#else
extern "C" EXPORT_SYMBOL void init_pysvn()
{
    the_pysvn_module = new pysvn_module;
}

// symbol required for the debug version
extern "C" EXPORT_SYMBOL void init_pysvn_d()
{
    init_pysvn();
}
#endif
