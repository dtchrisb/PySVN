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
//  pysvn_static_strings.cpp
//
#if defined( _MSC_VER )
// disable warning C4786: symbol greater than 255 character,
// nessesary to ignore as <map> causes lots of warning
#pragma warning(disable: 4786)
#endif

#include "pysvn.hpp"

#undef PYSVN_STATIC_PY_STRINGS_HPP

#define PYSVN_STATIC_STRING( name, value ) const char name[] = value;
#define PYSVN_STATIC_PY_STRING_P( name ) Py::String *name;
#include "pysvn_static_strings.hpp"
