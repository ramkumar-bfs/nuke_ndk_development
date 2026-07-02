// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef SLR_PRIMINFO_H
#define SLR_PRIMINFO_H

#include "DDImage/DDImage_API.h"

#include "fdk/math/types.h" // for Mat4dList

#include "usg/geom/Path.h"

#include <vector>

namespace slr
{
  class SlrSurfaceMaterial;
  class SlrLightMaterial;
  class SlrLightShader;

  //------------------------------------------------------------------


  /*! This structure represents a single usg::Prim acting as a renderable surface.
  */
  class DDImage_API SlrPrimInfo
  {
  public:
    uint32_t              objectIndex;        //!< Index of object in renderable_prims in SlrRenderContext
    usg::Path             primPath;           //!< Prim path in Stage (prototype path if an instance)
    int32_t               primAdapter;        //!< To drive render primitive conversion
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
    // Legacy scanline shading data, not used when ray tracing, deprecate
    fdk::Mat4dList transforms;  //!< Double-precision scene-transform matrices
  };

  using SlrPrimInfoList = std::vector<SlrPrimInfo>;


  //------------------------------------------------------------------


  /*! This structure represents a single usg::Prim acting as a light source.
  */
  class DDImage_API SlrLightInfo
  {
  public:
    uint32_t          lightIndex;         //!< Index of the light prim in the SlrLightInfoList
    usg::Path         primPath;           //!< Prim path in Stage
    usg::Path         materialPath;       //!< Material binding path in Stage (for projections)
    SlrLightMaterial* material;           //!< Light material (stored in SlrRenderContext)
    //
    bool              castShadows;        //!< Does the light cast a shadow?


    SlrLightInfo();

    //! Convenience to get light shader from material. Warning, does no null testing!
    SlrLightShader*  lightShader() const;

    friend DDImage_API std::ostream& operator << (std::ostream&, const SlrLightInfo&);

  };

  using SlrLightInfoList = std::vector<SlrLightInfo>;


} // namespace slr

#endif
