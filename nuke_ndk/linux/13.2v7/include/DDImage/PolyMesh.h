// PolyMesh.h

#ifndef DDImage_PolyMesh_H
#define DDImage_PolyMesh_H

#include "GeoInfo.h"
#include "PolygonPrimitive.h"


namespace DD 
{ 
  namespace Image 
  {
  
    /// PolyMesh is a connected series of polygons comprising a contiguous surface.
    class DDImage_API PolyMesh : public PolygonPrimitive 
    {
      struct SubFace
      {
        // number of vertices in the subface
        unsigned numVertex;

        // starting vertex index in Primitive::vertex_ array
        unsigned startVertex;
      };

      // sub faces array
      std::vector<SubFace, STL3DAllocator<SubFace> > _subFaces;
      
    public:

      PolyMesh();
      PolyMesh(unsigned icount, unsigned fcount);

      ~PolyMesh();

      /*virtual*/
      const char* Class() const { return "PolyMesh"; }

      /*virtual*/
      Primitive* duplicate() const { return new PolyMesh(*this); }
   
      /*virtual*/ 
      PrimitiveType getPrimitiveType() const { return ePolyMesh; };

      /*! Add a face to the mesh, return the face index of the new face or -1 if failure. */
      int add_face(unsigned nVerts, const int* vertIndices,  bool reverse = false);

      /*virtual*/
      unsigned faces() const;

      /*virtual*/
      Vector3 face_average_center(int n, const PointList*) const;

      /*virtual*/
      unsigned face_vertices(int n) const;

      /*virtual*/
      void get_face_vertices(int n, unsigned* array) const;

      /*virtual*/
      Vector3 get_face_normal(int n, const PointList* points) const;

      /*virtual*/
      void validate(PrimitiveContext*);
      
      /*virtual*/
      void draw_wireframe(ViewerContext*,
                          PrimitiveContext*,
                          Primitive* prev_prim=nullptr) const;
      /*virtual*/
      void draw_solid(ViewerContext*,
                      PrimitiveContext*,
                      Primitive* prev_prim=nullptr) const;

      /*virtual*/
      void draw_primitive_normal(ViewerContext*, PrimitiveContext*) const;

      /*virtual*/
      void draw_primitive_num(ViewerContext*, PrimitiveContext*) const;

      /*virtual*/
      void draw_vertex_num(ViewerContext*, PrimitiveContext*) const;

      /* virtual */
      void draw_solid_face(int n, ViewerContext* ctx, PrimitiveContext* ptx) const;

      /*virtual*/
      void print_info() const;

      /* virtual */
      bool IntersectsRay(const Ray &ray, int n, const PointList* pointList, CollisionResult *result) const
      {
        return Primitive::PolyIntersectsRay(ray, n, pointList, result);
      }
    };

  } 
} // namespace DD::Image

#endif

// end of PolyMesh.h
