// ClipboardTransferMode_as.cpp:  ActionScript "ClipboardTransferMode" class, for Gnash.
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

#include "desktop/ClipboardTransferMode_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function

// Added to the Flash Player 10 API from AIR.

namespace gnash {

// Forward declarations
namespace {
    void attachClipboardTransferModeStaticInterface(as_object& o);
}

// extern (used by Global.cpp)
void
clipboardtransfermode_class_init(as_object& where, const ObjectURI& uri)
{
    registerBuiltinObject(where, attachClipboardTransferModeStaticInterface, 
            uri);
}

namespace {

void
attachClipboardTransferModeStaticInterface(as_object& o)
{
    o.init_member("CLONE_ONLY", "cloneOnly");
    o.init_member("CLONE_PREFERRED", "clonePreferred");
    o.init_member("ORIGINAL_ONLY", "originalOnly");
    o.init_member("ORIGINAL_PREFERRED", "originalPreferred");
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

