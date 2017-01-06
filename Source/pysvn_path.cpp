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
#include <algorithm>
#include "svn_path.h"
#include <sys/types.h>
#include <sys/stat.h>

// SVN lives in a world of its own for URL and path syntax
// here are routines to convert from OS conventions to SVN's


// is this a path of a url svn?
bool is_svn_url( const std::string &path_or_url )
{
    return svn_path_is_url( path_or_url.c_str() ) != 0;
}

// convert a path or URL to what SVN likes
std::string svnNormalisedIfPath( const std::string &unnormalised, SvnPool &pool )
{
    if( is_svn_url( unnormalised ) )
    {
        return svnNormalisedUrl( unnormalised, pool );
    }
    else
    {
        return svnNormalisedPath( unnormalised, pool );
    }
}


#if defined(PYSVN_HAS_SVN_1_7)
std::string svnNormalisedUrl( const std::string &unnormalised, SvnPool &pool )
{
    const char *normalised_path = svn_uri_canonicalize( unnormalised.c_str(), pool );
    return std::string( normalised_path );
}

std::string svnNormalisedPath( const std::string &unnormalised, SvnPool &pool )
{
    const char *normalised_path = svn_dirent_internal_style( unnormalised.c_str(), pool );
    return std::string( normalised_path );
}


// convert a path to what the native OS likes
std::string osNormalisedPath( const std::string &unnormalised, SvnPool &pool )
{
    const char *local_path = svn_dirent_local_style( unnormalised.c_str(), pool );

    return std::string( local_path );
}

#else
std::string svnNormalisedUrl( const std::string &unnormalised, SvnPool &pool )
{
    const char *normalised_path = svn_path_canonicalize( unnormalised.c_str(), pool );
    return std::string( normalised_path );
}

std::string svnNormalisedPath( const std::string &unnormalised, SvnPool &pool )
{
    const char *normalised_path = svn_path_internal_style( unnormalised.c_str(), pool );
    return std::string( normalised_path );
}

// convert a path to what the native OS likes
std::string osNormalisedPath( const std::string &unnormalised, SvnPool &pool )
{
    const char *local_path = svn_path_local_style( unnormalised.c_str(), pool );

    return std::string( local_path );
}
#endif
