// Triangle.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Triangle_H
#define DDImage_Triangle_H

#include "PolygonPrimitive.h"

namespace DD
{
  namespace Image
  {

    /*! Simple triangle primitive.
     */
    class DDImage_API Triangle : public PolygonPrimitive
    {
    protected:

      /*! Destructive copy. */
      void copy(const Triangle*);

    public:
      /* virtual */ const char* Class() const;

      /*! Constructor accepting 3 points. */
      Triangle(int p0 = 0, int p1 = 1, int p2 = 2);

      /*! Copy constuctor. */
      Triangle(const Triangle &);

      Triangle& operator = (const Triangle& b) { copy(&b);
                                                 return *this; }
      /*! Copy this Triangle into a new one, and return a pointer to it.
          Vertex data is duplicated, point indices are unchanged.
       */
      /*virtual*/ Primitive* duplicate() const;

      //! Returns the xyz center average of the triangle.
      /*! Return the primitive's xyz local-space center. */
      /*virtual*/ Vector3 average_center(const PointList*) const;

      //! Test for the intersection of this primitive with a given ray.
      /*! If a collision is detected, information is returned in the collision result
      */
      /*virtual*/ bool IntersectsRay(const Ray &ray, int n, const PointList* pointList, CollisionResult *result) const 
      {
        return Primitive::PolyIntersectsRay(ray, n, pointList, result);
      }

      /*! Calculate the surface normal - points out with a ccw winding.
       */
      /*virtual*/ void validate(PrimitiveContext*);

      /*virtual*/ unsigned faces() const { return 1; }

      /*virtual*/ void draw_wireframe(ViewerContext*, PrimitiveContext*,
                                      Primitive* prev_prim = nullptr) const;
      /*virtual*/ void draw_solid(ViewerContext*, PrimitiveContext*,
                                  Primitive* prev_prim = nullptr) const;

      /*! Draw the single face as a filled triangle. */
      /*virtual*/ void draw_solid_face(int n, ViewerContext*, PrimitiveContext*) const;
      /*virtual*/ void draw_primitive_num(ViewerContext*, PrimitiveContext*) const;
      /*virtual*/ void draw_primitive_normal(ViewerContext*, PrimitiveContext*) const;

      /*virtual*/ void print_info() const;

      /*virtual*/ PrimitiveType getPrimitiveType()  const { return eTriangle;  }
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
