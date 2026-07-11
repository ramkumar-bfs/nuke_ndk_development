// Copyright (c) 2021 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef SLR_LIGHTINFO_H
#define SLR_LIGHTINFO_H

#include "DDImage/DDImage_API.h"

#include "fdk/math/types.h" // for Mat4dList
#include "fdk/math/Ray.h"

#include "usg/geom/Stage.h"

#include <vector>

namespace DD {
  namespace Image {
    class Pixel;
  }
}

namespace slr
{
  class SlrScene;
  class ShadowmapRender;
  using SlrShadowmapRender = ShadowmapRender;
  class SlrLightMaterial;
  class SlrLightShader;
  class SlrShadingContext;
  class SlrLightInfo;

  using RayContext = fdk::Rayf; // TODO: implement RayContext as a subclass of Ray
  class RayDifferentials;       // TODO: implement RayDifferentials

  /*! Callback to Slr2 to sample a shadow map and return the SQUARED distance from an
      occluder to the light(camera) origin. out[Chan_Z] will contain the distance value,
      and if requested out[Chan_Alpha] will contain the accumulated opacity all surfaces
      to infinity (or 1.0 opacity.) This is because the shadowmap render is not a
      deep pixel render, so if PW is placed between two objects and the closer object
      has 0.5 opacity, the alpha returned from this function will -not- be 0.5, it will
      be the accumulated opacity of both objects.
      Basically PW is always assumed to be at or in front of the frontmost object relative
      to the light(camera.)

      Warning! the distance value returned is the SQUARED vector length from the occluder
      to the light origin!
  */
  typedef void (ShadowmapProbe)(const SlrShadingContext& stx,
                                const SlrLightInfo&      ltInfo,
                                const RayContext&        illumRay,
                                const fdk::Vec3f&        PW,
                                DD::Image::Pixel&        out);


  /*! Replacement for legacy LightContext structure used in the legacy DD::Image::Render
      based architecture like ScanlineRender, DepthMapRender, etc.

      This structure represents a single usg::Prim at a single time sample. The Prim's
      attributes are not copied to time-sampled local arrays, that is left to the render
      light shader adapters at render time.
      However the Prim's concatenated scene transform is time sampled and stored in
      the 'depthmap_transforms' - note that all matrices are now double-precision.

      Contains an Axis object which contains transformed & normalized Eigen vectors for
      fast vector comparison in shaders.

      The 'depthmap_transforms' camera and projection matrices are used for shadowmap
      projections.
   */
  class DDImage_API SlrLightInfo
  {
  public:
    uint32_t              light_index;          //!< Index of the light prim in the SlrLightInfoList in SlrRenderContext
    usg::StageRef         stage;                //!< Stage containing Prim
    const usg::Path*      prim_path;            //!< Prim path in Stage (stored in Render prim_path_cache)
    SlrLightMaterial*     material;             //!< Light material (stored in SlrRenderContext)
    //
    bool                  cast_shadows;         //!< Does the light cast a shadow?

    // Time-sampled values updated in renderer:
    bool                  enabled;              //!< Light is 'on' for this scene
    fdk::TimeValue        time;                 //!< Time of this prim reference
    fdk::Vec3d            ltPW;                 //!< Light origin in world-space
    fdk::Vec3f            ltX;                  //!< Light X vector in world-space, normalized
    fdk::Vec3f            ltY;                  //!< Light Y vector in world-space, normalized
    fdk::Vec3f            ltZ;                  //!< Light Z vector in world-space, normalized
    //
    // Depthmap support updated in renderer:
    ShadowmapRender*      depthmap;             //!< Light's depth renderer, if it has one
    SlrScene*             depthmap_scene;       //!< To get at the light's depthmap transforms
    fdk::Mat4dList        depthmap_transforms;  //!< Double-precision scene-transform matrices configured for depthmap render


    SlrLightInfo() :
      light_index(0),
      prim_path(nullptr),
      material(nullptr),
      //
      cast_shadows(true),
      //
      enabled(true),
      time(fdk::defaultTimeValue()),
      ltPW(0.0f, 0.0f, 0.0f),
      ltX(1.0f, 0.0f, 0.0f),
      ltY(0.0f, 1.0f, 0.0f),
      ltZ(0.0f, 0.0f, 1.0f),
      depthmap(nullptr),
      depthmap_scene(nullptr)
    {}


    //! Convenience to get light shader from material. Warning, does no null testing!
    SlrLightShader*  light() const;

    // Light position and orientation vectors precalc'd and normalized for easy access in shaders.
    const fdk::Vec3f& x() const { return ltX; }
    const fdk::Vec3f& y() const { return ltY; }
    const fdk::Vec3f& z() const { return ltZ; }
    const fdk::Vec3d& p() const { return ltPW; }

    friend DDImage_API std::ostream& operator << (std::ostream&, const SlrLightInfo&);
  };

  using SlrLightInfoList = std::vector<SlrLightInfo>;


} // namespace slr

#endif
