// rTriangle.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_rTriangle_H
#define DDImage_rTriangle_H

#include "rPrimitive.h"
#include "VertexContext.h"
#include "SubdivisionContext.h"
#include "CameraOp.h"

namespace DD
{
  namespace Image
  {

    /*!
       Renderable triangle class, created once a projection into screen
       space is known. The Triangle primitive class creates one of these,
       and add_to_render() may then change this to 0,1, or 2 renderable
       triangles (depending on clipping). Other primitives such as grid
       will create quite a few more of these.
     */
    class DDImage_API rTriangle : public rPrimitive
    {

      /*! Split a triangle into two.  A new rTriangle is created by interpolating
          the intersection points of the two edges that penetrate the plane, and
          one of the vertices of the original triangle is moved. New triangle
          is added to scene, caller should add original triangle.
       */
      void split_at_near_plane(Scene*, PrimitiveContext *, int vout, int vin1, int vin2);

      /*!
         Do some preliminary calculation of the derivatives in screen space,
         and calculate the screen bounding box.

         The bounding box is used by the renderer to avoid calling
         the intersect_scanline function for much of the image.
         The derivative partial calculations are used by intersect_scanline
         to produce the final derivatives.
       */
      bool add(Scene*, PrimitiveContext*);
      bool add_displacement(Scene*, PrimitiveContext*, float);
      /*! Recursively subdivide the triangle while applying a distortion function. */
      void distort_recursive(int count, CameraOp::LensNFunc* lensfunc,
                             Scene* scene, PrimitiveContext*);

    public:

      /* It's used to mark if the edge of rTringle is also an edge of the original polygon Primitive */
      enum EdgeMask {
        eMaskEdge01  = 1,
        eMaskEdge12  = 2,
        eMaskEdge20  = 4,
        eMaskEdgeAll = 7
      };

      VArray v[3];              //!< in SCREEN coordiantes, w_normalized!!!
      float Ax, Ay, Bx, By, Nz; //!< For figuring out derivatives
      uint8_t edgesmask;        //!< triangle edge mask
      rTriangle* polygon;       //!< cirular list of all triangles that define the original polygon

    public:
      const char* Class() { return "rTriangle"; }

      /*! Constructor. */
      rTriangle(const GeoInfo * info, const Primitive * p);

      /*! Copy constructor. */
      rTriangle(const rTriangle &);

      /*virtual*/ rPrimitive* duplicate() const;

      /*! Transform this triangle into homogeneous clipping space.  Add
          any sub-triangles to the rendering scene.
          There's no need to further break a triangle down unless it
          intersects one of the frustum planes.
          Lastly we apply the homogeneous division by W which applies perspective
          to the points and places them into NDC space(Normalized Device Coordinates).
          This must be done AFTER clipping to guarantee that W is always > 0.
       */
      /*virtual*/ bool add_to_render(Scene*, PrimitiveContext*);

      /*! Transform this triangle into homogeneous clipping space.  
          And add to the displacement map list in scene
      */
      /*virtual*/ bool add_to_displacement_render(Scene*, PrimitiveContext*);

      /*! Intersect the screen-space projected triangle with an XY point,
          returning true if successful.
       */
      /*virtual*/ bool intersect(float x, float y) const;

      /*! Intersect the screen-space projected triangle with an XY point.
          Return true it was successful, and assigns the distance to the
          nearest edge.
       */
      /*virtual*/ bool intersect(float x, float y, float* dist) const;

      /*! Intersect the screen-space projected triangle with an XY point.
          Return true it was successful, and assigns the square of the
          distance to the nearest edge.
       */
      /*virtual*/ bool intersect_dist_squared(float x, float y, float* dist_squared) const;

      /*! Intersect the triangle with a scanline.  Return whether it was
          successful, and set \a X and \a R to the left and rightmost
          pixels affected, and set out to the value at 0,y (ie pixel 0
          of the scanline, even if outside the triangle), and set du/dv
          to the screen-space derivatives, such that out+x*du is the
          value at any pixel in the scanline.
       */
      /*virtual*/ bool intersect_scanline(float y, int& X, int& R, VArray& out, VArray& du, VArray& dv) const;

      /*! Not implemented yet.
       */
      /*virtual*/ float trace(Scene*);

      /*!
         Distance to the nearest edge the object in screen-space.
         Returns 0 or negative if inside the object.
       */
      /*virtual*/ float distance(float x, float y) const;

      /*!
         Square of the distance to the nearest edge the object in screen-space.
         Returns 0 or negative if inside the object.
       */
      /*virtual*/ float distance_squared(float x, float y) const;

      /*virtual*/ void subdivide(std::vector<rPrimitive*>& primitives, SubdivisionContext* stx);
      /*virtual*/ rPrimitiveType getPrimitiveType() const { return eRTriangle; }

      /* ! 
        Distance to the nearest not occluded polygon edge in screen-space.
        All triangles generated from the tessellation of the original Primitive
        are tested, the method check all polygon edges. Return immediately 
        when an edge distance is under mindist
      */
      float polygonEdgeDistance(const Vector4& P, const float mindist) const;

    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
