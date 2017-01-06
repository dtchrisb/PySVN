#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "svn_config.h"
#include "svn_pools.h"
#include "svn_client.h"

static int elapse_time();

int main( int argc, char **argv )
{
    if( argc < 4 )
    {
        std::cout << "Usage: " << argv[0] << " <path-to-wc-file> <num-calls> <recurse>" << std::endl;
        std::cout << "       " << argv[0] << " readme.txt 50 0" << std::endl;
        std::cout << "       " << argv[0] << " ../Sources 1 1" << std::endl;
        return 1;
    }
    apr_initialize();
    apr_pool_initialize();

    apr_pool_t *m_pool;

    apr_pool_create( &m_pool, NULL );

    svn_config_ensure( "", m_pool );

    svn_client_ctx_t    m_context;
    memset( &m_context, 0, sizeof( m_context ) );

    // get the config based on the config dir passed in
    svn_config_get_config( &m_context.config, "", m_pool );

    apr_pool_t *pool = svn_pool_create( NULL );

    svn_opt_revision_t revision;
    revision.kind = svn_opt_revision_working;


    char *path = argv[1];

    svn_boolean_t recurse = atoi( argv[3] );

    int t0 = elapse_time();

    int max_calls = atoi( argv[2] );
    int i;
    for( i=0; i<max_calls; i++ )
    {
        apr_array_header_t *props = NULL;

        svn_error_t *error = svn_client_proplist
            (
            &props,
            path,
            &revision,
            recurse,
            &m_context,
            pool
            );
    }

    int t1 = elapse_time();

    int total = t1 - t0;

    std::cout << "Time for " << max_calls << " calls " << total << "ms recurse=" << recurse << std::endl;
    std::cout << "Time for " << 1 << " call " << total/max_calls << "ms recurse=" << recurse << std::endl;

    return 0;
}


#ifdef WIN32
#include <windows.h>

static int elapse_time()
{
    return GetTickCount();
}

#else
#include <sys/time.h>

static bool need_init_elapse_time = true;
static struct timeval start_time;

static int elapse_time()
{
    if( need_init_elapse_time )
    {
        gettimeofday( &start_time, NULL );
        start_time.tv_usec = 0;
        need_init_elapse_time = false;
    }
    timeval now;
    gettimeofday( &now, NULL );

    return ((now.tv_sec - start_time.tv_sec)*1000) + (now.tv_usec/1000);
}
#endif
