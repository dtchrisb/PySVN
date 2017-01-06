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


static const char g_utf_8[] = "utf-8";

FunctionArguments::FunctionArguments
    (
    const char *function_name,
    const argument_description *arg_desc,
    const Py::Tuple &args,
    const Py::Dict &kws
    )
: m_function_name( function_name )
, m_arg_desc( arg_desc )
, m_args( args )
, m_kws( kws )
, m_checked_args()
, m_min_args( 0 )
, m_max_args( 0 )
{
    // find out the min and max number of args
    for( const argument_description *p=arg_desc; p->m_arg_name; ++p )
    {
        m_max_args++;
        if( p->m_required )
        {
            m_min_args++;
        }
    }
}

FunctionArguments::~FunctionArguments()
{
    // would like to check for all args processed here
    // but if an expection was raise because of an early
    // problem with the args we would assert on a false 
    // positive
}

static char *int_to_string_inner( int n, char *buffer )
{
    char digit = (n%10) + '0';
    int remainder = n/10;
    if( remainder > 0 )
        buffer = int_to_string_inner( remainder, buffer );
    *buffer++ = digit;
    return buffer;
}

static const char *int_to_string( int n )
{
    static char number_string[20];
    int_to_string_inner( n, &number_string[0] );
    return number_string;
}

void FunctionArguments::check()
{
    if( m_args.size() > m_max_args )
    {
        std::string msg = m_function_name;
        msg += "() takes exactly ";
        msg += int_to_string( m_max_args );
        msg += " arguments (";
        msg += int_to_string( m_args.size() );
        msg += " given)";
        throw Py::TypeError( msg );
    }

    Py::Tuple::size_type t_i;
    // place all the positional args in the checked args dict
    for( t_i=0; t_i<m_args.size(); t_i++ )
    {
        m_checked_args[ m_arg_desc[t_i].m_arg_name ] = m_args[t_i];
    }

    // look for args by name in the kws dict
    for( t_i=0; t_i<m_max_args; t_i++ )
    {
        const argument_description &arg_desc = m_arg_desc[t_i];

        // check for duplicate
        if( m_kws.hasKey( arg_desc.m_arg_name ) )
        {
            if( m_checked_args.hasKey( arg_desc.m_arg_name ) )
            {
                std::string msg = m_function_name;
                msg += "() multiple values for keyword argument '";
                msg += arg_desc.m_arg_name;
                msg += "'";
                throw Py::TypeError( msg );
            }

            m_checked_args[ arg_desc.m_arg_name ] = m_kws[ arg_desc.m_arg_name ];
        }
    }

    // check for names we dont known about
    Py::List::size_type l_i;
    Py::List names( m_kws.keys() );
    for( l_i=0; l_i< names.length(); l_i++ )
    {
        bool found = false;
        Py::String py_name( names[l_i] );
        std::string name( py_name.as_std_string( g_utf_8 ) );

        for( t_i=0; t_i<m_max_args; t_i++ )
        {
            if( name == m_arg_desc[t_i].m_arg_name )
                {
                found = true;
                break;
                }
        }

        if( !found )
        {
            std::string msg = m_function_name;
            msg += "() got an unexpected keyword argument '";
            msg += name;
            msg += "'";
            throw Py::TypeError( msg );
        }
    }

    // check for min args
    for( t_i=0; t_i<m_min_args; t_i++ )
    {
        const argument_description &arg_desc = m_arg_desc[t_i];

        // check for duplicate
        if( !m_checked_args.hasKey( arg_desc.m_arg_name ) )
        {
            std::string msg = m_function_name;
            msg += "() required argument '";
            msg += arg_desc.m_arg_name;
            msg += "'";
            throw Py::TypeError( msg );
        }
    }
}

bool FunctionArguments::hasArg( const char *arg_name )
{
    std::string std_arg_name( arg_name );

    bool found_arg_name = false;

    Py::Tuple::size_type t_i;
    // place all the positional args in the checked args dict
    for( t_i=0; t_i < m_max_args; t_i++ )
    {
        if( std_arg_name == m_arg_desc[t_i].m_arg_name )
        {
            found_arg_name = true;
            break;
        }
    }
    if( !found_arg_name )
    {
        std::string msg = m_function_name;
        msg += "() coding error: function does not have an arg called '";
        msg += std_arg_name;
        msg += "'";
        throw Py::RuntimeError( msg );
    }

    return m_checked_args.hasKey( arg_name );
}

bool FunctionArguments::hasArgNotNone( const char *arg_name )
{
    if( !hasArg( arg_name ) )
    {
        return false;
    }
    Py::Object arg = m_checked_args[ arg_name ];
    return !arg.isNone();
}

Py::Object FunctionArguments::getArg( const char *arg_name )
{
    if( !hasArg( arg_name ) )
    {
        std::string msg = m_function_name;
        msg += "() internal error - getArg called twice or for option arg that is missing with bad arg_name: ";
        msg += arg_name;
        throw Py::AttributeError( msg );
    }
    Py::Object arg = m_checked_args[ arg_name ];
    // Make sure that each arg is only fetched once
    // to detect coding errors
    m_checked_args.delItem( arg_name );
    return arg;
}

bool FunctionArguments::getBoolean( const char *name )
{
    return getArg( name ).isTrue();
}

bool FunctionArguments::getBoolean( const char *name, bool default_value )
{
    if( hasArg( name ) )
    {
        return getBoolean( name );
    }
    else
    {
        return default_value;
    }
}

int FunctionArguments::getInteger( const char *name )
{
    Py::Int int_val( getArg( name ) );
    return int_val;
}

int FunctionArguments::getInteger( const char *name, int default_value )
{
    if( hasArg( name ) )
    {
        return getInteger( name );
    }
    else
    {
        return default_value;
    }
}

long FunctionArguments::getLong( const char *name )
{
    Py::Long long_val( getArg( name ) );
    return long_val;
}

long FunctionArguments::getLong( const char *name, long default_value )
{
    if( hasArg( name ) )
    {
        return getLong( name );
    }
    else
    {
        return default_value;
    }
}

std::string FunctionArguments::getUtf8String( const char *name )
{
    Py::String any( getArg( name ) );
    return any.as_std_string( g_utf_8 );
}

std::string FunctionArguments::getUtf8String( const char *name, const std::string &default_value )
{
    if( hasArg( name ) )
    {
        return getUtf8String( name );
    }
    else
    {
        return default_value;
    }
}

std::string FunctionArguments::getBytes( const char *name )
{
    Py::String any( getArg( name ) );
    return any.as_std_string( g_utf_8 );
}

std::string FunctionArguments::getBytes( const char *name, const std::string &default_value )
{
    if( hasArg( name ) )
    {
        return getBytes( name );
    }
    else
    {
        return default_value;
    }
}

svn_opt_revision_t FunctionArguments::getRevision( const char *name )
{
    Py::Object obj( getArg( name ) );
    if( pysvn_revision::check( obj ) )
    {
        pysvn_revision *rev = static_cast<pysvn_revision *>( obj.ptr() );
        // copy out to caller
        return rev->getSvnRevision();
    }
    else
    {
        std::string msg = m_function_name;
        msg += "() expecting revision object for keyword ";
        msg += name;
        throw Py::AttributeError( msg );
    }
}

svn_opt_revision_t FunctionArguments::getRevision
    (
    const char *name,
    svn_opt_revision_kind default_value
    )
{
    if( hasArg( name ) )
    {
        return getRevision( name );
    }
    else
    {
        svn_opt_revision_t revision;
        revision.kind = default_value;
        if( revision.kind == svn_opt_revision_number )
            revision.value.number = 1;
        return revision;
    }
}

svn_opt_revision_t FunctionArguments::getRevision
    (
    const char *name,
    svn_opt_revision_t default_value
    )
{
    if( hasArg( name ) )
    {
        return getRevision( name );
    }
    else
    {
        return default_value;
    }
}

#if defined( PYSVN_HAS_SVN__DEPTH_PARAMETER )
svn_depth_t FunctionArguments::getDepth
    (
    const char *depth_name,
    const char *recursive_name,
    svn_depth_t default_value,
    svn_depth_t recursive_true_value,
    svn_depth_t recursive_false_value
    )
{
    if( hasArg( recursive_name ) && hasArg( depth_name ) )
    {
        std::string msg = m_function_name;
        msg += "() cannot mix ";
        msg += depth_name;
        msg += " and ";
        msg += recursive_name;
        throw Py::TypeError( msg );
    }

    if( hasArg( recursive_name ) )
    {
        if( getBoolean( recursive_name ) )
        {
            return recursive_true_value;
        }
        else
        {
            return recursive_false_value;
        }
    }

    if( hasArg( depth_name ) )
    {
        return getDepth( depth_name, default_value );
    }

    return default_value;
}

svn_depth_t FunctionArguments::getDepth
    (
    const char *depth_name,
    svn_depth_t default_value
    )
{
    if( hasArg( depth_name ) )
    {
        Py::Object value( getArg( depth_name ) );
        if( value.isNone() )
        {
            return default_value;
        }
        else
        {
            Py::ExtensionObject< pysvn_enum_value<svn_depth_t> > py_kind( value );
            return svn_depth_t( py_kind.extensionObject()->m_value );
        }
    }

    return default_value;
}

svn_depth_t FunctionArguments::getDepth
    (
    const char *depth_name
    )
{
    Py::ExtensionObject< pysvn_enum_value<svn_depth_t> > py_kind( getArg( depth_name ) );
    return svn_depth_t( py_kind.extensionObject()->m_value );
}
#endif // PYSVN_HAS_SVN__DEPTH_PARAMETER

#if defined( PYSVN_HAS_SVN_WC_CONFLICT_CHOICE_T )
svn_wc_conflict_choice_t FunctionArguments::getWcConflictChoice
    (
    const char *choice_name,
    svn_wc_conflict_choice_t default_value
    )
{
    if( hasArg( choice_name ) )
    {
        return getWcConflictChoice( choice_name );
    }

    return default_value;
}

svn_wc_conflict_choice_t FunctionArguments::getWcConflictChoice
    (
    const char *choice_name
    )
{
    Py::ExtensionObject< pysvn_enum_value<svn_wc_conflict_choice_t> > py_kind( getArg( choice_name ) );
    return svn_wc_conflict_choice_t( py_kind.extensionObject()->m_value );
}
#endif
