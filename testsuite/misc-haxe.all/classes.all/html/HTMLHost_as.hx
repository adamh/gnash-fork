// HTMLHost_as.hx:  ActionScript 3 "HTMLHost" class, for Gnash.
//
// Generated by gen-as3.sh on: 20090515 by "rob". Remove this
// after any hand editing loosing changes.
//
//   Copyright (C) 2009 Free Software Foundation, Inc.
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

// This test case must be processed by CPP before compiling to include the
//  DejaGnu.hx header file for the testing framework support.

#if flash9
import flash.html.HTMLHost;
import flash.display.MovieClip;
#else
import flash.HTMLHost;
import flash.MovieClip;
#end
import flash.Lib;
import Type;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class HTMLHost_as {
    static function main() {
        var x1:HTMLHost = new HTMLHost();

        // Make sure we actually get a valid class        
        if (x1 != null) {
            DejaGnu.pass("HTMLHost class exists");
        } else {
            DejaGnu.fail("HTMLHost class doesn't exist");
        }
// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
	if (x1.htmlLoader == HTMLLoader) {
	    DejaGnu.pass("HTMLHost.htmlLoader property exists");
	} else {
	    DejaGnu.fail("HTMLHost.htmlLoader property doesn't exist");
	}
	if (x1.windowRect == Rectangle) {
	    DejaGnu.pass("HTMLHost.windowRect property exists");
	} else {
	    DejaGnu.fail("HTMLHost.windowRect property doesn't exist");
	}

// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
	if (x1.HTMLHost == false) {
	    DejaGnu.pass("HTMLHost::HTMLHost() method exists");
	} else {
	    DejaGnu.fail("HTMLHost::HTMLHost() method doesn't exist");
	}
	if (x1.createWindow == HTMLLoader) {
	    DejaGnu.pass("HTMLHost::createWindow() method exists");
	} else {
	    DejaGnu.fail("HTMLHost::createWindow() method doesn't exist");
	}
	if (x1.updateLocation == null) {
	    DejaGnu.pass("HTMLHost::updateLocation() method exists");
	} else {
	    DejaGnu.fail("HTMLHost::updateLocation() method doesn't exist");
	}
	if (x1.updateStatus == null) {
	    DejaGnu.pass("HTMLHost::updateStatus() method exists");
	} else {
	    DejaGnu.fail("HTMLHost::updateStatus() method doesn't exist");
	}
	if (x1.updateTitle == null) {
	    DejaGnu.pass("HTMLHost::updateTitle() method exists");
	} else {
	    DejaGnu.fail("HTMLHost::updateTitle() method doesn't exist");
	}
	if (x1.windowBlur == null) {
	    DejaGnu.pass("HTMLHost::windowBlur() method exists");
	} else {
	    DejaGnu.fail("HTMLHost::windowBlur() method doesn't exist");
	}
	if (x1.windowClose == null) {
	    DejaGnu.pass("HTMLHost::windowClose() method exists");
	} else {
	    DejaGnu.fail("HTMLHost::windowClose() method doesn't exist");
	}
	if (x1.windowFocus == null) {
	    DejaGnu.pass("HTMLHost::windowFocus() method exists");
	} else {
	    DejaGnu.fail("HTMLHost::windowFocus() method doesn't exist");
	}

        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
