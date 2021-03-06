// MouseEvent_as.cpp:  ActionScript "MouseEvent" class, for Gnash.
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


#include "events/MouseEvent_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value mouseevent_toString(const fn_call& fn);
    as_value mouseevent_updateAfterEvent(const fn_call& fn);
    as_value mouseevent_CLICK(const fn_call& fn);
    as_value mouseevent_DOUBLE_CLICK(const fn_call& fn);
    as_value mouseevent_MOUSE_DOWN(const fn_call& fn);
    as_value mouseevent_MOUSE_MOVE(const fn_call& fn);
    as_value mouseevent_MOUSE_OUT(const fn_call& fn);
    as_value mouseevent_MOUSE_OVER(const fn_call& fn);
    as_value mouseevent_MOUSE_UP(const fn_call& fn);
    as_value mouseevent_MOUSE_WHEEL(const fn_call& fn);
    as_value mouseevent_ROLL_OUT(const fn_call& fn);
    as_value mouseevent_ROLL_OVER(const fn_call& fn);
    as_value mouseevent_ctor(const fn_call& fn);
    void attachMouseEventInterface(as_object& o);
    void attachMouseEventStaticInterface(as_object& o);
}

// extern (used by Global.cpp)
void
mouseevent_class_init(as_object& where, const ObjectURI& uri)
{
    registerBuiltinClass(where, mouseevent_ctor,
            attachMouseEventInterface,
            attachMouseEventStaticInterface, uri);
}

namespace {

void
attachMouseEventInterface(as_object& o)
{
    Global_as& gl = getGlobal(o);
    o.init_member("toString", gl.createFunction(mouseevent_toString));
    o.init_member("updateAfterEvent", gl.createFunction(mouseevent_updateAfterEvent));
    o.init_member("CLICK", gl.createFunction(mouseevent_CLICK));
    o.init_member("DOUBLE_CLICK", gl.createFunction(mouseevent_DOUBLE_CLICK));
    o.init_member("MOUSE_DOWN", gl.createFunction(mouseevent_MOUSE_DOWN));
    o.init_member("MOUSE_MOVE", gl.createFunction(mouseevent_MOUSE_MOVE));
    o.init_member("MOUSE_OUT", gl.createFunction(mouseevent_MOUSE_OUT));
    o.init_member("MOUSE_OVER", gl.createFunction(mouseevent_MOUSE_OVER));
    o.init_member("MOUSE_UP", gl.createFunction(mouseevent_MOUSE_UP));
    o.init_member("MOUSE_WHEEL", gl.createFunction(mouseevent_MOUSE_WHEEL));
    o.init_member("ROLL_OUT", gl.createFunction(mouseevent_ROLL_OUT));
    o.init_member("ROLL_OVER", gl.createFunction(mouseevent_ROLL_OVER));
}

void
attachMouseEventStaticInterface(as_object& /*o*/)
{

}

as_value
mouseevent_toString(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
mouseevent_updateAfterEvent(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
mouseevent_CLICK(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
mouseevent_DOUBLE_CLICK(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
mouseevent_MOUSE_DOWN(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
mouseevent_MOUSE_MOVE(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
mouseevent_MOUSE_OUT(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
mouseevent_MOUSE_OVER(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
mouseevent_MOUSE_UP(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
mouseevent_MOUSE_WHEEL(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
mouseevent_ROLL_OUT(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
mouseevent_ROLL_OVER(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
mouseevent_ctor(const fn_call& /*fn*/)
{
    return as_value();
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

