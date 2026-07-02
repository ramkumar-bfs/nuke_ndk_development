// Copyright (c) 2021 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef SLR_PRIMINFO_H
#define SLR_PRIMINFO_H

#include "DDImage/DDImage_API.h"

#include "fdk/math/Box3.h"
#include "fdk/math/Mat4.h"
#include "fdk/math/types.h" // for Mat4dList

#include "usg/geom/Stage.h"

#include <vector>

namespace slr
{
  class SlrSurfaceMaterial;

  //------------------------------------------------------------------

  // To help to map USD Prim types to render primitive adapters:
  enum SlrPrimTypeHint
  {
    eUnknownSlrPrimitive = -1,
    //
    eSlrSubdMesh,         //!< usg::MeshPrim with subdivision
    eSlrPolyMesh,         //!< usg::MeshPrim no subdivision
    //
    eSlrPoints,           //!< usg::PointsPrim with point rendering style (disk, square, etc)
    eSlrParticles,        //!< usg::PointsPrim with particle-style rendering (spheres, trails, discs, etc)
    eSlrParticleCards,    //!< usg::PointsPrim with 'sprite' textured-card rendering
  };

  //------------------------------------------------------------------


  /*! Replacement for legacy GeoInfo structure used in the legacy DD::Image::Render-based
      architecture like ScanlineRender, DepthMapRender, etc.

      This structure represents a single usg::Prim at a single time sample. The Prim's
      attributes are not copied to time-sampled local arrays, that is left to the render
      primitive adapters at render time.
      However the Prim's concatenated scene transform is time sampled and stored in
      the 'transforms' - note that all matrices are now double-precision.

      The previous GeoInfo often contained an internal list of 'atomic' Primitives such
      as triangles, points and polygons, but these are deprecated in favor of the SlrPrimInfo
      representing a single 'compound', or complex primitive such as a MeshPrim or PointsPrim,
      so there's no longer a reason for SlrPrimInfo to contain anything besides what's needed
      for the renderer to sample the usg::Prim and convert it to its internal render
      primitives, which is usually triangles.
  */
  class DDImage_API SlrPrimInfo
  {
  public:
    // Global values shared between all time-sampled copies of this SlrPrimInfo:
    uint32_t              object_index;       //!< Index of this object in the SlrPrimInfoList in SlrRenderContext
    usg::StageRef         stage;              //!< Stage containing Prim
    const usg::Path*      prim_path;          //!< Prim path in Stage (stored in Render prim_path_cache)
    SlrPrimTypeHint       prim_type_hint;     //!< To drive render primitive conversion
    SlrSurfaceMaterial*   material;           //!< Render material (stored in SlrRenderContext)
    //
    bool                  visible_to_camera;  //!< Should the object be visible to camera?
    bool                  casts_shadow;       //!< Does the object cast a shadow?
    bool                  receives_shadow;    //!< Does the object get shadowed by lights?

    // Time-sampled values updated during renderer setup:
    bool                  enabled;            //!< Object is 'on' for this scene
    fdk::TimeValue        motion_time;        //!< Time of this prim's xform & bbox sampling (scene motion time)
    fdk::TimeValue        topology_time;      //!< Time of this prim's topology sampling (usually shutter open or close)
    fdk::Box3f            bbox;               //!< Local-space bbox
    fdk::Mat4dList        transforms;         //!< Double-precision scene-transform matrices
    int32_t               num_sub_indices;    //!< To match motionblur-able topology. ie nFaces for MeshPrim, nPoints for PointsPrim.


    SlrPrimInfo() :
      object_index(0),
      prim_path(nullptr),
      prim_type_hint(eUnknownSlrPrimitive),
      material(nullptr),
      //
      visible_to_camera(true),
      casts_shadow(true),
      receives_shadow(true),
      //
      enabled(true),
      motion_time(fdk::defaultTimeValue()),
      topology_time(fdk::defaultTimeValue()),
      num_sub_indices(0)
    {
      //
    }

    //! For MeshPrim types. If -1 the faces count has not been determined yet!
    int32_t nFaces() const { return num_sub_indices; }

    //! For PointsPrim, PointInstancer types. If -1 the point count has not been determined yet!
    int32_t nPoints() const { return num_sub_indices; }


    friend DDImage_API std::ostream& operator << (std::ostream&, const SlrPrimInfo&);
  };

  using SlrPrimInfoList = std::vector<SlrPrimInfo>;


} // namespace slr

#endif
