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
//  pysvn_client_cmd_info.cpp
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

#if defined( PYSVN_HAS_CLIENT_ANNOTATE5 )
class AnnotatedLineInfo2
{
public:
    AnnotatedLineInfo2
        (
        apr_int64_t line_no,
        svn_revnum_t revision,
        apr_hash_t *rev_props,
        apr_hash_t *merged_rev_props,
        svn_revnum_t merged_revision,
        const char *merged_path,
        const char *line,
        svn_boolean_t local_change
        )
    : m_line_no( line_no )
    , m_revision( revision )
    , m_rev_props( rev_props ) //QQQ: need to copy?
    , m_merged_revision( merged_revision)
    , m_merged_rev_props( merged_rev_props ) //QQQ: need to copy?
    , m_merged_path()
    , m_line()
    , m_local_change( local_change )
    {
    if( merged_path != NULL )
        m_merged_path = merged_path;

    if( line != NULL )
        m_line = line;
    }

    ~AnnotatedLineInfo2()
    {
    }

    AnnotatedLineInfo2( const AnnotatedLineInfo2 &other )
    : m_line_no( other.m_line_no )
    , m_revision( other.m_revision )
    , m_rev_props( other.m_rev_props )
    , m_merged_revision( other.m_merged_revision )
    , m_merged_rev_props( other.m_merged_rev_props )
    , m_merged_path( other.m_merged_path )
    , m_line( other.m_line )
    , m_local_change( other.m_local_change )
    {
    }

    Py::Dict asDict( SvnPool &pool ) const
    {
        Py::Dict dict;
        dict[name_line] = Py::String( m_line );
        dict[name_number] = Py::Int( long( m_line_no ) );
        dict[name_revision] = Py::asObject( new pysvn_revision( svn_opt_revision_number, 0, m_revision ) );
        // rev_props
        dict[name_local_change] = Py::Boolean( m_local_change );
        if( SVN_IS_VALID_REVNUM( m_merged_revision ) )
        {
            dict[name_merged_revision] = Py::asObject( new pysvn_revision( svn_opt_revision_number, 0, m_merged_revision ) );
            dict[name_merged_path] = path_string_or_none( m_merged_path, pool );
            // merged_rev_props
        }
        else
        {
            dict[name_merged_revision] = Py::None();
            dict[name_merged_path] = Py::None();
            // merged_rev_props
        }

        return dict;
    }

    apr_int64_t     m_line_no;
    svn_revnum_t    m_revision;
    apr_hash_t     *m_rev_props;
    svn_revnum_t    m_merged_revision;
    apr_hash_t     *m_merged_rev_props;
    std::string     m_merged_path;
    std::string     m_line;
    bool            m_local_change;
};

extern "C" svn_error_t *annotate3_receiver
    (
    void *baton,
    svn_revnum_t start_revnum,
    svn_revnum_t end_revnum,
    apr_int64_t line_no,
    svn_revnum_t revision,
    apr_hash_t *rev_props,
    svn_revnum_t merged_revision,
    apr_hash_t *merged_rev_props,
    const char *merged_path,
    const char *line,
    svn_boolean_t local_change,
    apr_pool_t *pool
    );
class AnnotateBaton2
{
public:
    AnnotateBaton2()
    : m_all_entries()
    {
    }
    ~AnnotateBaton2()
    {
    }

    svn_client_blame_receiver3_t callback() { return &annotate3_receiver; }
    void *baton() { return static_cast<void *>( this ); };
    static AnnotateBaton2 *castBaton( void *baton_ ) { return static_cast<AnnotateBaton2 *>( baton_ ); }

    std::list<AnnotatedLineInfo2> m_all_entries;
};

extern "C" svn_error_t *annotate3_receiver
    (
    void *baton,
    svn_revnum_t start_revnum,
    svn_revnum_t end_revnum,
    apr_int64_t line_no,
    svn_revnum_t revision,
    apr_hash_t *rev_props,
    svn_revnum_t merged_revision,
    apr_hash_t *merged_rev_props,
    const char *merged_path,
    const char *line,
    svn_boolean_t local_change,
    apr_pool_t *pool
    )
{
    // There are cases when the author has been passed as NULL
    // protect against NULL passed for any of the strings
    if( merged_path == NULL )
        merged_path = "";
    if( line == NULL )
        line = "";
    
    AnnotateBaton2 *annotate_baton = AnnotateBaton2::castBaton( baton );

    annotate_baton->m_all_entries.push_back(
        AnnotatedLineInfo2(
            line_no,
            revision,
            rev_props,
            merged_rev_props,
            merged_revision,
            merged_path,
            line,
            local_change ) );

    return NULL;
}
#endif

#if defined( PYSVN_HAS_CLIENT_ANNOTATE4 )
class AnnotatedLineInfo
{
public:
    AnnotatedLineInfo
        (
        apr_int64_t line_no,
        svn_revnum_t revision,
        const char *author,
        const char *date,
        svn_revnum_t merged_revision,
        const char *merged_author,
        const char *merged_date,
        const char *merged_path,
        const char *line
        )
    : m_line_no( line_no )
    , m_revision( revision )
    , m_author()
    , m_date()
    , m_merged_revision( merged_revision)
    , m_merged_author()
    , m_merged_date()
    , m_merged_path()
    , m_line()
    {
    if( author != NULL )
        m_author = author;

    if( date != NULL )
        m_date = date;

    if( merged_author != NULL )
        m_merged_author = merged_author;

    if( merged_date != NULL )
        m_merged_date = merged_date;

    if( merged_path != NULL )
        m_merged_path = merged_path;

    if( line != NULL )
        m_line = line;
    }

    ~AnnotatedLineInfo()
    {
    }

    AnnotatedLineInfo( const AnnotatedLineInfo &other )
    : m_line_no( other.m_line_no )
    , m_revision( other.m_revision )
    , m_author( other.m_author )
    , m_date( other.m_date )
    , m_merged_revision( other.m_merged_revision )
    , m_merged_author( other.m_merged_author )
    , m_merged_date( other.m_merged_date )
    , m_merged_path( other.m_merged_path )
    , m_line( other.m_line )
    {
    }

    apr_int64_t m_line_no;
    svn_revnum_t m_revision;
    std::string m_author;
    std::string m_date;
    svn_revnum_t m_merged_revision;
    std::string m_merged_author;
    std::string m_merged_date;
    std::string m_merged_path;
    std::string m_line;
};

extern "C" svn_error_t *annotate2_receiver
    (
    void *baton,
    apr_int64_t line_no,
    svn_revnum_t revision,
    const char *author,
    const char *date,
    svn_revnum_t merged_revision,
    const char *merged_author,
    const char *merged_date,
    const char *merged_path,
    const char *line,
    apr_pool_t *pool
    );
class AnnotateBaton
{
public:
    AnnotateBaton()
    : m_all_entries()
    {
    }
    ~AnnotateBaton()
    {
    }

    svn_client_blame_receiver2_t callback() { return &annotate2_receiver; }
    void *baton() { return static_cast<void *>( this ); };
    static AnnotateBaton *castBaton( void *baton_ ) { return static_cast<AnnotateBaton *>( baton_ ); }

    std::list<AnnotatedLineInfo> m_all_entries;
};

extern "C" svn_error_t *annotate2_receiver
    (
    void *baton,
    apr_int64_t line_no,
    svn_revnum_t revision,
    const char *author,
    const char *date,
    svn_revnum_t merged_revision,
    const char *merged_author,
    const char *merged_date,
    const char *merged_path,
    const char *line,
    apr_pool_t *pool
    )
{
    // There are cases when the author has been passed as NULL
    // protect against NULL passed for any of the strings
    if( author == NULL )
        author = "";
    if( date == NULL )
        date = "";
    if( merged_author == NULL )
        merged_author = "";
    if( merged_date == NULL )
        merged_date = "";
    if( merged_path == NULL )
        merged_path = "";
    if( line == NULL )
        line = "";

    AnnotateBaton *annotate_baton = AnnotateBaton::castBaton( baton );
    annotate_baton->m_all_entries.push_back(
                    AnnotatedLineInfo(
                        line_no,
                        revision, author, date,
                        merged_revision, merged_author, merged_date, merged_path,
                        line ) );

    return NULL;
}

#else
class AnnotatedLineInfo
{
public:
    AnnotatedLineInfo
        (
        apr_int64_t line_no,
        svn_revnum_t revision,
        const char *author,
        const char *date,
        const char *line
        )
    : m_line_no( line_no )
    , m_revision( revision )
    , m_author( author )
    , m_date( date )
    , m_line( line )
    {
    }
    
    ~AnnotatedLineInfo()
    {
    }

    AnnotatedLineInfo( const AnnotatedLineInfo &other )
    : m_line_no( other.m_line_no )
    , m_revision( other.m_revision )
    , m_author( other.m_author )
    , m_date( other.m_date )
    , m_line( other.m_line )
    {
    }

    apr_int64_t m_line_no;
    svn_revnum_t m_revision;
    std::string m_author;
    std::string m_date;
    std::string m_line;
};

extern "C" svn_error_t *annotate1_receiver
    (
    void *baton,
    apr_int64_t line_no,
    svn_revnum_t revision,
    const char *author,
    const char *date,
    const char *line,
    apr_pool_t *pool
    );

class AnnotateBaton
{
public:
    AnnotateBaton()
    : m_all_entries()
    {
    }
    ~AnnotateBaton()
    {
    }
    svn_client_blame_receiver_t callback() { return &annotate1_receiver; }
    void *baton() { return static_cast<void *>( this ); };
    static AnnotateBaton *castBaton( void *baton_ ) { return static_cast<AnnotateBaton *>( baton_ ); }

    std::list<AnnotatedLineInfo> m_all_entries;
};

extern "C" svn_error_t *annotate1_receiver
    (
    void *baton,
    apr_int64_t line_no,
    svn_revnum_t revision,
    const char *author,
    const char *date,
    const char *line,
    apr_pool_t *pool
    )
{
    // There are cases when the author has been passed as NULL
    // protect against NULL passed for any of the strings
    if( author == NULL )
        author = "";
    if( date == NULL )
        date = "";
    if( line == NULL )
        line = "";

    AnnotateBaton *annotate_baton = AnnotateBaton::castBaton( baton );
    annotate_baton->m_all_entries.push_back( AnnotatedLineInfo( line_no, revision, author, date, line ) );

    return NULL;
}
#endif

#if defined( PYSVN_HAS_CLIENT_ANNOTATE5 )
Py::Object pysvn_client::cmd_annotate2( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_url_or_path },
    { false, name_revision_start },
    { false, name_revision_end },
    { false, name_peg_revision },
    { false, name_ignore_space },
    { false, name_ignore_eol_style },
    { false, name_ignore_mime_type },
    { false, name_include_merged_revisions },
    { false, NULL }
    };
    FunctionArguments args( "annotate", args_desc, a_args, a_kws );
    args.check();

    std::string path( args.getUtf8String( name_url_or_path, empty_string ) );
    svn_opt_revision_t revision_start = args.getRevision( name_revision_start, svn_opt_revision_number );
    svn_opt_revision_t revision_end = args.getRevision( name_revision_end, svn_opt_revision_head );
    svn_opt_revision_t peg_revision = args.getRevision( name_peg_revision, revision_end );

    svn_diff_file_ignore_space_t ignore_space = svn_diff_file_ignore_space_none;
    if( args.hasArg( name_ignore_space ) )
    {
        Py::ExtensionObject< pysvn_enum_value<svn_diff_file_ignore_space_t> > py_ignore_space( args.getArg( name_ignore_space ) );
        ignore_space = svn_diff_file_ignore_space_t( py_ignore_space.extensionObject()->m_value );
    }

    svn_boolean_t ignore_eol_style = args.getBoolean( name_ignore_eol_style, false );
    svn_boolean_t ignore_mime_type = args.getBoolean( name_ignore_mime_type, false );
    svn_boolean_t include_merged_revisions = args.getBoolean( name_include_merged_revisions, false );
    SvnPool pool( m_context );

    svn_diff_file_options_t *diff_options = svn_diff_file_options_create( pool );
    diff_options->ignore_space = ignore_space;
    diff_options->ignore_eol_style = ignore_eol_style;

    bool is_url = is_svn_url( path );
    revisionKindCompatibleCheck( is_url, peg_revision, name_peg_revision, name_url_or_path );
    revisionKindCompatibleCheck( is_url, revision_start, name_revision_start, name_url_or_path );
    revisionKindCompatibleCheck( is_url, revision_end, name_revision_end, name_url_or_path );

    AnnotateBaton2 annotate_baton;

    try
    {
        std::string norm_path( svnNormalisedIfPath( path, pool ) );

        checkThreadPermission();

        PythonAllowThreads permission( m_context );

        svn_error_t *error = svn_client_blame5
            (
            norm_path.c_str(),
            &peg_revision,
            &revision_start,
            &revision_end,
            diff_options,
            ignore_mime_type,
            include_merged_revisions,
            annotate_baton.callback(),
            annotate_baton.baton(),
            m_context,
            pool
            );

        permission.allowThisThread();
        if( error != NULL )
        {
            throw SvnException( error );
        }
    }
    catch( SvnException &e )
    {
        // use callback error over ClientException
        m_context.checkForError( m_module.client_error );

        throw_client_error( e );
    }

    // convert the entries into python objects
    Py::List all_lines;
    for( std::list<AnnotatedLineInfo2>::const_iterator entry_it = annotate_baton.m_all_entries.begin();
        entry_it != annotate_baton.m_all_entries.end();
        ++entry_it )
    {
        all_lines.append( entry_it->asDict( pool ) );
    }

    return all_lines;
}
#endif

Py::Object pysvn_client::cmd_annotate( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_url_or_path },
    { false, name_revision_start },
    { false, name_revision_end },
#if defined( PYSVN_HAS_CLIENT_ANNOTATE2 )
    { false, name_peg_revision },
#endif
#if defined( PYSVN_HAS_CLIENT_ANNOTATE3 )
    { false, name_ignore_space },
    { false, name_ignore_eol_style },
    { false, name_ignore_mime_type },
#endif
#if defined( PYSVN_HAS_CLIENT_ANNOTATE4 )
    { false, name_include_merged_revisions },
#endif
    { false, NULL }
    };
    FunctionArguments args( "annotate", args_desc, a_args, a_kws );
    args.check();

    std::string path( args.getUtf8String( name_url_or_path, empty_string ) );
    svn_opt_revision_t revision_start = args.getRevision( name_revision_start, svn_opt_revision_number );
    svn_opt_revision_t revision_end = args.getRevision( name_revision_end, svn_opt_revision_head );
#if defined( PYSVN_HAS_CLIENT_ANNOTATE2 )
    svn_opt_revision_t peg_revision = args.getRevision( name_peg_revision, revision_end );
#endif
#if defined( PYSVN_HAS_CLIENT_ANNOTATE3 )
    svn_diff_file_ignore_space_t ignore_space = svn_diff_file_ignore_space_none;
    if( args.hasArg( name_ignore_space ) )
    {
        Py::ExtensionObject< pysvn_enum_value<svn_diff_file_ignore_space_t> > py_ignore_space( args.getArg( name_ignore_space ) );
        ignore_space = svn_diff_file_ignore_space_t( py_ignore_space.extensionObject()->m_value );
    }

    svn_boolean_t ignore_eol_style = args.getBoolean( name_ignore_eol_style, false );
    svn_boolean_t ignore_mime_type = args.getBoolean( name_ignore_mime_type, false );
#endif
#if defined( PYSVN_HAS_CLIENT_ANNOTATE4 )
    svn_boolean_t include_merged_revisions = args.getBoolean( name_include_merged_revisions, false );
#endif
    SvnPool pool( m_context );

#if defined( PYSVN_HAS_CLIENT_ANNOTATE3 )
    svn_diff_file_options_t *diff_options = svn_diff_file_options_create( pool );
    diff_options->ignore_space = ignore_space;
    diff_options->ignore_eol_style = ignore_eol_style;
#endif

    bool is_url = is_svn_url( path );
#if defined( PYSVN_HAS_CLIENT_ANNOTATE2 )
    revisionKindCompatibleCheck( is_url, peg_revision, name_peg_revision, name_url_or_path );
#endif
    revisionKindCompatibleCheck( is_url, revision_start, name_revision_start, name_url_or_path );
    revisionKindCompatibleCheck( is_url, revision_end, name_revision_end, name_url_or_path );

    AnnotateBaton annotate_baton;

    try
    {
        std::string norm_path( svnNormalisedIfPath( path, pool ) );

        checkThreadPermission();

        PythonAllowThreads permission( m_context );

#if defined( PYSVN_HAS_CLIENT_ANNOTATE4 )
        svn_error_t *error = svn_client_blame4 // ignore deprecation warning - support old Client().annotate()
            (
            norm_path.c_str(),
            &peg_revision,
            &revision_start,
            &revision_end,
            diff_options,
            ignore_mime_type,
            include_merged_revisions,
            annotate_baton.callback(),
            annotate_baton.baton(),
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_ANNOTATE3 )
        svn_error_t *error = svn_client_blame3
            (
            norm_path.c_str(),
            &peg_revision,
            &revision_start,
            &revision_end,
            diff_options,
            ignore_mime_type,
            annotate_receiver,
            annotate_baton.callback(),
            annotate_baton.baton(),
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_ANNOTATE2 )
        svn_error_t *error = svn_client_blame2
            (
            norm_path.c_str(),
            &peg_revision,
            &revision_start,
            &revision_end,
            annotate_baton.callback(),
            annotate_baton.baton(),
            m_context,
            pool
            );
#else
        svn_error_t *error = svn_client_blame
            (
            norm_path.c_str(),
            &revision_start,
            &revision_end,
            annotate_baton.callback(),
            annotate_baton.baton(),
            m_context,
            pool
            );
#endif
        permission.allowThisThread();
        if( error != NULL )
        {
            throw SvnException( error );
        }
    }
    catch( SvnException &e )
    {
        // use callback error over ClientException
        m_context.checkForError( m_module.client_error );

        throw_client_error( e );
    }

    // convert the entries into python objects
    Py::List entries_list;
    std::list<AnnotatedLineInfo>::const_iterator entry_it = annotate_baton.m_all_entries.begin();
    while( entry_it != annotate_baton.m_all_entries.end() )
    {
        const AnnotatedLineInfo &entry = *entry_it;
        ++entry_it;

        Py::Dict entry_dict;
        entry_dict[name_author] = Py::String( entry.m_author, name_utf8 );
        entry_dict[name_date] = Py::String( entry.m_date );
        entry_dict[name_line] = Py::String( entry.m_line );
        entry_dict[name_number] = Py::Int( long( entry.m_line_no ) );
        entry_dict[name_revision] = Py::asObject( new pysvn_revision( svn_opt_revision_number, 0, entry.m_revision ) );

        entries_list.append( entry_dict );
    }

    return entries_list;
}

Py::Object pysvn_client::cmd_info( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_path },
    { false, NULL }
    };
    FunctionArguments args( "info", args_desc, a_args, a_kws );
    args.check();

    std::string path( args.getUtf8String( name_path ) );

    SvnPool pool( m_context );

    const svn_wc_entry_t *entry = NULL;

    try
    {
        checkThreadPermission();

        PythonAllowThreads permission( m_context );

        svn_wc_adm_access_t *adm_access = NULL;

#if defined( PYSVN_HAS_WC_ADM_PROBE_OPEN3 )
        const char *c_norm_path = svn_dirent_internal_style( path.c_str(), pool );
        std::string norm_path( c_norm_path );
        svn_error_t *error = svn_wc_adm_probe_open3 // ignore deprecation warning - support old Client().info()
                    (
                    &adm_access,
                    NULL,
                    norm_path.c_str(),
                    false,
                    0,
                    NULL,
                    NULL,
                    pool
                    );
#else
        std::string norm_path( svnNormalisedPath( path, pool ) );
        svn_error_t *error = svn_wc_adm_probe_open( &adm_access, NULL, norm_path.c_str(), false, false, pool );
#endif

        permission.allowThisThread();
        if( error != NULL )
            throw SvnException( error );

        permission.allowOtherThreads();
        error = svn_wc_entry // ignore deprecation warning - support old Client().info()
                    (
                    &entry,
                    norm_path.c_str(),
                    adm_access,
                    false,
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
        return Py::None();       // needed to remove warning about return value missing
    }

    if( entry == NULL )
        return Py::None();

    return toObject( *entry, pool, m_wrapper_entry );
}

#if defined( PYSVN_HAS_CLIENT_INFO3 )
extern "C" svn_error_t *info_receiver_c2( void *baton_, const char *abspath_or_url, const svn_client_info2_t *info, apr_pool_t *pool );

class InfoReceiveBaton
{
public:
    InfoReceiveBaton
        (
        PythonAllowThreads *permission,
        SvnPool &pool,
        Py::List &info_list,
        const DictWrapper &wrapper_info,
        const DictWrapper &wrapper_lock,
        const DictWrapper &wrapper_wc_info
        )
    : m_permission( permission )
    , m_pool( pool )
    , m_info_list( info_list )
    , m_wrapper_info( wrapper_info )
    , m_wrapper_lock( wrapper_lock )
    , m_wrapper_wc_info( wrapper_wc_info )
    {}
    ~InfoReceiveBaton()
    {}

    svn_client_info_receiver2_t callback() { return &info_receiver_c2; }
    void *baton() { return static_cast<void *>( this ); }
    static InfoReceiveBaton *castBaton( void *baton_ ) { return static_cast<InfoReceiveBaton *>( baton_ ); }

    PythonAllowThreads  *m_permission;
    SvnPool             &m_pool;
    Py::List            &m_info_list;
    const DictWrapper   &m_wrapper_info;
    const DictWrapper   &m_wrapper_lock;
    const DictWrapper   &m_wrapper_wc_info;
};

extern "C" svn_error_t *info_receiver_c2( void *baton_, const char *abspath_or_url, const svn_client_info2_t *info, apr_pool_t * )
{
    InfoReceiveBaton *baton = InfoReceiveBaton::castBaton( baton_ );

    PythonDisallowThreads callback_permission( baton->m_permission );

    if( abspath_or_url != NULL )
    {
        std::string std_path( abspath_or_url );
        if( std_path.empty() )
        {
            std_path = ".";
        }
        Py::String py_path( utf8_string_or_none( std_path ) );

        Py::Tuple py_pair( 2 );
        py_pair[0] = py_path;
        py_pair[1] = toObject(
                    *info,
                    baton->m_pool,
                    baton->m_wrapper_info,
                    baton->m_wrapper_lock,
                    baton->m_wrapper_wc_info );

        baton->m_info_list.append( py_pair );
    }

    return SVN_NO_ERROR;
}

#elif defined( PYSVN_HAS_CLIENT_INFO )
extern "C" svn_error_t *info_receiver_c( void *baton_, const char *path, const svn_info_t *info, apr_pool_t *pool );

class InfoReceiveBaton
{
public:
    InfoReceiveBaton
        (
        PythonAllowThreads *permission,
        SvnPool &,
        Py::List &info_list,
        const DictWrapper &wrapper_info,
        const DictWrapper &wrapper_lock,
        const DictWrapper &wrapper_wc_info
        )
    : m_permission( permission )
    , m_info_list( info_list )
    , m_wrapper_info( wrapper_info )
    , m_wrapper_lock( wrapper_lock )
    , m_wrapper_wc_info( wrapper_wc_info )
    {}
    ~InfoReceiveBaton()
    {}

    svn_info_receiver_t callback() { return &info_receiver_c; }
    void *baton() { return static_cast<void *>( this ); }
    static InfoReceiveBaton *castBaton( void *baton_ ) { return static_cast<InfoReceiveBaton *>( baton_ ); }

    PythonAllowThreads  *m_permission;
    Py::List            &m_info_list;
    const DictWrapper   &m_wrapper_info;
    const DictWrapper   &m_wrapper_lock;
    const DictWrapper   &m_wrapper_wc_info;
};

extern "C" svn_error_t *info_receiver_c( void *baton_, const char *path, const svn_info_t *info, apr_pool_t *pool )
{
    InfoReceiveBaton *baton = InfoReceiveBaton::castBaton( baton_ );

    PythonDisallowThreads callback_permission( baton->m_permission );

    if( path != NULL )
    {
        std::string std_path( path );
        if( std_path.empty() )
        {
            std_path = ".";
        }
        Py::String py_path( utf8_string_or_none( std_path ) );

        Py::Tuple py_pair( 2 );
        py_pair[0] = py_path;
        py_pair[1] = toObject(
                    *info,
                    baton->m_wrapper_info,
                    baton->m_wrapper_lock,
                    baton->m_wrapper_wc_info );

        baton->m_info_list.append( py_pair );
    }

    return SVN_NO_ERROR;
}
#endif

#if defined( PYSVN_HAS_CLIENT_INFO )
Py::Object pysvn_client::cmd_info2( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_url_or_path },
    { false, name_revision },
    { false, name_peg_revision},
    { false, name_recurse },
#if defined( PYSVN_HAS_CLIENT_INFO2 )
    { false, name_depth },
    { false, name_changelists },
#endif
#if defined( PYSVN_HAS_CLIENT_INFO3 )
    { false, name_fetch_excluded },
    { false, name_fetch_actual_only },
#endif
#if defined( PYSVN_HAS_CLIENT_INFO3 )
    { false, name_include_externals },
#endif
    { false, NULL }
    };
    FunctionArguments args( "info2", args_desc, a_args, a_kws );
    args.check();

    std::string path( args.getUtf8String( name_url_or_path ) );

    svn_opt_revision_kind kind = svn_opt_revision_unspecified;
    if( is_svn_url( path ) )
        kind = svn_opt_revision_head;

    svn_opt_revision_t revision = args.getRevision( name_revision, kind );
    svn_opt_revision_t peg_revision = args.getRevision( name_peg_revision, revision );

    SvnPool pool( m_context );

#if defined( PYSVN_HAS_CLIENT_INFO2 )
    apr_array_header_t *changelists = NULL;

    if( args.hasArg( name_changelists ) )
    {
        changelists = arrayOfStringsFromListOfStrings( args.getArg( name_changelists ), pool );
    }

    svn_depth_t depth = args.getDepth( name_depth, name_recurse, svn_depth_infinity, svn_depth_infinity, svn_depth_empty );
#else
    bool recurse = args.getBoolean( name_recurse, true );
#endif

#if defined( PYSVN_HAS_CLIENT_INFO3 )
    bool fetch_excluded = args.getBoolean( name_fetch_excluded, false );
    bool fetch_actual_only = args.getBoolean( name_fetch_actual_only, true );
#endif
#if defined( PYSVN_HAS_CLIENT_INFO3 )
    bool include_externals = args.getBoolean( name_include_externals, false );
#endif

    bool is_url = is_svn_url( path );
    revisionKindCompatibleCheck( is_url, peg_revision, name_peg_revision, name_url_or_path );
    revisionKindCompatibleCheck( is_url, revision, name_revision, name_url_or_path );

    Py::List info_list;

    try
    {
        std::string norm_path( svnNormalisedIfPath( path, pool ) );

        checkThreadPermission();

        PythonAllowThreads permission( m_context );

        InfoReceiveBaton info_baton( &permission, pool, info_list, m_wrapper_info, m_wrapper_lock, m_wrapper_wc_info );

#if defined( PYSVN_HAS_CLIENT_INFO3 )
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
#endif


#if defined( PYSVN_HAS_CLIENT_INFO4 )
        if( error == SVN_NO_ERROR )
        {
            error = svn_client_info4
                    (
                    abspath_or_url,
                    &peg_revision,
                    &revision,
                    depth,
                    fetch_excluded,
                    fetch_actual_only,
                    include_externals,
                    changelists,
                    info_baton.callback(),
                    info_baton.baton(),
                    m_context,
                    pool
                    );
        }
#elif defined( PYSVN_HAS_CLIENT_INFO3 )
        if( error == SVN_NO_ERROR )
        {
            error = svn_client_info3
                    (
                    abspath_or_url,
                    &peg_revision,
                    &revision,
                    depth,
                    fetch_excluded,
                    fetch_actual_only,
                    changelists,
                    info_baton.callback(),
                    info_baton.baton(),
                    m_context,
                    pool
                    );
        }
#elif defined( PYSVN_HAS_CLIENT_INFO2 )
        svn_error_t *error = 
            svn_client_info2
                (
                norm_path.c_str(),
                &peg_revision,
                &revision,
                info_baton.callback(),
                info_baton.baton(),
                depth,
                changelists,
                m_context,
                pool
                );
#else
        svn_error_t *error = 
            svn_client_info
                (
                norm_path.c_str(),
                &peg_revision,
                &revision,
                info_baton.callback(),
                info_baton.baton(),
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

    return info_list;
}
#endif

#if defined( PYSVN_HAS_CLIENT_LOG4 )
extern "C" svn_error_t *log4Receiver
    (
    void *baton_,
    svn_log_entry_t *log_entry,
    apr_pool_t *pool
    );

class Log4Baton
{
public:
    Log4Baton( PythonAllowThreads *permission, SvnPool &pool, Py::List &log_list )
        : m_permission( permission )
        , m_pool( pool )
        , m_now( apr_time_now() )
        , m_wrapper_log( NULL )
        , m_wrapper_log_changed_path( NULL )
        , m_log_list( log_list )
        , m_has_children( false )
        {}
    ~Log4Baton()
        {}

    svn_log_entry_receiver_t callback() { return &log4Receiver; }
    void *baton() { return static_cast< void * >( this ); }
    static Log4Baton *castBaton( void *baton_ ) { return static_cast<Log4Baton *>( baton_ ); }

    PythonAllowThreads  *m_permission;
    SvnPool             &m_pool;
    apr_time_t          m_now;
    DictWrapper         *m_wrapper_log;
    DictWrapper         *m_wrapper_log_changed_path;
    Py::List            &m_log_list;
    bool                m_has_children;
};

extern "C" svn_error_t *log4Receiver
    (
    void *baton_,
    svn_log_entry_t *log_entry,
    apr_pool_t *pool
    )
{
    Log4Baton *baton = Log4Baton::castBaton( baton_ );

    if( log_entry->revision == 0 )
    {
        // skip this blank entry
        // as the svn log command does
        return NULL;
    }

    PythonDisallowThreads callback_permission( baton->m_permission );

    Py::Dict entry_dict;

    Py::Object revprops;
    if( log_entry->revprops == NULL )
    {
        revprops = Py::None();
    }
    else
    {
        revprops = propsToObject( log_entry->revprops, baton->m_pool );
        Py::Dict revprops_dict;
        revprops_dict = revprops;

        if( revprops_dict.hasKey( "svn:date" ) )
        {
            Py::String date( revprops_dict[ "svn:date" ] );
            Py::Object int_date = toObject( convertStringToTime( date.as_std_string( name_utf8 ), baton->m_now, baton->m_pool ) );
            revprops_dict[ "svn:date" ] = int_date;
            entry_dict[ name_date ] = int_date;
        }
        if( revprops_dict.hasKey( "svn:author" ) )
        {
            entry_dict[ name_author ] = revprops_dict[ "svn:author" ];
        }
        if( revprops_dict.hasKey( "svn:log" ) )
        {
            Py::String message( revprops_dict[ "svn:log" ] );
            revprops_dict[ "svn:log" ] = message;
            entry_dict[ name_message ] = message;
        }
    }
    entry_dict[ name_revprops ] = revprops;
    entry_dict[ name_revision ] = Py::asObject( new pysvn_revision( svn_opt_revision_number, 0, log_entry->revision ) );

    Py::List changed_paths_list;
#if defined( PYSVN_HAS_CLIENT_LOG5 )
    if( log_entry->changed_paths2 != NULL )
    {
        for( apr_hash_index_t *hi = apr_hash_first( pool, log_entry->changed_paths2 );
                hi != NULL;
                    hi = apr_hash_next( hi ) )
        {
            Py::Dict changed_entry_dict;

            char *path = NULL;
            void *val = NULL;
            apr_hash_this( hi, (const void **) &path, NULL, &val );

            svn_log_changed_path_t *log_item = reinterpret_cast<svn_log_changed_path_t *> (val);

            changed_entry_dict[ name_path ] = Py::String( path );

            char action[2]; action[0] = log_item->action; action[1] = 0;
            changed_entry_dict[ name_action ] = Py::String( action );

            changed_entry_dict[ name_copyfrom_path ] = utf8_string_or_none( log_item->copyfrom_path );

            if( SVN_IS_VALID_REVNUM( log_item->copyfrom_rev ) )
                changed_entry_dict[ name_copyfrom_revision ] =
                    Py::asObject( new pysvn_revision( svn_opt_revision_number, 0, log_item->copyfrom_rev ) );
            else
                changed_entry_dict[ name_copyfrom_revision ] = Py::None();

            changed_paths_list.append( baton->m_wrapper_log_changed_path->wrapDict( changed_entry_dict ) );
        }
    }
#else
    if( log_entry->changed_paths != NULL )
    {
        for( apr_hash_index_t *hi = apr_hash_first( pool, log_entry->changed_paths );
                hi != NULL;
                    hi = apr_hash_next( hi ) )
        {
            Py::Dict changed_entry_dict;

            char *path = NULL;
            void *val = NULL;
            apr_hash_this( hi, (const void **) &path, NULL, &val );

            svn_log_changed_path_t *log_item = reinterpret_cast<svn_log_changed_path_t *> (val);

            changed_entry_dict[ name_path ] = Py::String( path );

            char action[2]; action[0] = log_item->action; action[1] = 0;
            changed_entry_dict[ name_action ] = Py::String( action );

            changed_entry_dict[ name_copyfrom_path ] = utf8_string_or_none( log_item->copyfrom_path );

            if( SVN_IS_VALID_REVNUM( log_item->copyfrom_rev ) )
                changed_entry_dict[ name_copyfrom_revision ] =
                    Py::asObject( new pysvn_revision( svn_opt_revision_number, 0, log_item->copyfrom_rev ) );
            else
                changed_entry_dict[ name_copyfrom_revision ] = Py::None();

            changed_paths_list.append( baton->m_wrapper_log_changed_path->wrapDict( changed_entry_dict ) );
        }
    }
#endif

    entry_dict[ name_changed_paths ] = changed_paths_list;
    entry_dict[ "has_children" ] = Py::Int( log_entry->has_children != 0 ? 1 : 0 );

    baton->m_log_list.append( baton->m_wrapper_log->wrapDict( entry_dict ) );

    return NULL;
}

// PYSVN_HAS_CLIENT_LOG4 version
Py::Object pysvn_client::cmd_log( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_url_or_path },
    { false, name_revision_start },
    { false, name_revision_end },
    { false, name_discover_changed_paths },
    { false, name_strict_node_history },
    { false, name_limit },
    { false, name_peg_revision },
    { false, name_include_merged_revisions },
    { false, name_revprops },
    { false, NULL }
    };
    FunctionArguments args( "log", args_desc, a_args, a_kws );
    args.check();

    SvnPool pool( m_context );

    svn_opt_revision_t revision_start = args.getRevision( name_revision_start, svn_opt_revision_head );
    svn_opt_revision_t revision_end = args.getRevision( name_revision_end, svn_opt_revision_number );
    bool discover_changed_paths = args.getBoolean( name_discover_changed_paths, false );
    bool strict_node_history = args.getBoolean( name_strict_node_history, true );
    int limit = args.getInteger( name_limit, 0 );
    svn_opt_revision_t peg_revision = args.getRevision( name_peg_revision, svn_opt_revision_unspecified );

    svn_boolean_t include_merged_revisions = args.getBoolean( name_include_merged_revisions, false );
    apr_array_header_t *revprops = NULL;
    if( args.hasArg( name_revprops ) )
    {
        Py::Object py_revprop = args.getArg( name_revprops );
        if( !py_revprop.isNone() )
        {
            revprops = arrayOfStringsFromListOfStrings( py_revprop, pool );
        }
    }

    Py::Object url_or_path_obj = args.getArg( name_url_or_path );
    Py::List url_or_path_list;
    if( url_or_path_obj.isList() )
    {
        url_or_path_list = url_or_path_obj;
    }
    else
    {
        Py::List py_list;
        py_list.append( url_or_path_obj );
        url_or_path_list = py_list;
    }

    for( size_t i=0; i<url_or_path_list.size(); i++ )
    {
        Py::Bytes py_path( asUtf8Bytes( url_or_path_list[ i ] ) );
        std::string path( py_path.as_std_string() );
        bool is_url = is_svn_url( path );

        revisionKindCompatibleCheck( is_url, peg_revision, name_peg_revision, name_url_or_path );
        revisionKindCompatibleCheck( is_url, revision_start, name_revision_start, name_url_or_path );
        revisionKindCompatibleCheck( is_url, revision_end, name_revision_end, name_url_or_path );
    }

    apr_array_header_t *targets = targetsFromStringOrList( url_or_path_list, pool );

    Py::List log_list;

    try
    {
        checkThreadPermission();

        PythonAllowThreads permission( m_context );

        Log4Baton baton( &permission, pool, log_list );
        baton.m_wrapper_log = &m_wrapper_log;
        baton.m_wrapper_log_changed_path = &m_wrapper_log_changed_path;

#if defined( PYSVN_HAS_CLIENT_LOG5 )
        apr_array_header_t *revision_ranges = apr_array_make( pool, 0, sizeof(svn_opt_revision_range_t *) );
        svn_opt_revision_range_t *range = reinterpret_cast<svn_opt_revision_range_t *>( apr_palloc( pool, sizeof(*range) ) );

        range->start = revision_start;
        range->end = revision_end;

        APR_ARRAY_PUSH( revision_ranges, svn_opt_revision_range_t * ) = range;

        svn_error_t *error = svn_client_log5
            (
            targets,
            &peg_revision,
            revision_ranges,
            limit,
            discover_changed_paths,
            strict_node_history,
            include_merged_revisions,
            revprops,
            baton.callback(),
            baton.baton(),
            m_context,
            pool
            );
#else
        svn_error_t *error = svn_client_log4
            (
            targets,
            &peg_revision,
            &revision_start,
            &revision_end,
            limit,
            discover_changed_paths,
            strict_node_history,
            include_merged_revisions,
            revprops,
            baton.callback(),
            baton.baton(),
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

    return log_list;
}
#else
class LogChangePathInfo
{
public:
    LogChangePathInfo( const char *path, svn_log_changed_path_t *info )
    : m_path( path )
    , m_action( info->action )
    , m_copy_from_path( info->copyfrom_path != NULL ? info->copyfrom_path : "" )
    , m_copy_from_revision( info->copyfrom_rev )
    {
    }

    LogChangePathInfo( const LogChangePathInfo &other )
    : m_path( other.m_path )
    , m_action( other.m_action )
    , m_copy_from_path( other.m_copy_from_path )
    , m_copy_from_revision( other.m_copy_from_revision )
    {
    }

    std::string     m_path;
    char            m_action;
    std::string     m_copy_from_path;
    svn_revnum_t    m_copy_from_revision;
};

class LogEntryInfo
{
public:
    LogEntryInfo
        (
        svn_revnum_t revision,
        const char *author,
        const char *date,
        const char *message
        )
    : m_revision( revision )
    , m_author( author )
    , m_date( date )
    , m_message( message )
    , m_changed_paths()
    {
    }
    
    ~LogEntryInfo()
    {
    }

    LogEntryInfo( const LogEntryInfo &other )
    : m_revision( other.m_revision )
    , m_author( other.m_author )
    , m_date( other.m_date )
    , m_message( other.m_message )
    , m_changed_paths( other.m_changed_paths )
    {
    }

    svn_revnum_t m_revision;
    std::string m_author;
    std::string m_date;
    std::string m_message;
    std::list<LogChangePathInfo> m_changed_paths;
};

static svn_error_t *logReceiver
    (
    void *baton,
    apr_hash_t *changedPaths,
    svn_revnum_t rev,
    const char *author,
    const char *date,
    const char *msg,
    apr_pool_t *pool
    )
{
    if( rev == 0 )
    {
        // skip this blank entry
        // as the svn log command does
        return NULL;
    }

    std::list<LogEntryInfo> *entries = (std::list<LogEntryInfo> *)baton;

    if( author == NULL )
        author = "";
    if( date == NULL )
        date = "";
    if( msg == NULL )
        msg = "";

    entries->push_back( LogEntryInfo( rev, author, date, msg ) );

    if( changedPaths != NULL )
    {
        LogEntryInfo &entry = entries->back();

        for( apr_hash_index_t *hi = apr_hash_first( pool, changedPaths );
                hi != NULL;
                    hi = apr_hash_next( hi ) )
        {
            char *path = NULL;
            void *val = NULL;
            apr_hash_this( hi, (const void **) &path, NULL, &val );

            svn_log_changed_path_t *log_item = reinterpret_cast<svn_log_changed_path_t *> (val);

            entry.m_changed_paths.push_back( LogChangePathInfo( path, log_item ) );
        }
    }

    return NULL;
}

// PYSVN_HAS_CLIENT_LOG, PYSVN_HAS_CLIENT_LOG2, PYSVN_HAS_CLIENT_LOG3 version
Py::Object pysvn_client::cmd_log( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_url_or_path },
    { false, name_revision_start },
    { false, name_revision_end },
    { false, name_discover_changed_paths },
    { false, name_strict_node_history },
#if defined( PYSVN_HAS_CLIENT_LOG2 ) || defined( PYSVN_HAS_CLIENT_LOG3 )
    { false, name_limit },
#endif
#if defined( PYSVN_HAS_CLIENT_LOG3 )
    { false, name_peg_revision },
#endif
#if defined( PYSVN_HAS_CLIENT_LOG4 )
    { false, name_include_merged_revisions },
    { false, name_revprops },
#endif
    { false, NULL }
    };
    FunctionArguments args( "log", args_desc, a_args, a_kws );
    args.check();

    svn_opt_revision_t revision_start = args.getRevision( name_revision_start, svn_opt_revision_head );
    svn_opt_revision_t revision_end = args.getRevision( name_revision_end, svn_opt_revision_number );
    bool discover_changed_paths = args.getBoolean( name_discover_changed_paths, false );
    bool strict_node_history = args.getBoolean( name_strict_node_history, true );
    int limit = args.getInteger( name_limit, 0 );
#if defined( PYSVN_HAS_CLIENT_LOG3 )
    svn_opt_revision_t peg_revision = args.getRevision( name_peg_revision, svn_opt_revision_unspecified );
#endif
#if defined( PYSVN_HAS_CLIENT_LOG4 )
    svn_boolean_t include_merged_revisions = args.getBoolean( name_include_merged_revisions, false );
    apr_array_header_t *revprops = NULL;
    Py::Object py_revprop = args.getArg( name_revprops );
    if( py_revprop is not None )
    {
        revprops = arrayOfStringsFromListOfStrings( py_revprop. pool );
    }
#endif

    Py::Object url_or_path_obj = args.getArg( name_url_or_path );
    Py::List url_or_path_list;
    if( url_or_path_obj.isList() )
    {
        url_or_path_list = url_or_path_obj;
    }
    else
    {
        Py::List py_list;
        py_list.append( url_or_path_obj );
        url_or_path_list = py_list;
    }

    for( size_t i=0; i<url_or_path_list.size(); i++ )
    {
        Py::Bytes py_path( asUtf8Bytes( url_or_path_list[ i ] ) );
        std::string path( py_path.as_std_string() );
        bool is_url = is_svn_url( path );

        // std::cout << "peg_revision "    << peg_revision.kind    << " " << peg_revision.value.number     << std::endl;
        // std::cout << "revision_start "  << revision_start.kind  << " " << revision_start.value.number   << std::endl;
        // std::cout << "revision_end "    << revision_end.kind    << " " << revision_end.value.number     << std::endl;

#if defined( PYSVN_HAS_CLIENT_LOG3 )
        revisionKindCompatibleCheck( is_url, peg_revision, name_peg_revision, name_url_or_path );
#endif
        revisionKindCompatibleCheck( is_url, revision_start, name_revision_start, name_url_or_path );
        revisionKindCompatibleCheck( is_url, revision_end, name_revision_end, name_url_or_path );
    }

    SvnPool pool( m_context );

    apr_array_header_t *targets = targetsFromStringOrList( url_or_path_list, pool );

#if defined( PYSVN_HAS_CLIENT_LOG4 )
    Log4Baton baton( permission, pool );
#else
    std::list<LogEntryInfo> all_entries;
#endif

    try
    {
        checkThreadPermission();

        PythonAllowThreads permission( m_context );

#if defined( PYSVN_HAS_CLIENT_LOG4 )
        svn_error_t *error = svn_client_log4
            (
            targets,
            &peg_revision,
            &revision_start,
            &revision_end,
            limit,
            discover_changed_paths,
            strict_node_history,
            include_merged_revisions,
            revprops,
            logReceiver,
            &all_entries,
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_LOG3 )
        svn_error_t *error = svn_client_log3
            (
            targets,
            &peg_revision,
            &revision_start,
            &revision_end,
            limit,
            discover_changed_paths,
            strict_node_history,
            logReceiver,
            &all_entries,
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_LOG2 )
        svn_error_t *error = svn_client_log2
            (
            targets,
            &revision_start,
            &revision_end,
            limit,
            discover_changed_paths,
            strict_node_history,
            logReceiver,
            &all_entries,
            m_context,
            pool
            );
#else
        svn_error_t *error = svn_client_log
            (
            targets,
            &revision_start,
            &revision_end,
            discover_changed_paths,
            strict_node_history,
            logReceiver,
            &all_entries,
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

    apr_time_t now = apr_time_now();

    // convert the entries into python objects
    Py::List entries_list;
    std::list<LogEntryInfo>::const_iterator entry_it = all_entries.begin();
    while( entry_it != all_entries.end() )
    {
        const LogEntryInfo &entry = *entry_it;
        ++entry_it;

        Py::Dict entry_dict;
        entry_dict[name_author] = Py::String( entry.m_author, name_utf8 );
        entry_dict[name_date] = toObject( convertStringToTime( entry.m_date, now, pool ) );
        entry_dict[name_message] = Py::String( entry.m_message, name_utf8 );
        entry_dict[name_revision] = Py::asObject( new pysvn_revision( svn_opt_revision_number, 0, entry.m_revision ) );

        Py::List changed_paths_list;
        std::list<LogChangePathInfo>::const_iterator changed_paths_it = entry.m_changed_paths.begin();
        while( changed_paths_it != entry.m_changed_paths.end() )
        {
            const LogChangePathInfo &change_entry = *changed_paths_it;
            ++changed_paths_it;

            Py::Dict changed_entry_dict;
            changed_entry_dict[name_path] = Py::String( change_entry.m_path, name_utf8 );
            changed_entry_dict[name_action] = Py::String( &change_entry.m_action, 1 );
            changed_entry_dict[name_copyfrom_path] = utf8_string_or_none( change_entry.m_copy_from_path );

            if( SVN_IS_VALID_REVNUM( change_entry.m_copy_from_revision ) )
                changed_entry_dict[name_copyfrom_revision] = Py::asObject( new pysvn_revision( svn_opt_revision_number, 0, change_entry.m_copy_from_revision ) );
            else
                changed_entry_dict[name_copyfrom_revision] = Py::None();

            changed_paths_list.append( m_wrapper_log_changed_path.wrapDict( changed_entry_dict ) );
        }

        entry_dict[name_changed_paths] = changed_paths_list;

        entries_list.append( m_wrapper_log.wrapDict( entry_dict ) );
    }

    return entries_list;
}
#endif

#if defined( PYSVN_HAS_CLIENT_STATUS5 )
static svn_error_t *status5EntriesFunc
    (
    void *baton,
    const char *path,
    const svn_client_status_t *status,
    apr_pool_t *scratch_pool
    );

class Status2EntriesBaton
{
public:
    Status2EntriesBaton( SvnPool &pool )
    : m_pool( pool )
    , m_hash( apr_hash_make( pool ) )
    {}

    ~Status2EntriesBaton()
    {}

    svn_client_status_func_t callback() { return &status5EntriesFunc; }

    void *baton() { return static_cast< void * >( this ); }
    static Status2EntriesBaton *castBaton( void *baton_ ) { return static_cast<Status2EntriesBaton *>( baton_ ); }

    SvnPool     &m_pool;
    apr_hash_t  *m_hash;
};

static svn_error_t *status5EntriesFunc
    (
    void *baton,
    const char *path,
    const svn_client_status_t *status,
    apr_pool_t *scratch_pool
    )
{
    Status2EntriesBaton *seb = Status2EntriesBaton::castBaton( baton );

    path = apr_pstrdup( seb->m_pool, path );
    svn_client_status_t *stat = svn_client_status_dup( status, seb->m_pool );
    apr_hash_set( seb->m_hash, path, APR_HASH_KEY_STRING, stat );
    return SVN_NO_ERROR;
}

Py::Object pysvn_client::cmd_status2( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_path },
    { false, name_recurse },
    { false, name_get_all },
    { false, name_update },
    { false, name_ignore },
    { false, name_ignore_externals },
    { false, name_depth },
    { false, name_changelists },
    { false, name_depth_as_sticky },
#if defined( PYSVN_HAS_CLIENT_STATUS6 )
    { false, name_check_out_of_date },
    { false, name_check_working_copy },
#endif
    { false, NULL }
    };
    FunctionArguments args( "status2", args_desc, a_args, a_kws );
    args.check();

    Py::String path( args.getUtf8String( name_path ) );

    SvnPool pool( m_context );

    apr_array_header_t *changelists = NULL;

    if( args.hasArg( name_changelists ) )
    {
        changelists = arrayOfStringsFromListOfStrings( args.getArg( name_changelists ), pool );
    }

    svn_depth_t depth = args.getDepth( name_depth, name_recurse, svn_depth_infinity, svn_depth_infinity, svn_depth_immediates );
    bool get_all = args.getBoolean( name_get_all, true );
    bool update = args.getBoolean( name_update, false );
    bool ignore = args.getBoolean( name_ignore, false );
    bool ignore_externals = args.getBoolean( name_ignore_externals, false );
    bool depth_as_sticky = args.getBoolean( name_depth_as_sticky, true );

#if defined( PYSVN_HAS_CLIENT_STATUS6 )
    bool  check_out_of_date = args.getBoolean( name_check_out_of_date, update );
    bool  check_working_copy = args.getBoolean( name_check_working_copy, true );
#endif

    Status2EntriesBaton baton( pool );

    Py::List entries_list;
    try
    {
        std::string norm_path( svnNormalisedIfPath( path, pool ) );

        checkThreadPermission();

        PythonAllowThreads permission( m_context );

        svn_revnum_t revnum;
        svn_opt_revision_t rev = { svn_opt_revision_head, {0} };


        const char *abspath_or_url = NULL;
        svn_error_t *error = svn_dirent_get_absolute( &abspath_or_url, norm_path.c_str(), pool );

#if defined( PYSVN_HAS_CLIENT_STATUS6 )
        if( error == NULL )
        {
            error = svn_client_status6
                (
                &revnum,            // revnum
                m_context,
                abspath_or_url,     // path
                &rev,
                depth,
                get_all,
                check_out_of_date,
                check_working_copy,
                !ignore,
                ignore_externals,
                depth_as_sticky,
                changelists,
                baton.callback(),
                baton.baton(),
                pool
                );
        }
#else
        if( error == NULL )
        {
            error = svn_client_status5
                (
                &revnum,            // revnum
                m_context,
                abspath_or_url,     // path
                &rev,
                depth,
                get_all,
                update,
                !ignore,
                ignore_externals,
                depth_as_sticky,
                changelists,
                baton.callback(),
                baton.baton(),
                pool
                );
        }
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

    // Loop over array, returning each name/status-structure

    for( apr_hash_index_t *hi = apr_hash_first( pool, baton.m_hash );
            hi != NULL;
                hi = apr_hash_next( hi ) )
    {
        const void *key;
        void *val;
        apr_hash_this( hi, &key, NULL, &val );

        svn_client_status_t *status = static_cast<svn_client_status_t *>( val );

        entries_list.append( toObject(
                Py::String( osNormalisedPath( (const char *)key, pool ), "UTF-8" ),
                *status,
                pool,
                m_wrapper_status2,
                m_wrapper_lock ) );
    }

    entries_list.sort();

    return entries_list;
}
#endif

#if defined( PYSVN_HAS_CLIENT_STATUS4 )
static svn_error_t *status4EntriesFunc
    (
    void *baton,
    const char *path,
    svn_wc_status2_t *status,
    apr_pool_t *pool
    );
#elif defined( PYSVN_HAS_CLIENT_STATUS2 )
static void status2EntriesFunc
    (
    void *baton,
    const char *path,
    svn_wc_status2_t *status
    );
#else
static void status1EntriesFunc
    (
    void *baton,
    const char *path,
    svn_wc_status_t *status
    );
#endif

class StatusEntriesBaton
{
public:
    StatusEntriesBaton( SvnPool &pool )
    : m_pool( pool )
    , m_hash( apr_hash_make( pool ) )
    {}

    ~StatusEntriesBaton()
    {}

#if defined( PYSVN_HAS_CLIENT_STATUS4 )
    svn_wc_status_func3_t callback() { return &status4EntriesFunc; }
#elif defined( PYSVN_HAS_CLIENT_STATUS2 )
    svn_wc_status_func2_t callback() { return &status2EntriesFunc; }
#else
    svn_wc_status_func_t callback() { return &status1EntriesFunc; }
#endif
    void *baton() { return static_cast< void * >( this ); }
    static StatusEntriesBaton *castBaton( void *baton_ ) { return static_cast<StatusEntriesBaton *>( baton_ ); }

    SvnPool     &m_pool;
    apr_hash_t  *m_hash;
};

#if defined( PYSVN_HAS_CLIENT_STATUS4 )
static svn_error_t *status4EntriesFunc
    (
    void *baton,
    const char *path,
    svn_wc_status2_t *status,
    apr_pool_t *pool
    )
{
    StatusEntriesBaton *seb = StatusEntriesBaton::castBaton( baton );

    path = apr_pstrdup( seb->m_pool, path );
    svn_wc_status2_t *stat = svn_wc_dup_status2( status, seb->m_pool ); // ignore deprecation needed for Client().status()
    apr_hash_set( seb->m_hash, path, APR_HASH_KEY_STRING, stat );
    return SVN_NO_ERROR;
}
#elif defined( PYSVN_HAS_CLIENT_STATUS2 )
static void status2EntriesFunc
    (
    void *baton,
    const char *path,
    svn_wc_status2_t *status
    )
{
    StatusEntriesBaton *seb = StatusEntriesBaton::castBaton( baton );

    path = apr_pstrdup( seb->m_pool, path );
    svn_wc_status2_t *stat = svn_wc_dup_status2( status, seb->m_pool );
    apr_hash_set( seb->m_hash, path, APR_HASH_KEY_STRING, stat );
}
#else
static void status1EntriesFunc
    (
    void *baton,
    const char *path,
    svn_wc_status_t *status
    )
{
    StatusEntriesBaton *seb = StatusEntriesBaton::castBaton( baton );

    path = apr_pstrdup( seb->m_pool, path );
    svn_wc_status2_t *stat = svn_wc_dup_status( status, seb->m_pool );
    apr_hash_set( seb->m_hash, path, APR_HASH_KEY_STRING, stat );
}
#endif

Py::Object pysvn_client::cmd_status( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_path },
    { false, name_recurse },
    { false, name_get_all },
    { false, name_update },
    { false, name_ignore },
#if defined( PYSVN_HAS_CLIENT_STATUS2 )
    { false, name_ignore_externals },
#endif
#if defined( PYSVN_HAS_CLIENT_STATUS3 )
    { false, name_depth },
    { false, name_changelists },
#endif
    { false, NULL }
    };
    FunctionArguments args( "status", args_desc, a_args, a_kws );
    args.check();

    Py::String path( args.getUtf8String( name_path ) );

    SvnPool pool( m_context );

#if defined( PYSVN_HAS_CLIENT_STATUS3 )
    apr_array_header_t *changelists = NULL;

    if( args.hasArg( name_changelists ) )
    {
        changelists = arrayOfStringsFromListOfStrings( args.getArg( name_changelists ), pool );
    }

    svn_depth_t depth = args.getDepth( name_depth, name_recurse, svn_depth_infinity, svn_depth_infinity, svn_depth_immediates );
#else
    bool recurse = args.getBoolean( name_recurse, true );
#endif
    bool get_all = args.getBoolean( name_get_all, true );
    bool update = args.getBoolean( name_update, false );
    bool ignore = args.getBoolean( name_ignore, false );
#if defined( PYSVN_HAS_CLIENT_STATUS2 )
    bool ignore_externals = args.getBoolean( name_ignore_externals, false );
#endif

    StatusEntriesBaton baton( pool );

    Py::List entries_list;
    try
    {
        std::string norm_path( svnNormalisedIfPath( path, pool ) );

        checkThreadPermission();

        PythonAllowThreads permission( m_context );

        svn_revnum_t revnum;
        svn_opt_revision_t rev = { svn_opt_revision_head, {0} };

#if defined( PYSVN_HAS_CLIENT_STATUS4 )
        svn_error_t *error = svn_client_status4 // ignore deprecation warning - support Client().status()
            (
            &revnum,            // revnum
            norm_path.c_str(),  // path
            &rev,
            baton.callback(),
            baton.baton(),
            depth,
            get_all,
            update,
            !ignore,
            ignore_externals,
            changelists,
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_STATUS3 )
        svn_error_t *error = svn_client_status3
            (
            &revnum,            // revnum
            norm_path.c_str(),  // path
            &rev,
            baton.callback(),
            baton.baton(),
            depth,
            get_all,
            update,
            !ignore,
            ignore_externals,
            changelists,
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_STATUS2 )
        svn_error_t *error = svn_client_status2
            (
            &revnum,            // revnum
            norm_path.c_str(),  // path
            &rev,
            baton.callback(),
            baton.baton(),
            recurse,
            get_all,
            update,
            !ignore,
            ignore_externals,
            m_context,
            pool
            );
#else
        svn_error_t *error = svn_client_status
            (
            &revnum,            // revnum
            norm_path.c_str(),  // path
            &rev,
            baton.callback(),
            baton.baton(),
            recurse,
            get_all,
            update,
            !ignore,
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

    // Loop over array, returning each name/status-structure

    for( apr_hash_index_t *hi = apr_hash_first( pool, baton.m_hash );
            hi != NULL;
                hi = apr_hash_next( hi ) )
    {
        const void *key;
        void *val;
        apr_hash_this( hi, &key, NULL, &val );

        pysvn_wc_status_t *status = static_cast<pysvn_wc_status_t *>( val );
        entries_list.append( toObject(
                Py::String( osNormalisedPath( (const char *)key, pool ), "UTF-8" ),
                *status,
                pool,
                m_wrapper_status,
                m_wrapper_entry,
                m_wrapper_lock ) );
    }

    entries_list.sort();

    return entries_list;
}
