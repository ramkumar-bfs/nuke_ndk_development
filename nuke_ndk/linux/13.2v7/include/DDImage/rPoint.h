// rPoint.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_rPoint_H
#define DDImage_rPoint_H

#include "Point.h"
#include "rPrimitive.h"
#include "VertexContext.h"

namespace DD
{
  namespace Image
  {

    /*! Renderable point primitive.
     */
    class DDImage_API rPoint : public rPrimitive
    {
    public:
      VArray v;
      Point::RenderMode render_mode;
      float radius;
      Vector4 _color;

    public:
      const char* Class() { return "rPoint"; }
      rPoint(const GeoInfo * info, const Primitive * p, Point::RenderMode mode, float r);
      rPoint(const GeoInfo * info, const Primitive * p, int mode, float r);
      rPoint(const GeoInfo * info, const Primitive * p, Point::RenderMode mode, const Vector4 &color, float r);
      rPoint(const GeoInfo * info, const Primitive * p, int mode, const Vector4 &color, float r);
      rPoint(const rPoint &);

      /*! Adds the point to the render if it is beyond the near plane. */
      /*virtual*/ bool add_to_render(Scene*, PrimitiveContext*);
      /*virtual*/ bool intersect(float x, float y) const;
      /*virtual*/ bool intersect(float x, float y, float* dist) const;
      /*virtual*/ bool intersect_dist_squared(float x, float y, float* dist_squared) const;

      /*! Intersect the point with a scanline.  Return whether it was
          successful and the left and right intersection vertices.
       */
      /*virtual*/ bool intersect_scanline(float y, int& X, int& R,
                                          VArray& out, VArray& du, VArray& dv) const;

      /*virtual*/ rPrimitiveType getPrimitiveType() const { return eRPoint; }

      /*! Not implemented yet.
       */
      /*virtual*/ float trace(Scene*);

      /*!
         Distance the given screen-space point is away from the object.
         Returns 0 or negative if inside the object.
       */
      /*virtual*/ float distance(float x, float y) const;

      /*!
         Square of the distance to the nearest edge the object in screen-space.
         Returns 0 or negative if inside the object.
       */
      /*virtual*/ float distance_squared(float x, float y) const;
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
