//
// ====================================================================
// Copyright (c) 2013 Barry A Scott.  All rights reserved.
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

#if defined( PYSVN_HAS_CLIENT_PATCH )
/*
svn_error_t *
svn_client_patch(const char *patch_abspath,
                 const char *wc_dir_abspath,
                 svn_boolean_t dry_run,
                 int strip_count,
                 svn_boolean_t reverse,
                 svn_boolean_t ignore_whitespace,
                 svn_boolean_t remove_tempfiles,
                 svn_client_patch_func_t patch_func,
                 void *patch_baton,
                 svn_client_ctx_t *ctx,
                 apr_pool_t *scratch_pool);
*/

static svn_error_t *patch_callback(
    void *baton,
    svn_boolean_t *filtered,
    const char *canon_path_from_patchfile,
    const char *patch_abspath,
    const char *reject_abspath,
    apr_pool_t *scratch_pool
    )
{
    *filtered = FALSE;

    return NULL;
}

Py::Object pysvn_client::cmd_patch( const Py::Tuple &a_args, const Py::Dict &a_kws )
{
    static argument_description args_desc[] =
    {
    { true,  name_patch_abspath },
    { true,  name_wc_dir_abspath },
    { false, name_strip_count },
    { false, name_dry_run },
    { false, name_reverse },
    { false, name_ignore_whitespace },
    { false, name_remove_tempfiles },
    { false, NULL }
    };
    FunctionArguments args( "patch", args_desc, a_args, a_kws );
    args.check();

    std::string patch_abspath( args.getUtf8String( name_patch_abspath ) );
    std::string wc_dir_abspath( args.getUtf8String( name_wc_dir_abspath ) );
    int strip_count = args.getInteger( name_strip_count, 0 );
    if( strip_count < 0 )
    {
        throw Py::ValueError( "strip_count must be >= 0" );
    }

    bool dry_run = args.getBoolean( name_dry_run, false );
    bool ignore_whitespace = args.getBoolean( name_ignore_whitespace, false );
    bool remove_tempfiles = args.getBoolean( name_remove_tempfiles, false );
    bool reverse = args.getBoolean( name_reverse, false );

    SvnPool pool( m_context );

    try
    {
        std::string norm_patch_abspath( svnNormalisedIfPath( patch_abspath, pool ) );
        std::string norm_wc_dir_abspath( svnNormalisedIfPath( wc_dir_abspath, pool ) );

        checkThreadPermission();

        PythonAllowThreads permission( m_context );

        svn_error_t *error = svn_client_patch
            (
            norm_patch_abspath.c_str(),
            norm_wc_dir_abspath.c_str(),
            dry_run,
            strip_count,
            reverse,
            ignore_whitespace,
            remove_tempfiles,
            patch_callback,
            NULL,
            m_context,
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
    }

    // cannot convert to Unicode as we have no idea of the encoding of the bytes
    return Py::None();
}
#endif
