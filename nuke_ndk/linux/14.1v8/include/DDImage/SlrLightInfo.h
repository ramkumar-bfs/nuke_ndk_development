// Copyright (c) 2021 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef SLR_LIGHTINFO_H
#define SLR_LIGHTINFO_H

#include "DDImage/DDImage_API.h"

#include "usg/geom/Path.h"

#include <vector>

namespace DD {
  namespace Image {
    class Pixel;
  }
}

namespace slr
{
  class SlrLightMaterial;
  class SlrLightShader;


  /*! Replacement for legacy LightContext structure used in the legacy DD::Image::Render
      based architecture like ScanlineRender, DepthMapRender, etc.

      This structure represents a single usg::Prim at a single time sample. The Prim's
      attributes are not copied to time-sampled local arrays, that is left to the render
      light shader adapters at render time.
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
