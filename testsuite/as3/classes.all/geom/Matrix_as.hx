// Matrix_as.hx:  ActionScript 3 "Matrix" class, for Gnash.
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
import flash.geom.Matrix;
import flash.display.MovieClip;
#else
import flash.Matrix;
import flash.MovieClip;
#end
import flash.Lib;
import Type;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class Matrix_as {
    static function main() {
        var x1:Matrix = new Matrix();

        // Make sure we actually get a valid class        
        if (x1 != null) {
            DejaGnu.pass("Matrix class exists");
        } else {
            DejaGnu.fail("Matrix class doesn't exist");
        }
// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
	if (x1.a == 0) {
	    DejaGnu.pass("Matrix.a property exists");
	} else {
	    DejaGnu.fail("Matrix.a property doesn't exist");
	}
	if (x1.b == 0) {
	    DejaGnu.pass("Matrix.b property exists");
	} else {
	    DejaGnu.fail("Matrix.b property doesn't exist");
	}
	if (x1.c == 0) {
	    DejaGnu.pass("Matrix.c property exists");
	} else {
	    DejaGnu.fail("Matrix.c property doesn't exist");
	}
	if (x1.d == 0) {
	    DejaGnu.pass("Matrix.d property exists");
	} else {
	    DejaGnu.fail("Matrix.d property doesn't exist");
	}
	if (x1.tx == 0) {
	    DejaGnu.pass("Matrix.tx property exists");
	} else {
	    DejaGnu.fail("Matrix.tx property doesn't exist");
	}
	if (x1.ty == 0) {
	    DejaGnu.pass("Matrix.ty property exists");
	} else {
	    DejaGnu.fail("Matrix.ty property doesn't exist");
	}

// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
// 	if (x1.clone == Matrix) {
// 	    DejaGnu.pass("Matrix::clone() method exists");
// 	} else {
// 	    DejaGnu.fail("Matrix::clone() method doesn't exist");
// 	}
	if (x1.concat == null) {
	    DejaGnu.pass("Matrix::concat() method exists");
	} else {
	    DejaGnu.fail("Matrix::concat() method doesn't exist");
	}
	if (x1.createBox == null) {
	    DejaGnu.pass("Matrix::createBox() method exists");
	} else {
	    DejaGnu.fail("Matrix::createBox() method doesn't exist");
	}
	if (x1.createGradientBox == null) {
	    DejaGnu.pass("Matrix::createGradientBox() method exists");
	} else {
	    DejaGnu.fail("Matrix::createGradientBox() method doesn't exist");
	}
// 	if (x1.deltaTransformPoint == Point) {
// 	    DejaGnu.pass("Matrix::deltaTransformPoint() method exists");
// 	} else {
// 	    DejaGnu.fail("Matrix::deltaTransformPoint() method doesn't exist");
// 	}
	if (x1.identity == null) {
	    DejaGnu.pass("Matrix::identity() method exists");
	} else {
	    DejaGnu.fail("Matrix::identity() method doesn't exist");
	}
	if (x1.invert == null) {
	    DejaGnu.pass("Matrix::invert() method exists");
	} else {
	    DejaGnu.fail("Matrix::invert() method doesn't exist");
	}
	if (x1.rotate == null) {
	    DejaGnu.pass("Matrix::rotate() method exists");
	} else {
	    DejaGnu.fail("Matrix::rotate() method doesn't exist");
	}
	if (x1.scale == null) {
	    DejaGnu.pass("Matrix::scale() method exists");
	} else {
	    DejaGnu.fail("Matrix::scale() method doesn't exist");
	}
	if (x1.toString == null) {
	    DejaGnu.pass("Matrix::toString() method exists");
	} else {
	    DejaGnu.fail("Matrix::toString() method doesn't exist");
	}
// 	if (x1.transformPoint == Point) {
// 	    DejaGnu.pass("Matrix::transformPoint() method exists");
// 	} else {
// 	    DejaGnu.fail("Matrix::transformPoint() method doesn't exist");
// 	}
	if (x1.translate == null) {
	    DejaGnu.pass("Matrix::translate() method exists");
	} else {
	    DejaGnu.fail("Matrix::translate() method doesn't exist");
	}

        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
