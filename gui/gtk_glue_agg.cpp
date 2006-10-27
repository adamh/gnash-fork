//
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
//
//

/* $Id: gtk_glue_agg.cpp,v 1.9 2006/10/27 10:16:00 bik Exp $ */

#include <cstdio>
#include <cerrno>
#include <cstring>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "gnash.h"
#include "log.h"
#include "render_handler.h"
#include "render_handler_agg.h"
#include "gtk_glue_agg.h"

namespace gnash
{

GtkAggGlue::GtkAggGlue() :
	_offscreenbuf(NULL),
	_offscreenbuf_size(0),
	_agg_renderer(NULL),
	_width(0),
	_height(0),
	_bpp(0)
{
}

GtkAggGlue::~GtkAggGlue()
{
  free(_offscreenbuf);
}

bool
GtkAggGlue::init(int /*argc*/, char **/*argv*/[])
{
    gdk_rgb_init();

		// GDK's gdk_draw_rgb_image() needs 24-bit RGB data, so we initialize the
		// AGG renderer with RGB24 and let GTK take care of the proper pixel format.
		_bpp = 24;

		return true;
}

void
GtkAggGlue::prepDrawingArea(GtkWidget *drawing_area)
{
    _drawing_area = drawing_area;
}

render_handler*
GtkAggGlue::createRenderHandler()
{
		_agg_renderer = create_render_handler_agg("RGB24");
    return _agg_renderer;
}

void
GtkAggGlue::setRenderHandlerSize(int width, int height)
{
	assert(width>0);
	assert(height>0);
	assert(_agg_renderer!=NULL);

	#define CHUNK_SIZE (100*100*(_bpp/8))

	if (width == _width && height == _height)
	   return;
	   
	int new_bufsize = width*height*(_bpp/8);
	
	// TODO: At the moment we only increase the buffer and never decrease it. Should be
	// changed sometime.
	if (new_bufsize > _offscreenbuf_size) {
		new_bufsize = static_cast<int>(new_bufsize / CHUNK_SIZE + 1) * CHUNK_SIZE;
		// TODO: C++ conform alternative to realloc?
		_offscreenbuf	= static_cast<unsigned char *>( realloc(_offscreenbuf, new_bufsize) );

		if (!_offscreenbuf) {
		  log_msg("Could not allocate %i bytes for offscreen buffer: %s\n",
				new_bufsize, strerror(errno)
			);
			return;
		}

		log_msg("GTK-AGG: %i bytes offscreen buffer allocated\n", new_bufsize);

		_offscreenbuf_size = new_bufsize;
		memset(_offscreenbuf, 0, new_bufsize);
	}

  _width = width;
	_height = height;

	// Only the AGG renderer has the function init_buffer, which is *not* part of
	// the renderer api. It allows us to change the renderers movie size (and buffer
	// address) during run-time.
	static_cast<render_handler_agg_base *>(_agg_renderer)->init_buffer(
	  _offscreenbuf,
		_offscreenbuf_size,
		_width,
		_height
	);
	
	// Note: At this point the buffer is empty and would show a black screen.
	// We have to tell the renderer to render the entire frame and not just the
	// invalidated region. As soon as the renderer supports that we can implement
	// it here.
}

static int
valid_coord(int coord, int max)
{ 
    if (coord<0) return 0;
    else if (coord>=max) return max;

    return coord;
}


void
GtkAggGlue::render()
{
	// Update the entire screen
	gdk_draw_rgb_image (
		_drawing_area->window,
		_drawing_area->style->fg_gc[GTK_STATE_NORMAL],
		0,
		0,
		_width,
		_height,
		GDK_RGB_DITHER_NONE,
		_offscreenbuf,
		(int)(_width*_bpp/8)
	);
}

void
GtkAggGlue::render(int minx, int miny, int maxx, int maxy)
{

	// Update only the invalidated rectangle
	gdk_draw_rgb_image (
		_drawing_area->window,
		_drawing_area->style->fg_gc[GTK_STATE_NORMAL],
		minx,
  	miny,
		maxx-minx,
		maxy-miny,
		GDK_RGB_DITHER_NORMAL,
		_offscreenbuf + miny*(_width*(_bpp/8)) + minx*(_bpp/8),
		(int)((_width)*_bpp/8)
	);
}

void
GtkAggGlue::configure(GtkWidget *const /*widget*/, GdkEventConfigure *const event)
{
	if (_agg_renderer)
		setRenderHandlerSize(event->width, event->height);
}



} // namespace gnash

