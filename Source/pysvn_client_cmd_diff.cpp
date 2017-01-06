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
//  pysvn_client_cmd_diff.cpp
//
#if defined( _MSC_VER )
// disable warning C4786: symbol greater than 255 character,
// nessesary to ignore as <map> causes lots of warning
#pragma warning(disable: 4786)
#endif

#include "pysvn.hpp"
#include "pysvn_static_strings.hpp"

class PySvnAprFile
{
public:
    PySvnAprFile( SvnPool &pool )
    : m_pool( pool )
    , m_apr_file( NULL )
    , m_filename( NULL )
    {
    }

    ~PySvnAprFile()
    {
        close();
        if( m_filename )
        {
            svn_error_clear( svn_io_remove_file2( m_filename, TRUE, m_pool ) );
        }
    }

    void open_unique_file( const std::string &tmp_dir )
    {
        svn_error_t *error = svn_io_open_unique_file3
            (
            &m_apr_file,
            &m_filename,
            tmp_dir.c_str(),
            svn_io_file_del_none,
            m_pool,
            m_pool
            );
        if( error != NULL )
        {
            throw SvnException( error );
        }
    }

    void readIntoStringBuf( svn_stringbuf_t **stringbuf )
    {
        close();

        apr_status_t status = apr_file_open( &m_apr_file, m_filename, APR_READ, APR_OS_DEFAULT, m_pool );
        if( status )
        {
            std::string msg( "opening file " ); msg += m_filename;
            throw SvnException( svn_error_create( status, NULL, msg.c_str() ) );
        }

        svn_error_t *error = svn_stringbuf_from_aprfile( stringbuf, m_apr_file, m_pool );
        if( error != NULL )
        {
            throw SvnException( error );
        }
    }

    void close()
    {
        // only close if we have an open file
        if( m_apr_file == NULL )
        {
            return;
        }
        apr_file_t *apr_file = m_apr_file;

        // prevent closing the file twice
        m_apr_file = NULL;

        apr_status_t status = apr_file_close( apr_file );
        if( status )
        {
            std::string msg( "closing file " ); msg += m_filename;
            throw SvnException( svn_error_create( status, NULL, msg.c_str() ) );
        }
    }

    apr_file_t *file()
    {
        return m_apr_file;
    }

private:
    SvnPool &m_pool;
    apr_file_t *m_apr_file;
    const char *m_filename;
};

class PySvnSvnStream
{
public:
    PySvnSvnStream( SvnPool &pool )
    : m_pool( pool )
    , m_svn_stream( NULL )
    , m_filename( NULL )
    {
    }

    ~PySvnSvnStream()
    {
        close();
        if( m_filename )
        {
            svn_error_clear( svn_io_remove_file2( m_filename, TRUE, m_pool ) );
        }
    }

    void open_unique_file( const std::string &tmp_dir )
    {
        svn_error_t *error = svn_stream_open_unique
            (
            &m_svn_stream,
            &m_filename,
            tmp_dir.c_str(),
            svn_io_file_del_none,
            m_pool,
            m_pool
            );
        if( error != NULL )
        {
            throw SvnException( error );
        }
    }
    
    void readIntoStringBuf( svn_stringbuf_t **stringbuf )
    {
        close();

        svn_error_t *error = svn_stringbuf_from_file2( stringbuf, m_filename, m_pool );
        if( error != NULL )
        {
            throw SvnException( error );
        }
    }

    void close()
    {
        // only close if we have an open file
        if( m_svn_stream == NULL )
        {
            return;
        }
        svn_stream_t *svn_stream = m_svn_stream;

        // prevent closing the file twice
        m_svn_stream = NULL;

        svn_error_t *error = svn_stream_close( svn_stream );
        if( error )
        {
            throw SvnException( error );
        }
    }

    svn_stream_t *stream()
    {
        return m_svn_stream;
    }

private:
    SvnPool         &m_pool;
    svn_stream_t    *m_svn_stream;
    const char      *m_filename;
};

Py::Object pysvn_client::cmd_diff( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_tmp_path },
    { true,  name_url_or_path },
    { false, name_revision1 },
    { false, name_url_or_path2 },
    { false, name_revision2 },
    { false, name_recurse },
    { false, name_ignore_ancestry },
    { false, name_diff_deleted },
#if defined( PYSVN_HAS_CLIENT_DIFF2 )
    { false, name_ignore_content_type },
#endif
#if defined( PYSVN_HAS_CLIENT_DIFF3 )
    { false, name_header_encoding },
    { false, name_diff_options },
#endif
#if defined( PYSVN_HAS_CLIENT_DIFF4 )
    { false, name_depth },
    { false, name_relative_to_dir },
    { false, name_changelists },
#endif
#if defined( PYSVN_HAS_CLIENT_DIFF5 )
    { false, name_show_copies_as_adds },
    { false, name_use_git_diff_format },
#endif
#if defined( PYSVN_HAS_CLIENT_DIFF6 )
    { false, name_diff_added },
    { false, name_ignore_properties },
    { false, name_properties_only },
#endif
    { false, NULL }
    };
    FunctionArguments args( "diff", args_desc, a_args, a_kws );
    args.check();

    std::string tmp_path( args.getUtf8String( name_tmp_path ) );
    std::string path1( args.getUtf8String( name_url_or_path ) );
    svn_opt_revision_t revision1 = args.getRevision( name_revision1, svn_opt_revision_base );
    std::string path2( args.getUtf8String( name_url_or_path2, path1 ) );
    svn_opt_revision_t revision2 = args.getRevision( name_revision2, svn_opt_revision_working );
#if defined( PYSVN_HAS_CLIENT_DIFF4 )
    svn_depth_t depth = args.getDepth( name_depth, name_recurse, svn_depth_infinity, svn_depth_infinity, svn_depth_files );
#else
    bool recurse = args.getBoolean( name_recurse, true );
#endif
    bool ignore_ancestry = args.getBoolean( name_ignore_ancestry, true );
    bool diff_deleted = args.getBoolean( name_diff_deleted, true );
#if defined( PYSVN_HAS_CLIENT_DIFF2 )
    bool ignore_content_type = args.getBoolean( name_ignore_content_type, false );
#endif

    SvnPool pool( m_context );

#if defined( PYSVN_HAS_CLIENT_DIFF3 )
    std::string header_encoding( args.getUtf8String( name_header_encoding, empty_string ) );
    const char *header_encoding_ptr = APR_LOCALE_CHARSET;
    if( !header_encoding.empty() )
        header_encoding_ptr = header_encoding.c_str();

    apr_array_header_t *options = NULL;
    if( args.hasArg( name_diff_options ) )
    {
        options = arrayOfStringsFromListOfStrings( args.getArg( name_diff_options ), pool );
    }
    else
    {
        options = apr_array_make( pool, 0, sizeof( const char * ) );
    }
#else
    apr_array_header_t *options = apr_array_make( pool, 0, sizeof( const char * ) );
#endif

#if defined( PYSVN_HAS_CLIENT_DIFF4 )
    std::string std_relative_to_dir;
    const char *relative_to_dir = NULL;
    if( args.hasArg( name_relative_to_dir ) )
    {
        std_relative_to_dir = args.getUtf8String( name_relative_to_dir );
        relative_to_dir = std_relative_to_dir.c_str();
    }

    apr_array_header_t *changelists = NULL;

    if( args.hasArg( name_changelists ) )
    {
        changelists = arrayOfStringsFromListOfStrings( args.getArg( name_changelists ), pool );
    }
#endif

#if defined( PYSVN_HAS_CLIENT_DIFF5 )
    bool show_copies_as_adds = args.getBoolean( name_show_copies_as_adds, false );
    bool use_git_diff_format = args.getBoolean( name_use_git_diff_format, false );
#endif
#if defined( PYSVN_HAS_CLIENT_DIFF6 )
    bool diff_added = args.getBoolean( name_diff_added, true );
    bool ignore_properties = args.getBoolean( name_ignore_properties, false );
    bool properties_only = args.getBoolean( name_properties_only, false );
#endif

    svn_stringbuf_t *stringbuf = NULL;

    try
    {
        std::string norm_tmp_path( svnNormalisedIfPath( tmp_path, pool ) );
        std::string norm_path1( svnNormalisedIfPath( path1, pool ) );
        std::string norm_path2( svnNormalisedIfPath( path2, pool ) );

        checkThreadPermission();

#if defined( PYSVN_HAS_CLIENT_DIFF6 )
        PySvnSvnStream output_stream( pool );
        PySvnSvnStream error_stream( pool );

        output_stream.open_unique_file( norm_tmp_path );
        error_stream.open_unique_file( norm_tmp_path );
#else
        PySvnAprFile output_file( pool );
        PySvnAprFile error_file( pool );

        output_file.open_unique_file( norm_tmp_path );
        error_file.open_unique_file( norm_tmp_path );
#endif

        PythonAllowThreads permission( m_context );

#if defined( PYSVN_HAS_CLIENT_DIFF6 )
        svn_error_t *error = svn_client_diff6
            (
            options,
            norm_path1.c_str(), &revision1,
            norm_path2.c_str(), &revision2,
            relative_to_dir,
            depth,
            ignore_ancestry,
            !diff_added,
            !diff_deleted,
            show_copies_as_adds,
            ignore_content_type,
            ignore_properties,
            properties_only,
            use_git_diff_format,
            header_encoding_ptr,
            output_stream.stream(),
            error_stream.stream(),
            changelists,
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_DIFF5 )
        svn_error_t *error = svn_client_diff5
            (
            options,
            norm_path1.c_str(), &revision1,
            norm_path2.c_str(), &revision2,
            relative_to_dir,
            depth,
            ignore_ancestry,
            !diff_deleted,
            show_copies_as_adds,
            ignore_content_type,
            use_git_diff_format,
            header_encoding_ptr,
            output_file.file(),
            error_file.file(),
            changelists,
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_DIFF4 )
        svn_error_t *error = svn_client_diff4
            (
            options,
            norm_path1.c_str(), &revision1,
            norm_path2.c_str(), &revision2,
            relative_to_dir,
            depth,
            ignore_ancestry,
            !diff_deleted,
            ignore_content_type,
            header_encoding_ptr,
            output_file.file(),
            error_file.file(),
            changelists,
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_DIFF3 )
        svn_error_t *error = svn_client_diff3
            (
            options,
            norm_path1.c_str(), &revision1,
            norm_path2.c_str(), &revision2,
            recurse,
            ignore_ancestry,
            !diff_deleted,
            ignore_content_type,
            header_encoding_ptr,
            output_file.file(),
            error_file.file(),
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_DIFF2 )
        svn_error_t *error = svn_client_diff2
            (
            options,
            norm_path1.c_str(), &revision1,
            norm_path2.c_str(), &revision2,
            recurse,
            ignore_ancestry,
            !diff_deleted,
            ignore_content_type,
            output_file.file(),
            error_file.file(),
            m_context,
            pool
            );
#else
        svn_error_t *error = svn_client_diff
            (
            options,
            norm_path1.c_str(), &revision1,
            norm_path2.c_str(), &revision2,
            recurse,
            ignore_ancestry,
            !diff_deleted,
            output_file.file(),
            error_file.file(),
            m_context,
            pool
            );
#endif
        permission.allowThisThread();
        if( error != NULL )
            throw SvnException( error );

#if defined( PYSVN_HAS_CLIENT_DIFF6 )
        output_stream.readIntoStringBuf( &stringbuf );
#else
        output_file.readIntoStringBuf( &stringbuf );
#endif
    }
    catch( SvnException &e )
    {
        // use callback error over ClientException
        m_context.checkForError( m_module.client_error );

        throw_client_error( e );
    }

    // cannot convert to Unicode as we have no idea of the encoding of the bytes
    return Py::String( stringbuf->data, (int)stringbuf->len );
}

#if defined( PYSVN_HAS_CLIENT_DIFF_PEG )
Py::Object pysvn_client::cmd_diff_peg( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_tmp_path },
    { true,  name_url_or_path },
    { false, name_peg_revision },
    { false, name_revision_start },
    { false, name_revision_end },
    { false, name_recurse },
    { false, name_ignore_ancestry },
    { false, name_diff_deleted },
#if defined( PYSVN_HAS_CLIENT_DIFF_PEG2 )
    { false, name_ignore_content_type },
#endif
#if defined( PYSVN_HAS_CLIENT_DIFF_PEG3 )
    { false, name_header_encoding },
    { false, name_diff_options },
#endif
#if defined( PYSVN_HAS_CLIENT_DIFF_PEG4 )
    { false, name_depth },
    { false, name_relative_to_dir },
    { false, name_changelists },
#endif
#if defined( PYSVN_HAS_CLIENT_DIFF_PEG5 )
    { false, name_show_copies_as_adds },
    { false, name_use_git_diff_format },
#endif
#if defined( PYSVN_HAS_CLIENT_DIFF_PEG6 )
    { false, name_diff_added },
    { false, name_ignore_properties },
    { false, name_properties_only },
#endif
    { false, NULL }
    };
    FunctionArguments args( "diff_peg", args_desc, a_args, a_kws );
    args.check();

    std::string tmp_path( args.getUtf8String( name_tmp_path ) );
    std::string path( args.getUtf8String( name_url_or_path ) );
    svn_opt_revision_t revision_start = args.getRevision( name_revision_start, svn_opt_revision_base );
    svn_opt_revision_t revision_end = args.getRevision( name_revision_end, svn_opt_revision_working );
    svn_opt_revision_t peg_revision = args.getRevision( name_peg_revision, revision_end );

    SvnPool pool( m_context );

#if defined( PYSVN_HAS_CLIENT_DIFF_PEG4 )
    svn_depth_t depth = args.getDepth( name_depth, name_recurse, svn_depth_infinity, svn_depth_infinity, svn_depth_files );
    std::string std_relative_to_dir;
    const char *relative_to_dir = NULL;
    if( args.hasArg( name_relative_to_dir ) )
    {
        std_relative_to_dir = args.getBytes( name_relative_to_dir );
        relative_to_dir = std_relative_to_dir.c_str();
    }

    apr_array_header_t *changelists = NULL;

    if( args.hasArg( name_changelists ) )
    {
        changelists = arrayOfStringsFromListOfStrings( args.getArg( name_changelists ), pool );
    }
#else
    bool recurse = args.getBoolean( name_recurse, true );
#endif
    bool ignore_ancestry = args.getBoolean( name_ignore_ancestry, true );
    bool diff_deleted = args.getBoolean( name_diff_deleted, true );
#if defined( PYSVN_HAS_CLIENT_DIFF_PEG2 )
    bool ignore_content_type = args.getBoolean( name_ignore_content_type, false );
#endif

#if defined( PYSVN_HAS_CLIENT_DIFF_PEG3 )
    std::string header_encoding( args.getUtf8String( name_header_encoding, empty_string ) );
    const char *header_encoding_ptr = APR_LOCALE_CHARSET;
    if( !header_encoding.empty() )
        header_encoding_ptr = header_encoding.c_str();

    apr_array_header_t *options = NULL;
    if( args.hasArg( name_diff_options ) )
    {
        options = arrayOfStringsFromListOfStrings( args.getArg( name_diff_options ), pool );
    }
    else
    {
        options = apr_array_make( pool, 0, sizeof( const char * ) );
    }
#else
    apr_array_header_t *options = apr_array_make( pool, 0, sizeof( const char * ) );
#endif

#if defined( PYSVN_HAS_CLIENT_DIFF_PEG5 )
    bool show_copies_as_adds = args.getBoolean( name_show_copies_as_adds, false );
    bool use_git_diff_format = args.getBoolean( name_use_git_diff_format, false );
#endif
#if defined( PYSVN_HAS_CLIENT_DIFF_PEG6 )
    bool diff_added = args.getBoolean( name_diff_added, true );
    bool ignore_properties = args.getBoolean( name_ignore_properties, false );
    bool properties_only = args.getBoolean( name_properties_only, false );
#endif

    bool is_url = is_svn_url( path );
    revisionKindCompatibleCheck( is_url, peg_revision, name_peg_revision, name_url_or_path );
    revisionKindCompatibleCheck( is_url, revision_start, name_revision_start, name_url_or_path );
    revisionKindCompatibleCheck( is_url, revision_end, name_revision_end, name_url_or_path );

    svn_stringbuf_t *stringbuf = NULL;

    try
    {
        std::string norm_tmp_path( svnNormalisedIfPath( tmp_path, pool ) );
        std::string norm_path( svnNormalisedIfPath( path, pool ) );

        checkThreadPermission();

        PythonAllowThreads permission( m_context );

#if defined( PYSVN_HAS_CLIENT_DIFF6 )
        PySvnSvnStream output_stream( pool );
        PySvnSvnStream error_stream( pool );

        output_stream.open_unique_file( norm_tmp_path );
        error_stream.open_unique_file( norm_tmp_path );
#else
        PySvnAprFile output_file( pool );
        PySvnAprFile error_file( pool );

        output_file.open_unique_file( norm_tmp_path );
        error_file.open_unique_file( norm_tmp_path );
#endif

        // std::cout << "peg_revision "    << peg_revision.kind    << " " << peg_revision.value.number     << std::endl;
        // std::cout << "revision_start "  << revision_start.kind  << " " << revision_start.value.number   << std::endl;
        // std::cout << "revision_end "    << revision_end.kind    << " " << revision_end.value.number     << std::endl;

#if defined( PYSVN_HAS_CLIENT_DIFF_PEG6 )
        svn_error_t *error = svn_client_diff_peg6
            (
            options,
            norm_path.c_str(),
            &peg_revision,
            &revision_start,
            &revision_end,
            relative_to_dir,
            depth,
            ignore_ancestry,
            !diff_added,
            !diff_deleted,
            show_copies_as_adds,
            ignore_content_type,
            ignore_properties,
            properties_only,
            use_git_diff_format,
            header_encoding_ptr,
            output_stream.stream(),
            error_stream.stream(),
            changelists,
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_DIFF_PEG5 )
        svn_error_t *error = svn_client_diff_peg5
            (
            options,
            norm_path.c_str(),
            &peg_revision,
            &revision_start,
            &revision_end,
            relative_to_dir,
            depth,
            ignore_ancestry,
            !diff_deleted,
            show_copies_as_adds,
            ignore_content_type,
            use_git_diff_format,
            header_encoding_ptr,
            output_file.file(),
            error_file.file(),
            changelists,
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_DIFF_PEG4 )
        svn_error_t *error = svn_client_diff_peg4
            (
            options,
            norm_path.c_str(),
            &peg_revision,
            &revision_start,
            &revision_end,
            relative_to_dir,
            depth,
            ignore_ancestry,
            !diff_deleted,
            ignore_content_type,
            header_encoding_ptr,
            output_file.file(),
            error_file.file(),
            changelists,
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_DIFF_PEG3 )
        svn_error_t *error = svn_client_diff_peg3
            (
            options,
            norm_path.c_str(),
            &peg_revision,
            &revision_start,
            &revision_end,
            recurse,
            ignore_ancestry,
            !diff_deleted,
            ignore_content_type,
            header_encoding_ptr,
            output_file.file(),
            error_file.file(),
            m_context,
            pool
            );
#elif defined( PYSVN_HAS_CLIENT_DIFF_PEG2 )
        svn_error_t *error = svn_client_diff_peg2
            (
            options,
            norm_path.c_str(),
            &peg_revision,
            &revision_start,
            &revision_end,
            recurse,
            ignore_ancestry,
            !diff_deleted,
            ignore_content_type,
            output_file.file(),
            error_file.file(),
            m_context,
            pool
            );
#else
        svn_error_t *error = svn_client_diff_peg
            (
            options,
            norm_path.c_str(),
            &peg_revision,
            &revision_start,
            &revision_end,
            recurse,
            ignore_ancestry,
            !diff_deleted,
            output_file.file(),
            error_file.file(),
            m_context,
            pool
            );
#endif
        permission.allowThisThread();
        if( error != NULL )
            throw SvnException( error );

#if defined( PYSVN_HAS_CLIENT_DIFF6 )
        output_stream.readIntoStringBuf( &stringbuf );
#else
        output_file.readIntoStringBuf( &stringbuf );
#endif
    }
    catch( SvnException &e )
    {
        // use callback error over ClientException
        m_context.checkForError( m_module.client_error );

        throw_client_error( e );
    }

    // cannot convert to Unicode as we have no idea of the encoding of the bytes
    return Py::String( stringbuf->data, (int)stringbuf->len );
}
#endif

#if defined( PYSVN_HAS_CLIENT_DIFF_SUMMARIZE )
extern "C" svn_error_t *diff_summarize_c
    (
    const svn_client_diff_summarize_t *diff,
    void *baton_,
    apr_pool_t *pool
    );

class DiffSummarizeBaton
{
public:
    DiffSummarizeBaton( PythonAllowThreads *permission, Py::List &diff_list )
        : m_permission( permission )
        , m_diff_list( diff_list )
        {}
    ~DiffSummarizeBaton()
        {}

    svn_client_diff_summarize_func_t callback() { return &diff_summarize_c; }
    void *baton() { return static_cast< void * >( this ); }
    static DiffSummarizeBaton *castBaton( void *baton_ ) { return static_cast<DiffSummarizeBaton *>( baton_ ); }

    PythonAllowThreads  *m_permission;

    DictWrapper         *m_wrapper_diff_summary;
    Py::List            &m_diff_list;
};

extern "C" svn_error_t *diff_summarize_c
    (
    const svn_client_diff_summarize_t *diff,
    void *baton_,
    apr_pool_t *pool
    )
{
    DiffSummarizeBaton *baton = DiffSummarizeBaton::castBaton( baton_ );

    PythonDisallowThreads callback_permission( baton->m_permission );

    Py::Dict diff_dict;

    diff_dict[ *py_name_path ] = Py::String( diff->path, name_utf8 );
    diff_dict[ *py_name_summarize_kind ] = toEnumValue( diff->summarize_kind );
    diff_dict[ *py_name_prop_changed ] = Py::Int( diff->prop_changed != 0 );
    diff_dict[ *py_name_node_kind ] = toEnumValue( diff->node_kind );

    baton->m_diff_list.append( baton->m_wrapper_diff_summary->wrapDict( diff_dict ) );

    return SVN_NO_ERROR;
}

Py::Object pysvn_client::cmd_diff_summarize( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_url_or_path1 },
    { false, name_revision1 },
    { false, name_url_or_path2 },
    { false, name_revision2 },
    { false, name_recurse },
    { false, name_ignore_ancestry },
#if defined( PYSVN_HAS_CLIENT_DIFF_SUMMARIZE2 )
    { false, name_depth },
    { false, name_changelists },
#endif
    { false, NULL }
    };
    FunctionArguments args( "diff_summarize", args_desc, a_args, a_kws );
    args.check();

    std::string path1( args.getUtf8String( name_url_or_path1 ) );
    svn_opt_revision_t revision1 = args.getRevision( name_revision1, svn_opt_revision_base );
    std::string path2( args.getUtf8String( name_url_or_path2, path1 ) );
    svn_opt_revision_t revision2 = args.getRevision( name_revision2, svn_opt_revision_working );

    SvnPool pool( m_context );

#if defined( PYSVN_HAS_CLIENT_DIFF_SUMMARIZE2 )
    svn_depth_t depth = args.getDepth( name_depth, name_recurse, svn_depth_infinity, svn_depth_infinity, svn_depth_files );

    apr_array_header_t *changelists = NULL;

    if( args.hasArg( name_changelists ) )
    {
        changelists = arrayOfStringsFromListOfStrings( args.getArg( name_changelists ), pool );
    }
#else
    bool recurse = args.getBoolean( name_recurse, true );
#endif
    bool ignore_ancestry = args.getBoolean( name_ignore_ancestry, true );

    Py::List diff_list;

    try
    {
        std::string norm_path1( svnNormalisedIfPath( path1, pool ) );
        std::string norm_path2( svnNormalisedIfPath( path2, pool ) );

        checkThreadPermission();

        PythonAllowThreads permission( m_context );

        DiffSummarizeBaton diff_baton( &permission, diff_list );
        diff_baton.m_wrapper_diff_summary = &m_wrapper_diff_summary;

#if defined( PYSVN_HAS_CLIENT_DIFF_SUMMARIZE2 )
        svn_error_t *error = svn_client_diff_summarize2
            (
            norm_path1.c_str(),
            &revision1,
            norm_path2.c_str(),
            &revision2,
            depth,
            ignore_ancestry,
            changelists,
            diff_baton.callback(),
            diff_baton.baton(),
            m_context,
            pool
            );
#else
        svn_error_t *error = svn_client_diff_summarize
            (
            norm_path1.c_str(),
            &revision1,
            norm_path2.c_str(),
            &revision2,
            recurse,
            ignore_ancestry,
            diff_baton.callback(),
            diff_baton.baton(),
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

    // cannot convert to Unicode as we have no idea of the encoding of the bytes
    return diff_list;
}

Py::Object pysvn_client::cmd_diff_summarize_peg( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_url_or_path },
    { false, name_peg_revision },
    { false, name_revision_start },
    { false, name_revision_end },
    { false, name_recurse },
    { false, name_ignore_ancestry },
#if defined( PYSVN_HAS_CLIENT_DIFF_SUMMARIZE_PEG2 )
    { false, name_depth },
    { false, name_changelists },
#endif
    { false, NULL }
    };
    FunctionArguments args( "diff_summarize_peg", args_desc, a_args, a_kws );
    args.check();

    std::string path( args.getUtf8String( name_url_or_path ) );
    svn_opt_revision_t revision_start = args.getRevision( name_revision_start, svn_opt_revision_base );
    svn_opt_revision_t revision_end = args.getRevision( name_revision_end, svn_opt_revision_working );
    svn_opt_revision_t peg_revision = args.getRevision( name_peg_revision, revision_end );

    SvnPool pool( m_context );

#if defined( PYSVN_HAS_CLIENT_DIFF_SUMMARIZE_PEG2 )
    svn_depth_t depth = args.getDepth( name_depth, name_recurse, svn_depth_infinity, svn_depth_infinity, svn_depth_files );
    apr_array_header_t *changelists = NULL;

    if( args.hasArg( name_changelists ) )
    {
        changelists = arrayOfStringsFromListOfStrings( args.getArg( name_changelists ), pool );
    }
#else
    bool recurse = args.getBoolean( name_recurse, true );
#endif
    bool ignore_ancestry = args.getBoolean( name_ignore_ancestry, true );

    bool is_url = is_svn_url( path );
    revisionKindCompatibleCheck( is_url, peg_revision, name_peg_revision, name_url_or_path );
    revisionKindCompatibleCheck( is_url, revision_start, name_revision_start, name_url_or_path );
    revisionKindCompatibleCheck( is_url, revision_end, name_revision_end, name_url_or_path );

    Py::List diff_list;

    try
    {
        std::string norm_path( svnNormalisedIfPath( path, pool ) );

        checkThreadPermission();

        PythonAllowThreads permission( m_context );

        DiffSummarizeBaton diff_baton( &permission, diff_list );
        diff_baton.m_wrapper_diff_summary = &m_wrapper_diff_summary;

#if defined( PYSVN_HAS_CLIENT_DIFF_SUMMARIZE_PEG2 )
        svn_error_t *error = svn_client_diff_summarize_peg2
            (
            norm_path.c_str(),
            &peg_revision,
            &revision_start,
            &revision_end,
            depth,
            ignore_ancestry,
            changelists,
            diff_baton.callback(),
            diff_baton.baton(),
            m_context,
            pool
            );
#else
        svn_error_t *error = svn_client_diff_summarize_peg
            (
            norm_path.c_str(),
            &peg_revision,
            &revision_start,
            &revision_end,
            recurse,
            ignore_ancestry,
            diff_baton.callback(),
            diff_baton.baton(),
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

    // cannot convert to Unicode as we have no idea of the encoding of the bytes
    return diff_list;
}
#endif
