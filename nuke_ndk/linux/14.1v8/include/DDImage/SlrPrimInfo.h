// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef SLR_PRIMINFO_H
#define SLR_PRIMINFO_H

#include "DDImage/DDImage_API.h"

#include "fdk/math/Box3.h"
#include "fdk/math/Mat4.h"
#include "fdk/math/types.h" // for Mat4dList

#include "usg/geom/Path.h"

#include <vector>

namespace slr
{
  class SlrSurfaceMaterial;

  //------------------------------------------------------------------

  // To help to map USD Prim types to render primitive adapters:
  enum SlrPrimTypeAdapter
  {
    eUnknownSlrPrimitive = -1,
    //
    eSlrSubdMesh,         //!< usg::MeshPrim with subdivision
    eSlrPolyMesh,         //!< usg::MeshPrim no subdivision
    //
    eSlrPoints,           //!< usg::PointsPrim with point rendering style (disk, square, etc)
    eSlrParticles,        //!< usg::PointsPrim with particle-style rendering (spheres, trails, discs, etc)
    eSlrParticleCards,    //!< usg::PointsPrim with 'sprite' textured-card rendering
    //
    eSlrPointInstancer,   //!< usg::PointInstancerPrim
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
    uint32_t              objectIndex;        //!< Index of object in renderable_prims in SlrRenderContext
    usg::Path             primPath;           //!< Prim path in Stage (prototype path if an instance)
    SlrPrimTypeAdapter    primAdapter;        //!< To drive render primitive conversion
    int32_t               facesetBaseIndex;   //!< Faceset base prim in renderable_prims in SlrRenderContext
    int32_t               facesetIndex;       //!< Faceset index if this prim is one, otherwise -1
    int32_t               instancerIndex;     //!< Instancer index in instancer_prims in SlrRenderContext
    int32_t               instanceIndex;      //!< Instance index if this prim is one, otherwise -1.
    usg::Path             materialPath;       //!< Material binding path in Stage
    SlrSurfaceMaterial*   material;           //!< Render material (stored in SlrRenderContext)
    //
    bool                  visibleToCamera;    //!< Should the object be visible to camera?
    bool                  castsShadow;        //!< Does the object cast a shadow?
    bool                  receivesShadow;     //!< Does the object get shadowed by lights?


    SlrPrimInfo();

    friend DDImage_API std::ostream& operator << (std::ostream&, const SlrPrimInfo&);


  public:
    //----------------------------------------------------------
    // Scanline shading data - not used when ray tracing!
    // Time-sampled values updated during renderer setup:
    fdk::Mat4dList transforms;  //!< Double-precision scene-transform matrices
  };

  using SlrPrimInfoList = std::vector<SlrPrimInfo>;


} // namespace slr

#endif
