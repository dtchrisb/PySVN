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
//    pysvn_transaction.cpp
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
#include "svn_sorts.h"

#include "pysvn_static_strings.hpp"

static void convertReposTree
    (
    Py::Dict &dict,
    bool copy_info,
    svn_repos_node_t *node,
    const std::string &path,
    apr_pool_t *pool
    );

//--------------------------------------------------------------------------------
pysvn_transaction::pysvn_transaction
    (
    pysvn_module &_module,
    Py::Dict result_wrappers
    )
: m_module( _module )
, m_transaction()
, m_exception_style( 1 )
{
}

void pysvn_transaction::init
    (
    const std::string &repos_path,
    const std::string &transaction_name,
    bool is_revision
    )
{
    svn_error_t *error = m_transaction.init( repos_path, transaction_name,
        is_revision );
    if( error != NULL )
    {
        SvnException e( error );
        throw_client_error( e );
    }
}

pysvn_transaction::~pysvn_transaction()
{
}

void pysvn_transaction::throw_client_error( SvnException &e )
{
    throw Py::Exception(
        m_module.client_error,
        e.pythonExceptionArg( m_exception_style ) );
}

Py::Object pysvn_transaction::getattr( const char *_name )
{
    std::string name( _name );

    // std::cout << "getattr( " << name << " )" << std::endl << std::flush;
    
    if( name == name___members__ )
    {
        Py::List members;

        members.append( Py::String( name_exception_style ) );

        return members;
    }

    if( name == name_exception_style )
        return Py::Int( m_exception_style );

    return getattr_default( _name );
}

int pysvn_transaction::setattr( const char *_name, const Py::Object &value )
{
    std::string name( _name );
    if( name == name_exception_style )
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
    else
    {
        std::string msg( "Unknown attribute: " );
        msg += name;
        throw Py::AttributeError( msg );
    }
    return 0;
}

Py::Object pysvn_transaction::cmd_cat( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_path },
    { false, NULL }
    };
    FunctionArguments args( "cat", args_desc, a_args, a_kws );
    args.check();

    std::string path( args.getUtf8String( name_path ) );

    SvnPool pool( m_transaction );

    svn_stringbuf_t * stringbuf = svn_stringbuf_create( empty_string, pool );
    svn_stream_t * stream = svn_stream_from_stringbuf( stringbuf, pool );
    
    try
    {
        svn_error_t *error;

        svn_fs_root_t *txn_root = NULL;
        error = m_transaction.root( &txn_root, pool );
        if( error != NULL )
            throw SvnException( error );

        svn_stream_t * fstream;
        error = svn_fs_file_contents( &fstream, txn_root, path.c_str(), pool );
        if( error != NULL )
            throw SvnException( error );

        char buf[BUFSIZ];
        apr_size_t len = BUFSIZ;
        do 
        {
#if defined( PYSVN_HAS_STREAM_READ_FULL )
            error = svn_stream_read_full( fstream, buf, &len );
#else
            error = svn_stream_read( fstream, buf, &len );
#endif
            if( error != NULL )
                throw SvnException( error );
            error = svn_stream_write( stream, buf, &len );
            if( error != NULL )
                throw SvnException( error );
        } 
        while (len == BUFSIZ);
  
    }
    catch( SvnException &e )
    {
        throw_client_error( e );
    }

    // return the bytes as is to the application
    // we can assume nothing about them
    return Py::String( stringbuf->data, (int)stringbuf->len );
}

Py::Object pysvn_transaction::cmd_changed( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { false, name_copy_info },
    { false, name_base_dir },
    { false, name_low_water_mark },
    { false, name_send_deltas },
    { false, NULL }
    };
    FunctionArguments args( "changed", args_desc, a_args, a_kws );
    args.check();

    bool copy_info = args.getBoolean( name_copy_info, false );
    bool send_deltas = args.getBoolean( name_send_deltas, false );

    svn_revnum_t low_water_mark = args.getInteger( name_low_water_mark, SVN_INVALID_REVNUM );

    std::string base_dir( args.getUtf8String( name_base_dir, "" ) );

    SvnPool pool( m_transaction );
    svn_repos_node_t *tree = NULL;

    try
    {
        svn_error_t *error;
        svn_revnum_t base_rev;
        if( m_transaction.is_revision() )
        {
            base_rev = m_transaction.revision() - 1;
        }
        else
        {
            base_rev = svn_fs_txn_base_revision( m_transaction );
        }
        if( !SVN_IS_VALID_REVNUM( base_rev ) )
        {
            error = svn_error_create( SVN_ERR_FS_NO_SUCH_REVISION,
                NULL, "Transaction is not based on a revision" );
            throw SvnException( error );
        }

        // Get the base root.
        svn_fs_root_t *base_rev_root = NULL;
        error = svn_fs_revision_root( &base_rev_root, m_transaction, base_rev, pool );
        if( error != NULL )
            throw SvnException( error );

        svn_fs_root_t *txn_root = NULL;
        error = m_transaction.root( &txn_root, pool );
        if( error != NULL )
            throw SvnException( error );

        // Request our editor
        const svn_delta_editor_t *editor = NULL;
        void *edit_baton = NULL;
        error = svn_repos_node_editor( 
                    &editor,
                    &edit_baton,
                    m_transaction,
                    base_rev_root,
                    txn_root,
                    pool,
                    pool );
        if( error != NULL )
            throw SvnException( error );

        // Drive our editor
        error = svn_repos_replay2
                    (
                    txn_root,
                    base_dir.c_str(),
                    low_water_mark,
                    send_deltas,
                    editor,
                    edit_baton,
                    NULL,               // authz_read_func
                    NULL,               // authz_read_baton
                    pool
                    );

        if( error != NULL )
            throw SvnException( error );

        // Return the tree we just built
        tree = svn_repos_node_from_baton( edit_baton );
    }
    catch( SvnException &e )
    {
        throw_client_error( e );
    }

    Py::Dict dict;
    convertReposTree( dict, copy_info, tree, empty_string, pool );

    return dict;
}

Py::Object pysvn_transaction::cmd_list( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { false,  name_path },
    { false, NULL }
    };
    FunctionArguments args( "list", args_desc, a_args, a_kws );
    args.check();

    std::string path( args.getUtf8String( name_path, empty_string ) );

    SvnPool pool( m_transaction );

    apr_hash_t *dir_entries = NULL;

    try
    {
        svn_error_t *error;

        svn_fs_root_t *txn_root = NULL;
        error = m_transaction.root( &txn_root, pool );
        if( error != NULL )
            throw SvnException( error );

        svn_node_kind_t kind;
        error = svn_fs_check_path( &kind, txn_root, path.c_str(), pool );
        if( error != NULL )
            throw SvnException( error );

        if( kind == svn_node_none )
        {
            error = svn_error_createf( SVN_ERR_FS_NOT_FOUND, NULL, "Path '%s' does not exist", path.c_str() );
            throw SvnException( error );
        }

        if( kind != svn_node_dir )
        {
            error = svn_error_createf( SVN_ERR_FS_NOT_DIRECTORY, NULL, "Path '%s' is not a directory", path.c_str() );
            throw SvnException( error );
        }

        error = svn_fs_dir_entries( &dir_entries, txn_root, path.c_str(), pool );
        if( error != NULL )
            throw SvnException( error );

    }
    catch( SvnException &e )
    {
        throw_client_error( e );
    }

    return direntsToObject( dir_entries, pool );
}

#if 0
Py::Object pysvn_transaction::cmd_diff( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_path },
    { false, NULL }
    };
    FunctionArguments args( "diff", args_desc, a_args, a_kws );
    args.check();

    return Py::None();
}
#endif

Py::Object pysvn_transaction::cmd_propdel( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_prop_name },
    { true,  name_path },
    { false, NULL }
    };
    FunctionArguments args( "propdel", args_desc, a_args, a_kws );
    args.check();

    std::string prop_name( args.getUtf8String( name_prop_name ) );
    std::string path( args.getUtf8String( name_path ) );

    SvnPool pool( m_transaction );

    try
    {
        svn_error_t * error;

        svn_fs_root_t *txn_root = NULL;
        error = m_transaction.root( &txn_root, pool );
        if( error != NULL )
            throw SvnException( error );

        svn_node_kind_t kind;
        error = svn_fs_check_path( &kind, txn_root, path.c_str(), pool );
        if( error != NULL )
            throw SvnException( error );

        if( kind == svn_node_none )
        {
            error = svn_error_createf( SVN_ERR_FS_NOT_FOUND, NULL, "Path '%s' does not exist", path.c_str() );
            throw SvnException( error );
        }

        error = svn_fs_change_node_prop
                (
                txn_root,
                path.c_str(),
                prop_name.c_str(),
                NULL,                   // delete value
                pool
                );
        if( error != NULL )
            throw SvnException( error );
    }
    catch( SvnException &e )
    {
        throw_client_error( e );
    }

    return Py::None();
}

Py::Object pysvn_transaction::cmd_propget( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_prop_name },
    { true,  name_path },
    { false, NULL }
    };
    FunctionArguments args( "propget", args_desc, a_args, a_kws );
    args.check();

    std::string prop_name( args.getUtf8String( name_prop_name ) );
    std::string path( args.getUtf8String( name_path ) );

    SvnPool pool( m_transaction );

    svn_string_t *prop_val = NULL;

    try
    {
        svn_error_t * error;
        svn_fs_root_t *txn_root = NULL;
        error = m_transaction.root( &txn_root, pool );
        if( error != NULL )
            throw SvnException( error );

        svn_node_kind_t kind;
        error = svn_fs_check_path( &kind, txn_root, path.c_str(), pool );
        if( error != NULL )
            throw SvnException( error );

        if( kind == svn_node_none )
        {
            error = svn_error_createf( SVN_ERR_FS_NOT_FOUND, NULL, "Path '%s' does not exist", path.c_str() );
            throw SvnException( error );
        }

        error = svn_fs_node_prop( &prop_val, txn_root, path.c_str(), prop_name.c_str(), pool );
        if( error != NULL )
            throw SvnException( error );
    }
    catch( SvnException &e )
    {
        throw_client_error( e );
    }

    if( prop_val == NULL )
    {
        return Py::None();
    }
    else
    {
        return Py::String( prop_val->data, prop_val->len, name_utf8 );
    }
}

Py::Object pysvn_transaction::cmd_proplist( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_path },
    { false, NULL }
    };
    FunctionArguments args( "proplist", args_desc, a_args, a_kws );
    args.check();

    std::string path( args.getUtf8String( name_path ) );

    SvnPool pool( m_transaction);

    apr_hash_t *props = NULL;

    try
    {
        svn_error_t * error;
        svn_fs_root_t *txn_root = NULL;
        error = m_transaction.root( &txn_root, pool );
        if( error != NULL )
            throw SvnException( error );

        svn_node_kind_t kind;
        error = svn_fs_check_path( &kind, txn_root, path.c_str(), pool );
        if( error != NULL )
            throw SvnException( error );

        if( kind == svn_node_none )
        {
            error = svn_error_createf( SVN_ERR_FS_NOT_FOUND, NULL, "Path '%s' does not exist", path.c_str() );
            throw SvnException( error );
        }

        error = svn_fs_node_proplist( &props, txn_root, path.c_str(), pool );
        if( error != NULL )
            throw SvnException( error );
    }
    catch( SvnException &e )
    {
        throw_client_error( e );
    }

    return propsToObject( props, pool );
}

Py::Object pysvn_transaction::cmd_propset( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_prop_name },
    { true,  name_prop_value },
    { true,  name_path },
    { false, NULL }
    };
    FunctionArguments args( "propset", args_desc, a_args, a_kws );
    args.check();

    std::string prop_name( args.getUtf8String( name_prop_name ) );
    std::string prop_val( args.getUtf8String( name_prop_value ) );
    std::string path( args.getUtf8String( name_path ) );

    SvnPool pool( m_transaction );

    try
    {
        svn_error_t * error;

        svn_fs_root_t *txn_root = NULL;
        error = m_transaction.root( &txn_root, pool );
        if( error != NULL )
            throw SvnException( error );

        const svn_string_t *svn_prop_val = svn_string_ncreate( prop_val.c_str(), prop_val.size(), pool );

        svn_node_kind_t kind;
        error = svn_fs_check_path( &kind, txn_root, path.c_str(), pool );
        if( error != NULL )
            throw SvnException( error );

        if( kind == svn_node_none )
        {
            error = svn_error_createf( SVN_ERR_FS_NOT_FOUND, NULL, "Path '%s' does not exist", path.c_str() );
            throw SvnException( error );
        }

        error = svn_fs_change_node_prop
                (
                txn_root,
                path.c_str(),
                prop_name.c_str(),
                svn_prop_val,
                pool
                );
        if( error != NULL )
            throw SvnException( error );
    }
    catch( SvnException &e )
    {
        throw_client_error( e );
    }

    return Py::None();
}

Py::Object pysvn_transaction::cmd_revpropdel( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_prop_name },
    { false, NULL }
    };
    FunctionArguments args( "revpropdel", args_desc, a_args, a_kws );
    args.check();

    std::string prop_name( args.getUtf8String( name_prop_name ) );

    SvnPool pool( m_transaction );

#if defined( PYSVN_HAS_FS_CHANGE_REV_PROP2 )
    const svn_string_t *old_prop_value = NULL;
#endif

    try
    {
        svn_error_t *error;
        if( m_transaction.is_revision() )
        {
#if defined( PYSVN_HAS_FS_CHANGE_REV_PROP2 )
            error = svn_fs_change_rev_prop2
                (
                m_transaction,
                m_transaction.revision(),
                prop_name.c_str(),
                &old_prop_value,
                NULL,            // value = NULL
                pool
                );
#else
            error = svn_fs_change_rev_prop
                (
                m_transaction,
                m_transaction.revision(),
                prop_name.c_str(),
                NULL,            // value = NULL
                pool
                );
#endif
        }
        else
        {
            error = svn_fs_change_txn_prop
                (
                m_transaction,
                prop_name.c_str(),
                NULL,            // value = NULL
                pool
                );
        }
        if( error != NULL )
            throw SvnException( error );
    }
    catch( SvnException &e )
    {
        throw_client_error( e );
    }

#if defined( PYSVN_HAS_FS_CHANGE_REV_PROP2 )
    if( old_prop_value != NULL )
    {
        return Py::String( old_prop_value->data, (int)old_prop_value->len );
    }
#endif

    return Py::None();
}

Py::Object pysvn_transaction::cmd_revpropget( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_prop_name },
    { false, NULL }
    };
    FunctionArguments args( "revpropget", args_desc, a_args, a_kws );
    args.check();

    std::string prop_name( args.getUtf8String( name_prop_name ) );

    SvnPool pool( m_transaction );

    svn_string_t *prop_val = NULL;

    try
    {
        svn_error_t *error;
        if( m_transaction.is_revision() )
        {
            error = svn_fs_revision_prop
                (
                &prop_val,
                m_transaction,
                m_transaction.revision(),
                prop_name.c_str(),
                pool
                );
        }
        else
        {
            error = svn_fs_txn_prop
                (
                &prop_val,
                m_transaction,
                prop_name.c_str(),
                pool
                );
        }
        if( error != NULL )
            throw SvnException( error );
    }
    catch( SvnException &e )
    {
        throw_client_error( e );
    }

    if( prop_val == NULL )
    {
        return Py::None();
    }
    else
    {
        return Py::String( prop_val->data, prop_val->len, name_utf8 );
    }
}

Py::Object pysvn_transaction::cmd_revproplist( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { false, NULL }
    };
    FunctionArguments args( "revproplist", args_desc, a_args, a_kws );
    args.check();

    SvnPool pool( m_transaction);

    apr_hash_t *props = NULL;

    try
    {
        svn_error_t *error;
        if( m_transaction.is_revision() )
        {
            error = svn_fs_revision_proplist
                (
                &props,
                m_transaction,
                m_transaction.revision(),
                pool
                );
        }
        else
        {
            error = svn_fs_txn_proplist
                (
                &props,
                m_transaction,
                pool
                );
        }
        if( error != NULL )
            throw SvnException( error );
    }
    catch( SvnException &e )
    {
        throw_client_error( e );
    }

    return propsToObject( props, pool );
}

Py::Object pysvn_transaction::cmd_revpropset( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_prop_name },
    { true,  name_prop_value },
    { false, NULL }
    };
    FunctionArguments args( "revpropset", args_desc, a_args, a_kws );
    args.check();

    std::string prop_name( args.getUtf8String( name_prop_name ) );
    std::string prop_val( args.getUtf8String( name_prop_value ) );

    SvnPool pool( m_transaction );
        
#if defined( PYSVN_HAS_FS_CHANGE_REV_PROP2 )
    const svn_string_t *old_prop_value = NULL;
#endif

    try
    {
        const svn_string_t *svn_prop_val = svn_string_ncreate( prop_val.c_str(), prop_val.size(), pool );
        svn_error_t *error;
        if( m_transaction.is_revision() )
        {
#if defined( PYSVN_HAS_FS_CHANGE_REV_PROP2 )
            error = svn_fs_change_rev_prop2
                (
                m_transaction,
                m_transaction.revision(),
                prop_name.c_str(),
                &old_prop_value,
                svn_prop_val,
                pool
                );

#else
            error = svn_fs_change_rev_prop
                (
                m_transaction,
                m_transaction.revision(),
                prop_name.c_str(),
                svn_prop_val,
                pool
                );
#endif
        }
        else
        {
            error = svn_fs_change_txn_prop
                (
                m_transaction,
                prop_name.c_str(),
                svn_prop_val,
                pool
                );
        }
        if( error != NULL )
            throw SvnException( error );
    }
    catch( SvnException &e )
    {
        throw_client_error( e );
    }

#if defined( PYSVN_HAS_FS_CHANGE_REV_PROP2 )
    if( old_prop_value != NULL )
    {
        return Py::String( old_prop_value->data, (int)old_prop_value->len );
    }
#endif

    return Py::None();
}

void pysvn_transaction::init_type()
{
    behaviors().name("Transaction");
    behaviors().doc( pysvn_transaction_doc );
    behaviors().supportGetattr();
    behaviors().supportSetattr();

    add_keyword_method("cat", &pysvn_transaction::cmd_cat, pysvn_transaction_cat_doc );
    add_keyword_method("changed", &pysvn_transaction::cmd_changed, pysvn_transaction_changed_doc );
    add_keyword_method("list", &pysvn_transaction::cmd_list, pysvn_transaction_list_doc );
#if 0
    add_keyword_method("diff", &pysvn_transaction::cmd_diff, pysvn_transaction_diff_doc );
#endif
    add_keyword_method("propdel", &pysvn_transaction::cmd_propdel, pysvn_transaction_propdel_doc );
    add_keyword_method("propget", &pysvn_transaction::cmd_propget, pysvn_transaction_propget_doc );
    add_keyword_method("proplist", &pysvn_transaction::cmd_proplist, pysvn_transaction_proplist_doc );
    add_keyword_method("propset", &pysvn_transaction::cmd_propset, pysvn_transaction_propset_doc );

    add_keyword_method("revpropdel", &pysvn_transaction::cmd_revpropdel, pysvn_transaction_revpropdel_doc );
    add_keyword_method("revpropget", &pysvn_transaction::cmd_revpropget, pysvn_transaction_revpropget_doc );
    add_keyword_method("revproplist", &pysvn_transaction::cmd_revproplist, pysvn_transaction_revproplist_doc );
    add_keyword_method("revpropset", &pysvn_transaction::cmd_revpropset, pysvn_transaction_revpropset_doc );
}


static void convertReposTree
    (
    Py::Dict &dict,
    bool copy_info,
    svn_repos_node_t *node,
    const std::string &path,
    apr_pool_t *pool
    )
{
    if( node == NULL )
        return;

    bool is_changed = false;

    // is node changed?
    if( node->action == 'A' )
        is_changed = true;

    else if( node->action == 'D' )
        is_changed = true;

    else if( node->action == 'R' )
    {
        if( node->text_mod )
            is_changed = true;
        if( node->prop_mod )
            is_changed = true;
    }

    else
        is_changed = false;

    if( is_changed )
    {
        if( copy_info )
        {
            Py::Tuple value( 6 );
            char action[2] = {node->action, 0};
            value[0] = Py::String( action );
            value[1] = toEnumValue( node->kind );
            value[2] = Py::Int( node->text_mod );
            value[3] = Py::Int( node->prop_mod );
            if( node->copyfrom_path == NULL )
                value[4] = Py::Int( 0 );
            else
                value[4] = Py::Int( node->copyfrom_rev );
            value[5] = utf8_string_or_none( node->copyfrom_path );

            dict[ Py::String( path, name_utf8 ) ] = value;
        }
        else
        {
            Py::Tuple value( 4 );
            char action[2] = {node->action, 0};
            value[0] = Py::String( action );
            value[1] = toEnumValue( node->kind );
            value[2] = Py::Int( node->text_mod );
            value[3] = Py::Int( node->prop_mod );

            dict[ Py::String( path, name_utf8 ) ] = value;
        }
    }

    /* Return here if the node has no children. */
    node = node->child;
    if( node == NULL )
        return;

    /* Recursively handle the node's children. */
    std::string full_path( path );
    if( !full_path.empty() )
        full_path += "/";
    full_path += node->name;

    convertReposTree( dict, copy_info, node, full_path, pool );
    while( node->sibling != NULL )
    {
        node = node->sibling;

        std::string full_path( path );
        if( !full_path.empty() )
            full_path += "/";
        full_path += node->name;

        convertReposTree( dict, copy_info, node, full_path, pool );
    }
}
