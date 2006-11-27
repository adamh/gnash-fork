// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License

// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

//

// Test case for Function ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: MovieClip.as,v 1.16 2006/11/27 09:40:42 strk Exp $";

#include "check.as"

// Get a reference to a MovieClip
var mc = _root;
check(typeof(mc)=="movieclip");

// Check some references
check(this != undefined);
check(_parent == undefined);
check(_root != undefined);
check(_root == this);

// Check inheritance
check(MovieClip);
check_equals(mc.__proto__, MovieClip.prototype);

// Check methods existance

// SWF5 or higher
check(mc.attachMovie);
check(mc.getBytesLoaded);
check(mc.getBytesTotal);
xcheck(mc.getBounds);

// This seems unavailable
// when targetting SWF > 6
#if OUTPUT_VERSION > 6
check_equals(mc.duplicateMovieClip, undefined);
#else
check(mc.duplicateMovieClip);
#endif

#if OUTPUT_VERSION >= 6
	xcheck(mc.beginFill);
	xcheck(mc.beginGradientFill);
        xcheck(mc.clear);
	check(mc.createEmptyMovieClip);
	check(mc.createTextField);
	xcheck(mc.curveTo);
	xcheck(mc.lineStyle);
	xcheck(mc.lineTo);
	check(mc.attachAudio);
	xcheck(mc.endFill);
	check(mc.getDepth);
	xcheck(mc.globalToLocal);
	check(mc.getURL);
	check(mc.gotoAndPlay);
	check(mc.gotoAndStop);
	check(mc.hitTest);
	check(mc.nextFrame != undefined);
	check(mc.play != undefined);
	check(mc.prevFrame != undefined);
	check(mc.stop != undefined);
	check(mc.swapDepths != undefined);

	// These two seem unavailable
	// when targetting SWF > 6
#if OUTPUT_VERSION > 6
	check_equals(mc.loadMovie, undefined);
	check_equals(mc.removeMovieClip, undefined);
#else
	check(mc.loadMovie);
	check(mc.removeMovieClip);
#endif

#endif // OUTPUT_VERSION >= 6

#if OUTPUT_VERSION >= 7
    xcheck(mc.getInstanceAtDepth != undefined);
    xcheck(mc.getSWFVersion != undefined);
    xcheck(mc.getTextSnapshot != undefined);

    // can't confirm this works !
    // maybe we should just NOT use the _root for this ?
    //check(mc.loadVariables != undefined);

    xcheck(mc.localToGlobal);
    xcheck(mc.moveTo);
    xcheck(mc.setMask);
    check(mc.startDrag);
    check(mc.stopDrag);
    xcheck(mc.unloadMovie);
    xcheck(mc.enabled);

    // maybe this is the start condition...
    check_equals(mc.focusEnabled, undefined);
    check_equals(mc.hitArea, undefined);
    check_equals(mc.menu, undefined);

    xcheck_equals(mc.getNextHighestDepth(), 0);
#else
    check_equals(mc.getNextHighestDepth(), undefined);
#endif

// Even handlers are initially undefined, user can
// assign them a function to be called on that event...
check_equals(mc.onData, undefined);
check_equals(mc.onDragOut, undefined);
check_equals(mc.onDragOver, undefined);
check_equals(mc.onEnterFrame, undefined);
check_equals(mc.onKeyDown, undefined);
check_equals(mc.onKeyUp, undefined);
check_equals(mc.onKillFocus, undefined);
check_equals(mc.onLoad, undefined);
check_equals(mc.onMouseDown, undefined);
check_equals(mc.onMouseMove, undefined);
check_equals(mc.onMouseUp, undefined);
check_equals(mc.onPress, undefined);
check_equals(mc.onRelease, undefined);
check_equals(mc.onReleaseOutside, undefined);
check_equals(mc.onRollOut, undefined);
check_equals(mc.onRollOver, undefined);
check_equals(mc.onSetFocus, undefined);
check_equals(mc.onUnload, undefined);

// Check property existance

// These are undefined by default
check_equals(mc.tabChildren, undefined);
mc.tabChildren = false;
check_equals(mc.tabChildren, false);
mc.tabChildren = true;
check_equals(mc.tabChildren, true);
check_equals(mc.tabEnabled, undefined);
check_equals(mc.tabIndex, undefined);
check_equals(mc.trackAsMenu, undefined);
xcheck_equals(mc.useHandCursor, true);
check_equals(mc._alpha, 100);
check(mc._currentframe != undefined);
check(mc._droptarget != undefined);
check(mc._focusrect != undefined);
check(mc._framesloaded != undefined);
check(mc._height != undefined);
check(mc._highquality != undefined);
check(mc._y != undefined);
check(mc._ymouse != undefined);
check(mc._yscale != undefined);
xcheck(mc._lockroot != undefined);
check(mc._name != undefined);
check(mc._parent == undefined);
check(mc._rotation != undefined);
check(mc._soundbuftime != undefined);
check(mc._target != undefined);
check(mc._totalframes != undefined);
check(mc._url != undefined);
check(mc._visible != undefined);
check(mc._width != undefined);
check(mc._x != undefined);
check(mc._xmouse != undefined);
check(mc._xscale != undefined);

#if OUTPUT_VERSION >= 6
// Test movieclip creation
var mc2 = createEmptyMovieClip("mc2_mc", 50, 0, 0, 0);
xcheck(mc2 != undefined);
xcheck_equals(mc2_mc.getBytesLoaded(), 0);
xcheck_equals(mc2_mc.getBytesTotal(), 0);
xcheck_equals(mc2.getBytesLoaded(), 0);
xcheck_equals(mc2.getBytesTotal(), 0);

var mc3 = createEmptyMovieClip("mc3_mc", 50);
check(mc3 != undefined);
check_equals(mc3_mc.getBytesLoaded(), 0);
check_equals(mc3_mc.getBytesTotal(), 0);
check_equals(mc3.getBytesLoaded(), 0);
check_equals(mc3.getBytesTotal(), 0);
#endif
