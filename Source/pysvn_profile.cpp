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
//  pysvn_profile.cpp
//
//  Functions to help profile performance of the pysvn code
//
#ifdef WIN32
#include <windows.h>

int elapse_time()
{
    return GetTickCount();
}

#endif
