// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

#ifndef GNASH_RENDER_HANDLER_OGL_H
#define GNASH_RENDER_HANDLER_OGL_H


#if defined(NOT_SGI_GL) || defined(__APPLE_CC__)
# ifdef __APPLE_CC__
# include <AGL/agl.h>
# endif
#include <vector>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
# if defined(__APPLE_CC__) && (__APPLE_CC__ >= 5465)
# define GLUCALLBACKTYPE GLvoid (*)()
# else
# define GLUCALLBACKTYPE GLvoid (*)(...)
# endif
#else
# define GLUCALLBACKTYPE void (*)()
# include <GL/gl.h>
# ifdef WIN32
#  define GL_CLAMP_TO_EDGE 0x812F
# else
# include <GL/glx.h>
# ifdef OSMESA_TESTING
#  include <GL/osmesa.h>
# endif // OSMESA_TESTING
# endif
# include <GL/glu.h>
# ifndef APIENTRY
#  define APIENTRY
# endif
#endif

#include "Renderer.h"
#include "Geometry.h"
#include "BitmapInfo.h"

#include <map>


namespace gnash {




typedef std::vector<const Path*> PathRefs;



struct oglVertex {
  oglVertex(double x, double y, double z = 0.0)
    : _x(x), _y(y), _z(z)
  {
  }
  
  oglVertex(const point& p)
    : _x(p.x), _y(p.y), _z(0.0)
  {
  }

  GLdouble _x;
  GLdouble _y;
  GLdouble _z;
};

typedef std::map<const Path*, std::vector<oglVertex> > PathPointMap;

class Tesselator
{
public:
  Tesselator();  
  ~Tesselator();
  
  void beginPolygon();
  
  void feed(std::vector<oglVertex>& vertices);
  
  void tesselate();
  
  void beginContour();
  void endContour();
  
  void rememberVertex(GLdouble* v);
  
  static void
  error(GLenum error);

  static void combine(GLdouble coords [3], void *vertex_data[4],
                      GLfloat weight[4], void **outData, void* userdata);
  

  
private:
  std::vector<GLdouble*> _vertices;
  GLUtesselator* _tessobj;
};

class WholeShape
{
public:
  void newPath(const Path& new_path)
  {
    PathRefs refs;
    refs.push_back(&new_path);
    
    shape.push_back(refs);
  }
  
  void addPath(const Path& add_path)
  {
    PathRefs& refs = shape.back();
    refs.push_back(&add_path);
  }
  
  void addPathRefs(const PathRefs& pathrefs)
  {
  
    PathRefs new_refs(pathrefs.begin(), pathrefs.end());
    
    shape.push_back(new_refs);
  }
  
  
  const std::vector<PathRefs>& get() const
  {
    return shape;
  }
  
private:
  std::vector<PathRefs> shape;

};


class bitmap_info_ogl : public BitmapInfo
{
  public:
  
    /// Set line and fill styles for mesh & line_strip rendering.
    enum bitmap_wrap_mode
    {
      WRAP_REPEAT,
      WRAP_CLAMP
    };
    
    bitmap_info_ogl(GnashImage* image, GLenum pixelformat,
                    bool ogl_accessible);
    ~bitmap_info_ogl();

    void apply(const gnash::SWFMatrix& bitmap_matrix,
               bitmap_wrap_mode wrap_mode) const;
  private:
    inline bool ogl_accessible() const;
    void setup() const;    
    void upload(boost::uint8_t* data, size_t width, size_t height) const;
    
    mutable std::auto_ptr<GnashImage> _img;
    GLenum _pixel_format;
    GLenum _ogl_img_type;
    mutable bool _ogl_accessible;  
    mutable GLuint _texture_id;
    size_t _orig_width;
    size_t _orig_height;
};


DSOEXPORT Renderer* create_Renderer_ogl(bool init = true);



} // namespace gnash


#endif // __RENDER_HANDLER_OGL_H__

