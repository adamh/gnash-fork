// DataEvent_as.cpp:  ActionScript "DataEvent" class, for Gnash.
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


#include "events/DataEvent_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value dataevent_toString(const fn_call& fn);
    as_value dataevent_DATA(const fn_call& fn);
    as_value dataevent_UPLOAD_COMPLETE_DATA(const fn_call& fn);
    as_value dataevent_ctor(const fn_call& fn);
    void attachDataEventInterface(as_object& o);
    void attachDataEventStaticInterface(as_object& o);
    as_object* getDataEventInterface();

}

// extern (used by Global.cpp)
void
dataevent_class_init(as_object& where, const ObjectURI& uri)
{
    registerBuiltinClass(where, dataevent_ctor, attachDataEventInterface, 
        attachDataEventStaticInterface, uri);
}

namespace {

void
attachDataEventInterface(as_object& o)
{
    Global_as& gl = getGlobal(o);
    o.init_member("toString", gl.createFunction(dataevent_toString));
    o.init_member("DATA", gl.createFunction(dataevent_DATA));
    o.init_member("UPLOAD_COMPLETE_DATA", gl.createFunction(dataevent_UPLOAD_COMPLETE_DATA));
}

void
attachDataEventStaticInterface(as_object& /*o*/)
{

}

as_value
dataevent_toString(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
dataevent_DATA(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
dataevent_UPLOAD_COMPLETE_DATA(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
dataevent_ctor(const fn_call& /*fn*/)
{

    return as_value(); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

