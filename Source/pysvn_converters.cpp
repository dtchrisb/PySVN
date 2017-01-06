//
// ====================================================================
// Copyright (c) 2003-2009 Barry A Scott.  All rights reserved.
//
// This software is licensed as described in the file LICENSE.txt,
// which you should have received as part of this distribution.
//
// ====================================================================
//

#if defined( _MSC_VER )
// disable warning C4786: symbol greater than 255 character,
// nessesary to ignore as <map> causes lots of warning
#pragma warning(disable: 4786)
#endif

#include "pysvn.hpp"
#include "svn_config.h"
#include "svn_time.h"

#include "pysvn_static_strings.hpp"

static const std::string str_URL( "URL" );
static const std::string str_action( "action" );
static const std::string str_author( "author" );
static const std::string str_base_abspath( "base_abspath" );
static const std::string str_changed_author( "changed_author" );
static const std::string str_changed_date( "changed_date" );
static const std::string str_changed_revision( "changed_revision" );
static const std::string str_changelist( "changelist" );
static const std::string str_checksum( "checksum" );
static const std::string str_comment( "comment" );
static const std::string str_commit_author( "commit_author" );
static const std::string str_commit_revision( "commit_revision" );
static const std::string str_commit_time( "commit_time" );
static const std::string str_conflict_new( "conflict_new" );
static const std::string str_conflict_old( "conflict_old" );
static const std::string str_conflict_work( "conflict_work" );
static const std::string str_conflicts( "conflicts" );
static const std::string str_copy_from_revision( "copy_from_revision" );
static const std::string str_copy_from_url( "copy_from_url" );
static const std::string str_copyfrom_rev( "copyfrom_rev" );
static const std::string str_copyfrom_url( "copyfrom_url" );
static const std::string str_creation_date( "creation_date" );
static const std::string str_date( "date" );
static const std::string str_depth( "depth" );
static const std::string str_entry( "entry" );
static const std::string str_expiration_date( "expiration_date" );
static const std::string str_filesize( "filesize" );
static const std::string str_is_absent( "is_absent" );
static const std::string str_is_binary( "is_binary" );
static const std::string str_is_conflicted( "is_copied" );
static const std::string str_is_copied( "is_copied" );
static const std::string str_is_dav_comment( "is_dav_comment" );
static const std::string str_is_deleted( "is_deleted" );
static const std::string str_is_file_external( "is_file_external" );
static const std::string str_is_locked( "is_locked" );
static const std::string str_is_switched( "is_switched" );
static const std::string str_is_versioned( "is_versioned" );
static const std::string str_kind( "kind" );
static const std::string str_last_changed_author( "last_changed_author" );
static const std::string str_last_changed_date( "last_changed_date" );
static const std::string str_last_changed_rev( "last_changed_rev" );
static const std::string str_local_abspath( "local_abspath" );
static const std::string str_lock( "lock" );
static const std::string str_lock_comment( "lock_comment" );
static const std::string str_lock_creation_date( "lock_creation_date" );
static const std::string str_lock_owner( "lock_owner" );
static const std::string str_lock_token( "lock_token" );
static const std::string str_merged_file( "merged_file" );
static const std::string str_mime_type( "mime_type" );
static const std::string str_moved_from_abspath( "moved_from_abspath" );
static const std::string str_moved_to_abspath( "moved_to_abspath" );
static const std::string str_my_abspath( "my_abspath" );
static const std::string str_name( "name" );
static const std::string str_node_kind( "node_kind" );
static const std::string str_node_status( "node_status" );
static const std::string str_ood_changed_author( "ood_changed_author" );
static const std::string str_ood_changed_date( "ood_changed_date" );
static const std::string str_ood_changed_rev( "ood_changed_rev" );
static const std::string str_ood_kind( "ood_kind" );
static const std::string str_operation( "operation" );
static const std::string str_owner( "owner" );
static const std::string str_path( "path" );
static const std::string str_path_in_repos( "path_in_repos" );
static const std::string str_peg_rev( "peg_rev" );
static const std::string str_post_commit_err( "post_commit_err" );
static const std::string str_prejfile( "prejfile" );
static const std::string str_prop_status( "prop_status" );
static const std::string str_prop_time( "prop_time" );
static const std::string str_properties_time( "properties_time" );
static const std::string str_property_name( "property_name" );
static const std::string str_property_reject_file( "property_reject_file" );
static const std::string str_reason( "reason" );
static const std::string str_recorded_size( "recorded_size" );
static const std::string str_recorded_time( "recorded_time" );
static const std::string str_repos( "repos" );
static const std::string str_repos_UUID( "repos_UUID" );
static const std::string str_repos_lock( "repos_lock" );
static const std::string str_repos_node_status( "repos_node_status" );
static const std::string str_repos_prop_status( "repos_prop_status" );
static const std::string str_repos_relpath( "repos_relpath" );
static const std::string str_repos_root_URL( "repos_root_URL" );
static const std::string str_repos_text_status( "repos_text_status" );
static const std::string str_repos_url( "repos_url" );
static const std::string str_rev( "rev" );
static const std::string str_revision( "revision" );
static const std::string str_schedule( "schedule" );
static const std::string str_size( "size" );
static const std::string str_src_left_version( "src_left_version" );
static const std::string str_src_right_version( "src_right_version" );
static const std::string str_text_status( "text_status" );
static const std::string str_text_time( "text_time" );
static const std::string str_their_abspath( "their_abspath" );
static const std::string str_token( "token" );
static const std::string str_url( "url" );
static const std::string str_uuid( "uuid" );
static const std::string str_wc_info( "wc_info" );
static const std::string str_wc_is_locked( "wc_is_locked" );
static const std::string str_wcroot_abspath( "wcroot_abspath" );
static const std::string str_working_size( "working_size" );

#include <iostream>

//------------------------------------------------------------
//
//  DictWrapper
//
//------------------------------------------------------------
DictWrapper::DictWrapper( Py::Dict result_wrappers, const std::string &wrapper_name )
: m_wrapper_name( wrapper_name )
, m_have_wrapper( false )
, m_wrapper()
{
    if( result_wrappers.hasKey( wrapper_name ) )
    {
        m_wrapper = result_wrappers[ wrapper_name ];
        m_have_wrapper = true;
    }
}

DictWrapper::~DictWrapper()
{
}

Py::Object DictWrapper::wrapDict( Py::Dict result ) const
{
    if( m_have_wrapper )
    {
        Py::Tuple args( 1 );
        args[0] = result;

        try
        {
            return m_wrapper.apply( args );
        }
        catch( Py::Exception &e )
        {
            std::cerr << "pysvn: unhandled exception calling " << m_wrapper_name << std::endl;
            PyErr_Print();
            e.clear();

            return Py::None();
        }
    }
    else
    {
        return result;
    }
}

//------------------------------------------------------------
//
//  Converters
//
//------------------------------------------------------------
Py::Object utf8_string_or_none( const char *str )
{
    if( str == NULL )
        return Py::None();
    else
        return Py::String( str, name_utf8 );
}

Py::Object utf8_string_or_none( const std::string &str )
{
    if( str.empty() )
        return Py::None();
    else
        return Py::String( str, name_utf8 );
}

Py::Object path_string_or_none( const char *str, SvnPool &pool )
{
    if( str == NULL )
        return Py::None();
    else
        return Py::String( osNormalisedPath( str, pool ), name_utf8 );
}

Py::Object path_string_or_none( const std::string &str, SvnPool &pool )
{
    if( str.empty() )
        return Py::None();
    else
        return Py::String( osNormalisedPath( str, pool ), name_utf8 );
}

apr_time_t convertStringToTime( const std::string &text, apr_time_t now, SvnPool &pool )
{
    svn_boolean_t matched = 0;
    apr_time_t result = 0;

    svn_error_t *error = svn_parse_date
        (
        &matched,
        &result,
        text.c_str(),
        now,
        pool
        );
    if( error != NULL || !matched )
    {
        return 0;
    }

    return result;
}

Py::Object toSvnRevNum( svn_revnum_t rev )
{
    return Py::asObject( new pysvn_revision( svn_opt_revision_number, 0, rev ) );
}

Py::Object toObject( apr_time_t t )
{
    return Py::Float( double( t )/1000000 );
}

#if defined( PYSVN_HAS_SVN_COMMIT_INFO_T )
Py::Object toObject( pysvn_commit_info_t *commit_info, int commit_style )
{
    if( commit_info == NULL )
        return Py::None();

    if( commit_style == 0 )
    {
        if( !SVN_IS_VALID_REVNUM( commit_info->revision ) )
            return Py::None();

        return toSvnRevNum( commit_info->revision );
    }
    else
    if( commit_style == 1 )
    {
        Py::Dict commit_info_dict;

        commit_info_dict[ str_date ] = utf8_string_or_none( commit_info->date );
        commit_info_dict[ str_author ] = utf8_string_or_none( commit_info->author );
        commit_info_dict[ str_post_commit_err ] = utf8_string_or_none( commit_info->post_commit_err );
        if( SVN_IS_VALID_REVNUM( commit_info->revision ) )
        {
            commit_info_dict[ str_revision ] = toSvnRevNum( commit_info->revision );
        }
        else
        {
            commit_info_dict[ str_revision ] = Py::None();
        }

        return commit_info_dict;
    }
    else
    {
        throw Py::RuntimeError( "commit_style value invalid" );
    }
}
#else
Py::Object toObject( pysvn_commit_info_t *commit_info )
{
    if( commit_info == NULL || !SVN_IS_VALID_REVNUM( commit_info->revision ) )
        return Py::None();

    return toSvnRevNum( commit_info->revision );
}
#endif

Py::Object toFilesize( svn_filesize_t filesize )
{
    if( filesize == SVN_INVALID_FILESIZE )
    {
        return Py::None();
    }
    else
    {
#ifdef HAVE_LONG_LONG
        return Py::LongLong( static_cast<PY_LONG_LONG>( filesize ) );
#else
        return Py::Int( static_cast<int>( filesize ) );
#endif
    }
}


#if defined( PYSVN_HAS_CLIENT_STATUS_T )
Py::Object toObject
    (
    Py::String path,
    svn_client_status_t &svn_status,
    SvnPool &pool,
    const DictWrapper &wrapper_status2,
    const DictWrapper &wrapper_lock
    )
{
    Py::Dict status;

    status[ str_path ] = path;
    status[ str_local_abspath ] = path_string_or_none( svn_status.local_abspath, pool );
    status[ str_kind ] = toEnumValue( svn_status.kind );
    status[ str_filesize ] = toFilesize( svn_status.filesize );
    status[ str_is_versioned] = Py::Boolean( svn_status.versioned );
    status[ str_is_conflicted ] = Py::Boolean( svn_status.conflicted );
    status[ str_node_status ] = toEnumValue( svn_status.node_status );
    status[ str_text_status ] = toEnumValue( svn_status.text_status );
    status[ str_prop_status ] = toEnumValue( svn_status.prop_status );
    status[ str_wc_is_locked ] = Py::Boolean( svn_status.wc_is_locked );
    status[ str_is_copied ] = Py::Boolean( svn_status.copied );
    status[ str_repos_root_URL ] = utf8_string_or_none( svn_status.repos_root_url );
    status[ str_repos_UUID ] = utf8_string_or_none( svn_status.repos_uuid );
    status[ str_repos_relpath ] = utf8_string_or_none( svn_status.repos_relpath );
    status[ str_revision ] = toSvnRevNum( svn_status.revision );
    status[ str_changed_revision ] = toSvnRevNum( svn_status.changed_rev );
    status[ str_changed_date ] = toObject( svn_status.changed_date );
    status[ str_changed_author ] = utf8_string_or_none( svn_status.changed_author );
    status[ str_is_switched ] = Py::Boolean( svn_status.switched );
    status[ str_is_file_external ] = Py::Boolean( svn_status.file_external );
    if( svn_status.lock == NULL )
    {
        status[ str_lock ] = Py::None();
    }
    else
    {
        status[ str_lock ] = toObject( *svn_status.lock, wrapper_lock );
    }
    status[ str_changelist ] = utf8_string_or_none( svn_status.changelist );
    status[ str_depth ] = toEnumValue( svn_status.depth );
    status[ str_ood_kind ] = toEnumValue( svn_status.ood_kind );
    status[ str_repos_node_status ] = toEnumValue( svn_status.repos_node_status );
    status[ str_repos_text_status ]  = toEnumValue( svn_status.repos_text_status );
    status[ str_repos_prop_status ]  = toEnumValue( svn_status.repos_prop_status );
    if( svn_status.repos_lock == NULL )
    {
        status[ str_repos_lock ] = Py::None();
    }
    else
    {
        status[ str_repos_lock ] = toObject( *svn_status.repos_lock, wrapper_lock );
    }
    status[ str_ood_changed_rev ] = toSvnRevNum( svn_status.ood_changed_rev );
    status[ str_ood_changed_date ] = toObject( svn_status.ood_changed_date );
    status[ str_ood_changed_author ] = utf8_string_or_none( svn_status.ood_changed_author );
    status[ str_moved_from_abspath ] = utf8_string_or_none( svn_status.moved_from_abspath );
    status[ str_moved_to_abspath ] = utf8_string_or_none( svn_status.moved_to_abspath );

    return wrapper_status2.wrapDict( status );
}
#endif

Py::Object toObject
    (
    Py::String path,
    pysvn_wc_status_t &svn_status,
    SvnPool &pool,
    const DictWrapper &wrapper_status,
    const DictWrapper &wrapper_entry,
    const DictWrapper &wrapper_lock
    )
{
    Py::Dict status;

    status[ str_path ] = path;
    if( svn_status.entry == NULL )
    {
        status[ str_entry ] = Py::None();
    }
    else
    {
        status[ str_entry ] = toObject( *svn_status.entry, pool, wrapper_entry );
    }
    if( svn_status.repos_lock == NULL )
    {
        status[ str_repos_lock ] = Py::None();
    }
#if defined( PYSVN_HAS_CLIENT_STATUS2 )
    else
    {
        status[ str_repos_lock ] = toObject( *svn_status.repos_lock, wrapper_lock );
    }
#endif

    long is_versioned;
    switch( svn_status.text_status )
    {
    // exists, but uninteresting
    case svn_wc_status_normal:
    // is scheduled for addition
    case svn_wc_status_added:
    // under v.c., but is missing
    case svn_wc_status_missing:
    // scheduled for deletion
    case svn_wc_status_deleted:
    // was deleted and then re-added
    case svn_wc_status_replaced:
    // text or props have been modified
    case svn_wc_status_modified:
    // local mods received repos mods (### unused)
    case svn_wc_status_merged:
    // local mods received conflicting repos mods
    case svn_wc_status_conflicted:
        is_versioned = 1;
        break;

    // an unversioned resource is in the way of the versioned resource
    case svn_wc_status_obstructed:
    // does not exist
    case svn_wc_status_none:
    // is not a versioned thing in this wc
    case svn_wc_status_unversioned:
    // is unversioned but configured to be ignored
    case svn_wc_status_ignored:
    // an unversioned directory path populated by an svn:externals
    // property; this status is not used for file externals
    case svn_wc_status_external:
    // a directory doesn't contain a complete entries list
    case svn_wc_status_incomplete:
    // assume any new status not versioned
    default:
        is_versioned = 0;
    }

    status[ str_is_versioned ] = Py::Int( is_versioned );
    status[ str_is_locked ] = Py::Int( svn_status.locked );
    status[ str_is_copied ] = Py::Int( svn_status.copied );
    status[ str_is_switched ] = Py::Int( svn_status.switched );
    status[ str_prop_status ] = toEnumValue( svn_status.prop_status );
    status[ str_text_status ] = toEnumValue( svn_status.text_status );
    status[ str_repos_prop_status ] = toEnumValue( svn_status.repos_prop_status );
    status[ str_repos_text_status ] = toEnumValue( svn_status.repos_text_status );

    return wrapper_status.wrapDict( status );
}

Py::Object toObject
    (
    const svn_wc_entry_t &svn_entry,
    SvnPool &pool,
    const DictWrapper &wrapper_entry
    )
{
    Py::Dict entry;

    entry[ str_checksum ] = utf8_string_or_none( svn_entry.checksum );
    entry[ str_commit_author ] = utf8_string_or_none( svn_entry.cmt_author );
    entry[ str_commit_revision ] = toSvnRevNum( svn_entry.cmt_rev );
    entry[ str_commit_time ] = toObject( svn_entry.cmt_date );
    entry[ str_conflict_new ] = path_string_or_none( svn_entry.conflict_new, pool );
    entry[ str_conflict_old ] = path_string_or_none( svn_entry.conflict_old, pool );
    entry[ str_conflict_work ] = path_string_or_none( svn_entry.conflict_wrk, pool );
    entry[ str_copy_from_revision ] = toSvnRevNum( svn_entry.copyfrom_rev );
    entry[ str_copy_from_url ] = utf8_string_or_none( svn_entry.copyfrom_url );
    entry[ str_is_absent ] = Py::Int( svn_entry.absent );
    entry[ str_is_copied ] = Py::Int( svn_entry.copied );
    entry[ str_is_deleted ] = Py::Int( svn_entry.deleted );
    entry[ str_kind ] = toEnumValue( svn_entry.kind );
    entry[ str_name ] = path_string_or_none( svn_entry.name, pool );
    entry[ str_properties_time ] = toObject( svn_entry.prop_time );
    entry[ str_property_reject_file ] = path_string_or_none( svn_entry.prejfile, pool );
    entry[ str_repos ] = utf8_string_or_none( svn_entry.repos );
    entry[ str_revision ] = toSvnRevNum( svn_entry.revision );
    entry[ str_schedule ] = toEnumValue( svn_entry.schedule );
    entry[ str_text_time ] = toObject( svn_entry.text_time );
    entry[ str_url ] = utf8_string_or_none( svn_entry.url );
    entry[ str_uuid ] = utf8_string_or_none( svn_entry.uuid );
#if defined( PYSVN_HAS_CLIENT_STATUS2 )
    entry[ str_lock_token ] = utf8_string_or_none( svn_entry.lock_token );
    entry[ str_lock_owner ] = utf8_string_or_none( svn_entry.lock_owner );
    entry[ str_lock_comment ] = utf8_string_or_none( svn_entry.lock_comment );
    entry[ str_lock_creation_date ] = toObject( svn_entry.lock_creation_date );
#endif

    return wrapper_entry.wrapDict( entry );
}

#if defined( PYSVN_HAS_CLIENT_INFO )
Py::Object toObject
    (
    const svn_info_t &info,
    const DictWrapper &wrapper_info,
    const DictWrapper &wrapper_lock,
    const DictWrapper &wrapper_wc_info
    )
{
    Py::Dict py_info;

    // Where the item lives in the repository.
    py_info[str_URL] = utf8_string_or_none( info.URL );

    // The revision of the object.  If path_or_url is a working-copy
    // path, then this is its current working revnum.  If path_or_url
    // is a URL, then this is the repos revision that path_or_url lives in.
    py_info[str_rev] = toSvnRevNum( info.rev );

    // The node's kind.
    py_info[str_kind] = toEnumValue( info.kind );

    // The root URL of the repository.
    py_info[str_repos_root_URL] = utf8_string_or_none( info.repos_root_URL );

    // The repository's UUID.
    py_info[str_repos_UUID] = utf8_string_or_none( info.repos_UUID );

    // The last revision in which this object changed.
    py_info[str_last_changed_rev] = toSvnRevNum( info.last_changed_rev );

    // The date of the last_changed_rev.
    py_info[str_last_changed_date] = toObject( info.last_changed_date );

    // The author of the last_changed_rev.
    py_info[str_last_changed_author] = utf8_string_or_none( info.last_changed_author );

#if defined( PYSVN_HAS_CLIENT_LOCK )
    // An exclusive lock, if present.  Could be either local or remote.
    if( info.lock == NULL )
    {
        py_info[str_lock] = Py::None();
    }
    else
    {
        py_info[str_lock] = toObject( *info.lock, wrapper_lock );
    }
#endif

    // Whether or not to ignore the next 10 wc-specific fields.
    if( info.has_wc_info == 0 )
    {
        py_info[str_wc_info] = Py::None();
    }
    else
    {
        Py::Dict py_wc_info;

        py_wc_info[str_schedule] = toEnumValue( info.schedule );
        py_wc_info[str_copyfrom_url] = utf8_string_or_none( info.copyfrom_url );
        py_wc_info[str_copyfrom_rev] = toSvnRevNum( info.copyfrom_rev );
        py_wc_info[str_text_time] = toObject( info.text_time );
        py_wc_info[str_prop_time] = toObject( info.prop_time );
        py_wc_info[str_checksum] = utf8_string_or_none( info.checksum );
        py_wc_info[str_conflict_old] = utf8_string_or_none( info.conflict_old );
        py_wc_info[str_conflict_new] = utf8_string_or_none( info.conflict_new );
        py_wc_info[str_conflict_work] = utf8_string_or_none( info.conflict_wrk );
        py_wc_info[str_prejfile] = utf8_string_or_none( info.prejfile );
#ifdef PYSVN_HAS_SVN_INFO_T__CHANGELIST
        py_wc_info[str_changelist] = utf8_string_or_none( info.changelist );
        py_wc_info[str_depth] = toEnumValue( info.depth );
#endif
#ifdef PYSVN_HAS_SVN_INFO_T__SIZES
#ifdef HAVE_LONG_LONG
        if( info.working_size == SVN_INFO_SIZE_UNKNOWN )
            py_wc_info[str_working_size] = Py::None();
        else
            py_wc_info[str_working_size] = Py::LongLong( static_cast<PY_LONG_LONG>( info.working_size ) );
        if( info.size == SVN_INFO_SIZE_UNKNOWN )
            py_wc_info[str_size] = Py::None();
        else
            py_wc_info[str_size] = Py::LongLong( static_cast<PY_LONG_LONG>( info.size ) );
#else
        if( info.working_size == SVN_INFO_SIZE_UNKNOWN )
            py_wc_info[str_working_size] = Py::None();
        else
            py_wc_info[str_working_size] = Py::Int( static_cast<int>( info.working_size ) );
        if( info.size == SVN_INFO_SIZE_UNKNOWN )
            py_wc_info[str_size] = Py::None();
        else
            py_wc_info[str_size] = Py::Int( static_cast<int>( info.size ) );
#endif
#endif
        py_info[str_wc_info] = wrapper_wc_info.wrapDict( py_wc_info );
    }

    return wrapper_info.wrapDict( py_info );
}
#endif

#if defined( PYSVN_HAS_CLIENT_INFO3 )
Py::Object toObject
    (
    const svn_wc_conflict_version_t *info
    )
{
    if( info == NULL )
    {
        return Py::None();
    }

    Py::Dict d;

    d[str_repos_url] = utf8_string_or_none( info->repos_url );
    d[str_peg_rev] = toSvnRevNum( info->peg_rev );
    d[str_path_in_repos] = utf8_string_or_none( info->path_in_repos );
    d[str_node_kind] = toEnumValue( info->node_kind );
#if defined( PYSVN_HAS_SVN_1_8 )
    d[str_repos_UUID] = utf8_string_or_none( info->repos_uuid );
#endif

    return d;
}

Py::String toHex( const unsigned char *bytes, size_t length )
{
    static char hex_digits[] = "0123456789abcdef";
    std::string human;


    for( size_t index=0; index < length; ++index )
    {
        human += hex_digits[ bytes[index] >> 4 ];
        human += hex_digits[ bytes[index] & 0x0f ];
    }

    return Py::String( human );
}

Py::Object toObject
    (
    const svn_client_info2_t &info,
    SvnPool &pool,
    const DictWrapper &wrapper_info,
    const DictWrapper &wrapper_lock,
    const DictWrapper &wrapper_wc_info
    )
{
    Py::Dict py_info;

    // Where the item lives in the repository.
    py_info[str_URL] = utf8_string_or_none( info.URL );

    // The revision of the object.  If path_or_url is a working-copy
    // path, then this is its current working revnum.  If path_or_url
    // is a URL, then this is the repos revision that path_or_url lives in.
    py_info[str_rev] = toSvnRevNum( info.rev );

    // The root URL of the repository.
    py_info[str_repos_root_URL] = utf8_string_or_none( info.repos_root_URL );

    // The repository's UUID.
    py_info[str_repos_UUID] = utf8_string_or_none( info.repos_UUID );

    // The node's kind.
    py_info[str_kind] = toEnumValue( info.kind );

    py_info[str_size] = toFilesize( info.size );

    // The last revision in which this object changed.
    py_info[str_last_changed_rev] = toSvnRevNum( info.last_changed_rev );

    // The date of the last_changed_rev.
    py_info[str_last_changed_date] = toObject( info.last_changed_date );

    // The author of the last_changed_rev.
    py_info[str_last_changed_author] = utf8_string_or_none( info.last_changed_author );

    // An exclusive lock, if present.  Could be either local or remote.
    if( info.lock == NULL )
    {
        py_info[str_lock] = Py::None();
    }
    else
    {
        py_info[str_lock] = toObject( *info.lock, wrapper_lock );
    }

    // Whether or not to ignore the next 10 wc-specific fields.
    if( info.wc_info == NULL )
    {
        py_info[str_wc_info] = Py::None();
    }
    else
    {
        Py::Dict py_wc_info;

        py_wc_info[str_schedule] = toEnumValue( info.wc_info->schedule );
        py_wc_info[str_copyfrom_url] = utf8_string_or_none( info.wc_info->copyfrom_url );
        py_wc_info[str_copyfrom_rev] = toSvnRevNum( info.wc_info->copyfrom_rev );
        if( info.wc_info->checksum != NULL )
        {
            switch( info.wc_info->checksum->kind )
            {
            case svn_checksum_md5:
                py_wc_info[str_checksum] = toHex( info.wc_info->checksum->digest, 16 );
                break;
            case svn_checksum_sha1:
                py_wc_info[str_checksum] = toHex( info.wc_info->checksum->digest, 20 );
                break;
            default:
                py_wc_info[str_checksum] = Py::None();
            }
        }
        else
        {
            py_wc_info[str_checksum] = Py::None();
        }

        py_wc_info[str_changelist] = utf8_string_or_none( info.wc_info->changelist );
        py_wc_info[str_depth] = toEnumValue( info.wc_info->depth );
#ifdef HAVE_LONG_LONG
        if( info.wc_info->recorded_size == SVN_INFO_SIZE_UNKNOWN )
            py_wc_info[str_recorded_size] = Py::None();
        else
            py_wc_info[str_recorded_size] = Py::LongLong( static_cast<PY_LONG_LONG>( info.wc_info->recorded_size ) );
#else
        if( info.wc_info->recorded_size == SVN_INFO_SIZE_UNKNOWN )
            py_wc_info[str_recorded_size] = Py::None();
        else
            py_wc_info[str_recorded_size] = Py::Int( static_cast<int>( info.wc_info->recorded_size ) );
#endif
        py_wc_info[str_recorded_time] = toObject( info.wc_info->recorded_time );

        // I'm guesses that recorded size is what used to be size and working_size
        py_wc_info[str_size] = py_wc_info[str_recorded_size];
        py_wc_info[str_working_size] = py_wc_info[str_recorded_size];

        // I'm guesses that recorded date is what used to be text_time and prop_time
        py_wc_info[str_text_time] = py_wc_info[str_recorded_time];
        py_wc_info[str_prop_time] = py_wc_info[str_recorded_time];

        // conflicts
        int count = 0;
        if( info.wc_info->conflicts != NULL )
        {
            count = info.wc_info->conflicts->nelts;
        }

        switch( count )
        {
        case 0:
            py_wc_info[str_conflict_old] = Py::None();
            py_wc_info[str_conflict_new] = Py::None();
            py_wc_info[str_conflict_work] = Py::None();
            py_wc_info[str_prejfile] = Py::None();
            break;

        case 1:
            // map for backwards compatibility
            {
            svn_wc_conflict_description2_t *item = ((svn_wc_conflict_description2_t **)info.wc_info->conflicts->elts)[0];

            py_wc_info[str_conflict_old] = utf8_string_or_none( item->my_abspath );
            py_wc_info[str_conflict_new] = utf8_string_or_none( item->their_abspath );
            py_wc_info[str_conflict_work] = utf8_string_or_none( item->merged_file );
            py_wc_info[str_prejfile] = utf8_string_or_none( item->their_abspath );
            }
            break;

        default:
            {
            Py::List all_conflicts;

            for( int i=0; i<count; ++i )
            {
                Py::Dict conflict;

                svn_wc_conflict_description2_t *item = ((svn_wc_conflict_description2_t **)info.wc_info->conflicts->elts)[i];

                conflict[str_local_abspath] = path_string_or_none( item->local_abspath, pool );
                conflict[str_node_kind] = toEnumValue( item->node_kind );
                conflict[str_kind] = toEnumValue( item->kind );
                if( item->kind == svn_wc_conflict_kind_property )
                {
                    conflict[str_property_name] = utf8_string_or_none( item->property_name );
                }
                else
                {
                    conflict[str_property_name] = Py::None();
                }

                if( item->kind == svn_wc_conflict_kind_text )
                {
                    conflict[str_is_binary] = Py::Boolean( item->is_binary );
                    conflict[str_mime_type] = utf8_string_or_none( item->mime_type );
                }
                else
                {
                    conflict[str_is_binary] = Py::None();
                    conflict[str_mime_type] = Py::None();
                }
                conflict[str_action] = toEnumValue( item->action );
                conflict[str_reason] = toEnumValue( item->reason );
                conflict[str_base_abspath] = path_string_or_none( item->base_abspath, pool );
                conflict[str_their_abspath] = path_string_or_none( item->their_abspath, pool );
                conflict[str_my_abspath] = path_string_or_none( item->my_abspath, pool );
                conflict[str_merged_file] = path_string_or_none( item->merged_file, pool );

                conflict[str_operation] = toEnumValue( item->operation );
                conflict[str_src_left_version] = toObject( item->src_left_version );
                conflict[str_src_right_version] = toObject( item->src_right_version );

                all_conflicts.append( conflict );
            }

            py_wc_info[str_conflicts] = all_conflicts;
            }
            break;
        }

        py_wc_info[str_wcroot_abspath] = utf8_string_or_none( info.wc_info->wcroot_abspath );
        py_wc_info[str_moved_to_abspath] = utf8_string_or_none( info.wc_info->moved_to_abspath );
        py_wc_info[str_moved_from_abspath] = utf8_string_or_none( info.wc_info->moved_from_abspath );

        py_info[str_wc_info] = wrapper_wc_info.wrapDict( py_wc_info );
    }

    return wrapper_info.wrapDict( py_info );
}
#endif

#if defined( PYSVN_HAS_CLIENT_LOCK )
Py::Object toObject
    (
    const svn_lock_t &lock,
    const DictWrapper &wrapper_lock
    )
{

    Py::Dict py_lock;

    py_lock[str_path] = utf8_string_or_none( lock.path );
    py_lock[str_token] = utf8_string_or_none( lock.token );
    py_lock[str_owner] = utf8_string_or_none( lock.owner );
    py_lock[str_comment] = utf8_string_or_none( lock.comment );
    py_lock[str_is_dav_comment] = Py::Boolean( lock.is_dav_comment != 0 );
    if( lock.creation_date == 0 )
    {
        py_lock[str_creation_date] = Py::None();
    }
    else
    {
        py_lock[str_creation_date] = toObject( lock.creation_date );
    }
    if( lock.expiration_date == 0 )
    {
        py_lock[str_expiration_date] = Py::None();
    }
    else
    {
        py_lock[str_expiration_date] = toObject( lock.expiration_date );
    }

    return wrapper_lock.wrapDict( py_lock );
}
#endif

Py::Object propsToObject( apr_hash_t *props, SvnPool &pool )
{
    Py::Dict py_prop_dict;

    for( apr_hash_index_t *hi = apr_hash_first( pool, props ); hi; hi = apr_hash_next( hi ) )
    {
        const void *key = NULL;
        void *val = NULL;

        apr_hash_this (hi, &key, NULL, &val);
        const svn_string_t *propval = (const svn_string_t *)val;
        
        py_prop_dict[ Py::String( (const char *)key ) ] = Py::String( propval->data, (int)propval->len );
    }

    return py_prop_dict;
}

#if defined( PYSVN_HAS_CLIENT_PROPGET5 )
Py::Object inheritedPropsToObject( apr_array_header_t *inherited_props, SvnPool &pool )
{
    Py::Dict all_inherited_props;

    for (int j = 0; j < inherited_props->nelts; ++j)
    {
        svn_prop_inherited_item_t *item = ((svn_prop_inherited_item_t **)inherited_props->elts)[j];

        Py::String path_or_url = utf8_string_or_none( item->path_or_url );
        Py::Dict py_prop_dict = propsToObject( item->prop_hash, pool );

        all_inherited_props[ path_or_url ] = py_prop_dict;
    }

    return all_inherited_props;
}
#endif

void proplistToObject( Py::List &py_path_propmap_list, apr_array_header_t *props, SvnPool &pool )
{
    for (int j = 0; j < props->nelts; ++j)
    {
        svn_client_proplist_item_t *item = ((svn_client_proplist_item_t **)props->elts)[j];

        Py::Object py_prop_dict( propsToObject( item->prop_hash, pool ) );

        std::string node_name( item->node_name->data, item->node_name->len );

        Py::Tuple py_path_proplist( 2 );
        py_path_proplist[0] = Py::String( osNormalisedPath( node_name, pool ) );
        py_path_proplist[1] = py_prop_dict;

        py_path_propmap_list.append( py_path_proplist );
    }
}

Py::Object revnumListToObject( apr_array_header_t *revs, SvnPool &pool )
{
    Py::List py_list;

    for (int j = 0; j < revs->nelts; ++j)
    {
        svn_revnum_t revnum = ((svn_revnum_t *)revs->elts)[j];
        py_list.append( toSvnRevNum( revnum ) );
    }

    return py_list;
}

Py::Bytes asUtf8Bytes( Py::Object obj )
{
    Py::String any( obj );
    Py::Bytes utf8( any.encode( name_utf8 ) );
    return utf8;
}

apr_array_header_t *targetsFromStringOrList( Py::Object arg, SvnPool &pool )
{
    int num_targets = 1;
    if( arg.isList() )
    {
        Py::List paths( arg );
        num_targets = paths.length();
    }

    apr_array_header_t *targets = apr_array_make( pool, num_targets, sizeof( const char * ) );

    std::string type_error_message;
    try
    {
        if( arg.isList() )
        {
            Py::List path_list( arg );

            for( Py::List::size_type i=0; i<path_list.length(); i++ )
            {
                type_error_message = "expecting path list members to be strings (arg 1)";

                Py::Bytes path_str( asUtf8Bytes( path_list[i] ) );
                std::string norm_path( svnNormalisedIfPath( path_str.as_std_string(), pool ) );

                *(char **)apr_array_push( targets ) = apr_pstrdup( pool, norm_path.c_str() );
            }
        }
        else
        {
            type_error_message = "expecting path to be a string (arg 1)";
            Py::Bytes path_str( asUtf8Bytes( arg ) );

            std::string norm_path( svnNormalisedIfPath( path_str.as_std_string(), pool ) );

            *(char **)apr_array_push( targets ) = apr_pstrdup( pool, norm_path.c_str() );
        }
    }
    catch( Py::TypeError & )
    {
        throw Py::TypeError( type_error_message );
    }

    return targets;
}

apr_array_header_t *arrayOfStringsFromListOfStrings( Py::Object arg, SvnPool &pool )
{
    apr_array_header_t *array = NULL;

    std::string type_error_message;
    try
    {
        type_error_message = "expecting list of strings";
        Py::List path_list( arg );
        Py::List::size_type num_targets = path_list.length();

        array = apr_array_make( pool, num_targets, sizeof( const char * ) );

        for( Py::List::size_type i=0; i<num_targets; i++ )
        {
            type_error_message = "expecting list members to be strings";

            Py::Bytes str( asUtf8Bytes( path_list[i] ) );
            *(char **)apr_array_push( array ) = apr_pstrdup( pool, str.as_std_string().c_str() );
        }
    }
    catch( Py::TypeError & )
    {
        throw Py::TypeError( type_error_message );
    }

    return array;
}

Py::List toListOfStrings( Py::Object obj )
{
    Py::List list;
    if( obj.isList() )
        list = obj;
    else
        list.append( obj );

    // check all members of the list are strings
    for( Py::List::size_type i=0; i<list.length(); i++ )
    {
        Py::String path_str( list[i] );
    }

    return list;
}

apr_hash_t *hashOfStringsFromDictOfStrings( Py::Object arg, SvnPool &pool )
{
    Py::Dict dict( arg );

    apr_hash_t *hash = apr_hash_make( pool );

    std::string type_error_message;
    try
    {
        Py::List all_keys( dict.keys() );

        for( Py::List::size_type i=0; i<all_keys.length(); i++ )
        {
            type_error_message = "expecting string key in dict";
            Py::Bytes key( asUtf8Bytes( all_keys[i] ) );

            type_error_message = "expecting string value in dict";
            Py::Bytes value( asUtf8Bytes( dict[ key ] ) );

            char *hash_key = apr_pstrdup( pool, key.as_std_string().c_str() );
            svn_string_t *hash_value = svn_string_create( value.as_std_string().c_str(), pool );

            apr_hash_set( hash, hash_key, APR_HASH_KEY_STRING, hash_value );
        }
    }
    catch( Py::TypeError & )
    {
        throw Py::TypeError( type_error_message );
    }

    return hash;
}

Py::Object direntsToObject( apr_hash_t *dirents, SvnPool &pool )
{
    Py::Dict py_dirents_dict;

    for( apr_hash_index_t *hi = apr_hash_first( pool, dirents ); hi; hi = apr_hash_next( hi ) )
    {
        const void *key = NULL;
        void *val = NULL;

        apr_hash_this (hi, &key, NULL, &val);
        const svn_fs_dirent_t *dirent = (const svn_fs_dirent_t *)val;

        py_dirents_dict[ Py::String( (const char *)key ) ] = toEnumValue( dirent->kind );
    }

    return py_dirents_dict;
}

//------------------------------------------------------------
//
//
//
//------------------------------------------------------------
#if defined( PYSVN_HAS_COMMIT_CALLBACK2_T )

CommitInfoResult::CommitInfoResult( SvnPool &pool )
: m_all_results( apr_array_make( pool, 16, sizeof( svn_commit_info_t * ) ) )
, m_pool( pool )
{
}

CommitInfoResult::~CommitInfoResult()
{
}

int CommitInfoResult::count()
{
    if( m_all_results == NULL )
        return 0;

    return m_all_results->nelts;
}

const svn_commit_info_t *CommitInfoResult::result( int index )
{
    assert( m_all_results != NULL );
    assert( index >= 0 && index < m_all_results->nelts );
    return APR_ARRAY_IDX( m_all_results, index, const svn_commit_info_t * );
}

extern "C" svn_error_t *CommitInfoResult_callback( const svn_commit_info_t *commit_info, void *baton, apr_pool_t * )
{
    CommitInfoResult *result = CommitInfoResult::castBaton( baton );

    if( result->m_all_results == NULL )
    {
        return svn_error_create( APR_ENOMEM, NULL, "no memory for commit info results" );
    }

    svn_commit_info_t *copy = svn_commit_info_dup( commit_info, result->m_pool );
    if( copy == NULL )
    {
        return svn_error_create( APR_ENOMEM, NULL, "no memory for commit info results" );
    }

    APR_ARRAY_PUSH( result->m_all_results, const svn_commit_info_t * ) = copy;

    return SVN_NO_ERROR;
}


Py::Object toObject( const svn_commit_info_t *commit_info )
{
    Py::Dict commit_info_dict;

    commit_info_dict[ str_date ] = utf8_string_or_none( commit_info->date );
    commit_info_dict[ str_author ] = utf8_string_or_none( commit_info->author );
    if( commit_info->post_commit_err == NULL )
    {
        commit_info_dict[ str_post_commit_err ] = Py::None();
    }
    else
    {
        // this field is sometimes a pointer to a none UTF8 string
        commit_info_dict[ str_post_commit_err ] = utf8_string_or_none( commit_info->post_commit_err );
    }

    if( SVN_IS_VALID_REVNUM( commit_info->revision ) )
    {
        commit_info_dict[ str_revision ] = toSvnRevNum( commit_info->revision );
    }
    else
    {
        commit_info_dict[ str_revision ] = Py::None();
    }

    return commit_info_dict;
}

Py::Object toObject( CommitInfoResult &commit_info, const DictWrapper &wrapper_commit_info, int commit_style )
{
    if( commit_info.count() == 0 )
    {
        Py::Dict commit_info_dict;

        commit_info_dict[ str_date ] = Py::None();
        commit_info_dict[ str_author ] = Py::None();
        commit_info_dict[ str_post_commit_err ] = Py::None();
        commit_info_dict[ str_revision ] = Py::None();

        return commit_info_dict;
    }

    if( commit_style == 0 )
    {
        // assume the last commit revision is the best to return
        const svn_commit_info_t *last = commit_info.result( commit_info.count()-1 );

        if( !SVN_IS_VALID_REVNUM( last->revision ) )
        {
            return Py::None();
        }

        return toSvnRevNum( last->revision );
    }
    else
    if( commit_style == 1 )
    {
        const svn_commit_info_t *last = commit_info.result( commit_info.count()-1 );
        return toObject( last );
    }
    else
    if( commit_style == 2 )
    {
        Py::List all_results;

        for( int index=0; index<commit_info.count(); ++index )
        {
            Py::Dict py_commit_info( toObject( commit_info.result( index ) ) );
            all_results.append( wrapper_commit_info.wrapDict( py_commit_info ) );
        }

        return all_results;
    }
    else
    {
        throw Py::RuntimeError( "commit_style value invalid" );
    }
}
#endif
