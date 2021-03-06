// LoaderInfo_as.cpp:  ActionScript "LoaderInfo" class, for Gnash.
//
//   Copyright (C) 2009, 2010 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#include "display/LoaderInfo_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value loaderinfo_complete(const fn_call& fn);
    as_value loaderinfo_httpStatus(const fn_call& fn);
    as_value loaderinfo_init(const fn_call& fn);
    as_value loaderinfo_ioError(const fn_call& fn);
    as_value loaderinfo_open(const fn_call& fn);
    as_value loaderinfo_progress(const fn_call& fn);
    as_value loaderinfo_unload(const fn_call& fn);
    as_value loaderinfo_ctor(const fn_call& fn);
    void attachLoaderInfoInterface(as_object& o);
    void attachLoaderInfoStaticInterface(as_object& o);
    as_object* getLoaderInfoInterface();

}

// extern (used by Global.cpp)
void
loaderinfo_class_init(as_object& where, const ObjectURI& uri)
{
    Global_as& gl = getGlobal(where);
    as_object* proto = gl.createObject();
    as_object* cl = gl.createClass(&loaderinfo_ctor, proto);
    attachLoaderInfoInterface(*proto);
    attachLoaderInfoStaticInterface(*cl);

    // Register _global.LoaderInfo
    where.init_member(uri, cl, as_object::DefaultFlags);
}

namespace {

void
attachLoaderInfoInterface(as_object& o)
{
    Global_as& gl = getGlobal(o);
    o.init_member("complete", gl.createFunction(loaderinfo_complete));
    o.init_member("httpStatus", gl.createFunction(loaderinfo_httpStatus));
    o.init_member("init", gl.createFunction(loaderinfo_init));
    o.init_member("ioError", gl.createFunction(loaderinfo_ioError));
    o.init_member("open", gl.createFunction(loaderinfo_open));
    o.init_member("progress", gl.createFunction(loaderinfo_progress));
    o.init_member("unload", gl.createFunction(loaderinfo_unload));
}

void
attachLoaderInfoStaticInterface(as_object& /*o*/)
{
}

as_value
loaderinfo_complete(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
loaderinfo_httpStatus(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
loaderinfo_init(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
loaderinfo_ioError(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
loaderinfo_open(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
loaderinfo_progress(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
loaderinfo_unload(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
loaderinfo_ctor(const fn_call& /*fn*/)
{
    return as_value(); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

