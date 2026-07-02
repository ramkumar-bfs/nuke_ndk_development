// Copyright (c) 2023 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef SLR_INTEGRATOR_H
#define SLR_INTEGRATOR_H

#include "DDImage/Pixel.h"
#include "DDImage/DeepPixel.h"
#include "DDImage/Knob.h"
#include "DDImage/Knobs.h"
#include "DDImage/SlrPlugin.h"
#include "DDImage/SlrSurfaceShader.h" // convenience for SlrIntegrator plugins

#include "ndk/geo/render/RayContext.h"

#include "fdk/math/types.h" // for Vec2fList

#include <unordered_map>

namespace DD {
  namespace Image {
    class Op;
  }
}

namespace slr
{
  class SlrRenderContext;
  class SlrThreadContext;
  class SlrShadingContext;
  class SlrIntersectionInfo;


  //-----------------------------------------------------------------------------


  /*! This class is passed to an SlrIntegrator from the renderer pixel loop for each pixel
      being rendered. It contains all the camera rays being shot for a pixel so the integrator
      could perform spatial weighting since each ray origin is at a known relative subpixel
      location inside the pixel.
  */
  class DDImage_API SlrIntegratorContext
  {
  public:
    const fdk::Vec2i&  pixelXY() const { return _pixelXY; }
    int32_t            pixelX() const { return _pixelXY.x; }
    int32_t            pixelY() const { return _pixelXY.y; }

    bool  isPixel(int32_t x, int32_t y) const { return (x == _pixelXY.x && y == _pixelXY.y); }
    bool  isPixelX(int32_t x) const { return (x == _pixelXY.x); }
    bool  isPixelY(int32_t y) const { return (y == _pixelXY.y); }

    bool  isDebugPixel() const;
    bool  isDebugSample() const;

    const DD::Image::Format&  renderFormat() const;
    const DD::Image::Box&     renderRegion() const;

    //! Number of primary/camera ray samples.
    int32_t  nSamples() const { return _nPrimaryRays; }

    //! Sampling random number generator owned by this thread.
    ndk::SamplingRngSet&   samplingRng() const;

    //!
    const fdk::Vec2fList&  subpixelXYs() const { return _subpixelXYs; }
    //! Returns the subpixel XY offset coordinate inside pixelXY where 0,0 is the center of the
    //! pixel, -1,-1 is the pixel left/bottom edge, and +1,+1 is the pixel right/top edge.
    const fdk::Vec2f&      subpixelXY(size_t i) const { return _subpixelXYs[i]; }

    //!
    const int32_t               primaryRayBundle() const { return _primaryRayBundle; }
    //!
    const int32_t               nPrimaryRays() const { return _nPrimaryRays; }

    //!
    const fdk::TimeValueList&  times() const { return _times; }
    fdk::TimeValue             time(size_t i) const { return _times[i]; }

    //----------------------------------------------------------

    //! Background for final integrated result.
    //! The output pixel of the integrator is expected to incorporate the background into its
    //! final result.
    //! This is provided as a separate value as an integrator may need this value first in
    //! the integration vs. compositing the integrated result at the end.
    const DD::Image::Pixel&  bgPixel() const { return _bgPixel; }


    //! Final integrated & flattened result.
    DD::Image::Pixel&  outputPixel() { return _outputPixel; }

    //----------------------------------------------------------

    //! Intersect a bundle of rays with the closest surfaces to the ray origins, filling in
    //! shading contexts. There will only be one shading context created per ray, so the size
    //! of the StxBundle will never exceed nRays. If the rays are a coherent set the hits can
    //! be described as a 'wavefront' pushed out from the ray origins along the ray directions.
    //!
    //! Building surface shading contexts requires evaluating additional surface properties so
    //! is more costly than getting the raw geometric hit info.
    //!
    //! Note that \a stxBundle is -not- cleared before adding hits to it.
    void  getNearestIntersections(const SlrShadingContext& srcStx,
                                  const ndk::RayContext*   rtxArray,
                                  int32_t                  nRays,
                                  StxBundle&               stxBundle) const;

    //! Intersect a bundle of rays with the closest surfaces to the ray origins, filling in
    //! geometric hit infos. There will only be one hit info created per ray, so the size
    //! of the hit list will never exceed nRays. If the rays are a coherent set the hits can
    //! be described as a 'wavefront' pushed out from the ray origins along the ray directions.
    //!
    //! Note - \a hitList -is- cleared before adding the hits.
    void  getNearestIntersections(const SlrShadingContext& srcStx,
                                  const ndk::RayContext*   rtxArray,
                                  int32_t                  nRays,
                                  ndk::RayHitPointfList&   hitList) const;

    //! Intersect a single ray with all surfaces, filling in shading contexts. In this case the
    //! output StxBundle contains the list of intersections along a single ray path rather than a list
    //! from multiple ray paths.
    //!
    //! This is the most expensive intersection method as all intersectable surfaces are retrieved,
    //! even potentially hidden ones, and building surface shading contexts requires evaluating
    //! additional surface properties.
    //!
    //! Note that \a stxBundle is -not- cleared before adding hits to it.
    void  getAllIntersections(const SlrShadingContext& srcStx,
                              const ndk::RayContext&   Rtx,
                              StxBundle&               stxBundle) const;

    //! Intersect a single ray with the all surfaces, filling in geometric hit infos.
    //! Note - \a hitList is -not- cleared before adding the hits.
    void  getAllIntersections(const SlrShadingContext& srcStx,
                              const ndk::RayContext&   Rtx,
                              ndk::RayHitPointfList&   hitList) const;

    //! Intersect a 2d UV point with surface UV coords, returning true if hitStx is filled in.
    bool  getUVIntersection(const SlrShadingContext& srcStx,
                            const fdk::Vec2f&        Puv,
                            SlrShadingContext&       hitStx) const;

    //! Start a new RayContext bundle making it the current bundle and returning a reference
    //! to it. Note that only one bundle may be active at a time since each bundle must be a
    //! contiguous section of RayContexts in the cache.
    ndk::RayContextBundle&  startNewRtxBundle() const;

    //! Access the current RayContextBundle for this thread which can then be added to. Note
    //! that only one bundle may be current at a time.
    ndk::RayContextBundle&  currentRtxBundle() const;

    //-------------------------------------------------------

    //! Start a new StxBundle making it the current bundle and returning a reference to it.
    //! Note that only one bundle may be active at a time since each bundle must be a
    //! contiguous section of SlrShadingContexts in the cache.
    StxBundle&  startNewStxBundle() const;

    //! Access the current StxBundle for this thread which can then be added to. Note
    //! that only one bundle may be current at a time.
    StxBundle&  currentStxBundle() const;

    //! List of currently-created SlrShadingContext bundles.
    const StxBundleList&  stxBundles() const;

    //!
    StxBundleList&  activeStxBundles() const;

    //----------------------------------------------------------

  protected:
    friend class SlrThreadContext;
    friend class RenderEngine;
    friend class IntersectedSurfaces;
    SlrThreadContext*   _sttx;
    fdk::Vec2i          _pixelXY;           //!< Output pixel screen coords
    //
    int32_t             _primaryRayBundle;  //!< All the camera rays for this output pixel
    int32_t             _nPrimaryRays;      //!< Number of primary rays in bundle
    fdk::Vec2fList      _subpixelXYs;       //!< All the subpixel uv coords for this output pixel, one per primary ray
    fdk::TimeValueList  _times;             //!< All the sample times, one per primary ray
    //
    DD::Image::Pixel    _bgPixel;           //!< Background pixel data
    DD::Image::Pixel    _outputPixel;       //!< The output pixel data, flattened
    //DD::Image::DeepPixel  _outputDeepPixel;   //!< The output pixel deep samples data

    SlrIntegratorContext(SlrThreadContext*);
  };


  //-----------------------------------------------------------------------------


  /*! \class DD::Image::SlrIntegrator
  */
  class DDImage_API SlrIntegrator : public SlrPlugin
  {
  private:
    //! Disabled copy constructor.
    SlrIntegrator(const SlrIntegrator&) = delete;
    //! Disabled assignment operator.
    SlrIntegrator& operator = (const SlrIntegrator&) = delete;


  public:
    //-----------------------------------------------------------------
    /*! This structure creates a subclass of SlrIntegrator, possibly from loading a plugin file
        or a compiled-in Description.
    */
    class DDImage_API PluginDescription : public SlrPlugin::Description
    {
    public:
      //! Constructor method definitions used for 'build()' methods in plugins.
      typedef SlrIntegrator* (*IntegratorBuilder)(void);

    public:
      //! SlrIntegrator specific builder.
      PluginDescription(const char*       integratorClass,
                        IntegratorBuilder integratorBuilder);

      /*! Prefix and suffix strings prepended and appended to integratorClass() string to
          construct plugin filenames (vs. the loaded Description name)
      */
      static const char*  pluginPrefix() { return slr::pluginPrefix; }
      static const char*  pluginSuffix() { return "Integrator"; }

      //! Name of the shader class without any leading pluginPrefix() or trailing pluginSuffix().
      const char*         integratorClass() const { return _pluginClass; }
    };
    //-----------------------------------------------------------------


  protected:
    static SlrIntegrator* _create(const char* integratorClass);

    //! Default implementation combines the channels from all inputs.
    virtual void  _validateIntegrator(int32_t nPixelSamples);


  public:
    //!
    SlrIntegrator();

    //! Must have a virtual destructor
    virtual ~SlrIntegrator() {}

    //! Returns the class name, must implement.
    virtual const char*  integratorClass() const = 0;

    //! Create a SlrIntegrator instance from a integrator class name. Calling code takes ownership of returned pointer.
    static SlrIntegrator*  create(const PluginDescription& integratorDescription);
    static SlrIntegrator*  create(const char* integratorClass);


  public:

    //!
    virtual void knobs(DD::Image::Knob_Callback f) {}
    //!
    virtual int knob_changed(DD::Image::Knob* knob) { return 0; }


    //!
    //! Calls subclass _validateIntegrator().
    void validateIntegrator(int32_t nPixelSamples);


    //! Generate an output Pixel result given a list of source camera rays via the \a Itx
    //! SlrIntegratorContext's outputPixel().
    //! These are the final output floats copied into the renderer Iop's output channels in its
    //! engine (or deep engine) method.
    //!
    //! The default implmentation of this method takes a list of camera rays and does intersection
    //! testing with each, creating a set of hit intersection SlrShadingContexts(Stxs) from the
    //! -nearest- intersections. That's passed to the SlrIntegrator subclass' \a integrate() method
    //! which will typically perform shading and combine the results together to form the final
    //! output Pixel result.
    //!
    //! An integrator subclass can replace this default behavior to create custom calculations
    //! from the camera rays, or completely ignore them altogether. An example of this would
    //! be placing debug information in the output Pixel floats.
    //!
    //! Note that \a camStx is an -empty- Stx provided by the renderer and intended only for
    //! spawning the hit SlrShadingContexts. It does not contain any valid surface information
    //! since it represents the camera's origin and not a surface. However \a camStx could contain
    //! a valid atmosphereShader representing the volume that the camera origin is inside and the
    //! current Ior refractive index.
    //! \a camRtxBundle is the list of RayContexts generated from the camera.
    virtual void  integrateRays(SlrIntegratorContext&        Itx,
                                const SlrShadingContext&     camStx,
                                const ndk::RayContextBundle& camRtxBundle) const;

    //! Perform shading, etc to calculate the final output Pixel result.
    //! An integrator will typically perform shading on each hit intersection then combine the results
    //! together to form the final output Pixel result.
    //!
    //! \a hitStxBundleList contains StxBundles where each one contains a list of hit intersection
    //! SlrShadingContexts(Stxs) produced by the integrateRays() method. Most often bundles will only
    //! contain a single intersection Stx from the nearest object hit
    //!
    //! Must implement.
    virtual void  integrate(SlrIntegratorContext& Itx,
                            const StxBundleList&  hitStxBundleList) const = 0;



    //!
    void print(const char*   prefix,
               std::ostream& o) const;
    friend std::ostream& operator << (std::ostream& o, const SlrIntegrator& b) { b.print("", o); return o; }


  }; // SlrIntegrator

  using SlrIntegratorList = std::vector<SlrIntegrator*>;
  using SlrIntegratorMap  = std::unordered_map<std::string, SlrIntegrator*>;


} // namespace slr


#endif
