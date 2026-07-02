//------------------------------------------------------------------------------
// DDImage/SlrThreadContext.h
//
// Copyright (c) 2021 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef SLR_THREADCONTEXT_H
#define SLR_THREADCONTEXT_H

#include "DDImage/SlrShadingContext.h"
#include "DDImage/SlrLightInfo.h"

#include "DDImage/Pixel.h"
#include <DDImage/Interest.h> // for InterestRatchet
#include "DDImage/Thread.h"
#include "DDImage/CameraOp.h" // for lens func

#include <thread>

namespace slr
{
  class SlrTraceableBvh;
  class SlrIntersectionInfo;

  /*! The render context (SlrRenderContext) has one of these for each thread it's
      performing shading calls with.

      It stores thread-safe variables to help avoid per-sample allocation/deletion
      costs for temporary structures like a Pixel in a SlrShader.

      Since there's usually very few of these (one per thread) the vars can be
      generous with their memory reserves. ie even at 90 threads (which may
      kill Nuke performance in other ways,) 90 of these objects would only
      occupy a relatively small amount of temporary memory.
  */
  class DDImage_API SlrThreadContext
  {
    public:
      SlrThreadContext();
      ~SlrThreadContext();

      uint32_t  index() const { return _index; }
      uint32_t  nThreads() const { return _nThreads; }
      DD::Image::ThreadId threadId() const { return _threadId; }

      //! Returns a reference to the shader context list.
      const std::vector<SlrShadingContext>&  shaderContextList() const { return _stxList; }

      /*! Assigns an InterestRatchet to all the Pixels. This InterestRatchet var is usually
          a local var sitting in the renderer's engine() method, which is thread safe.
      */
      void  setInterestRatchet(DD::Image::InterestRatchet* interest);

      //-------------------------------------------------------
      // SlrShaderContext management:

      //! Clear the shader context list but keep the memory allocation.
      void  clearShaderContexts() { _stxList.clear(); }

      //! Get the current SlrShaderContext (the last in the list.)
      SlrShadingContext&  currentShaderContext() { assert(_stxList.size() > 0); return _stxList[_stxList.size()-1]; }

      //! Get the shader context for index i.
      SlrShadingContext&  getShaderContext(uint32_t i) { assert(i < _stxList.size()); return _stxList[i]; }

      //! Add a SlrShaderContext to the end of the list, and return it, copying \a src_stx if it's not null.
      SlrShadingContext&  pushShaderContext(const SlrShadingContext* src_stx = nullptr);

      //! Add SlrShaderContext \a src_stx to the end of the list, copying the source context, returning a reference to it.
      SlrShadingContext&  pushShaderContext(const SlrShadingContext& src_stx,
                                            const fdk::Vec3f&        Rdir,
                                            float                    minT,
                                            float                    maxT,
                                            uint32_t                 ray_type,
                                            uint32_t                 sides_mode);

      //! Add SlrShaderContext \a src_stx to the end of the list, copying the source context, returning a reference to it.
      SlrShadingContext&  pushShaderContext(const SlrShadingContext& src_stx,
                                            const ndk::RayContext&   Rtx,
                                            uint32_t                 ray_type,
                                            uint32_t                 sides_mode);

      //! Remove a SlrShaderContext from the end of the list, and return the new index or -1 if empty.
      int  popShaderContext();


      //-------------------------------------------------------

      //! Render camera world-to-eye xform (world-space coords to camera local-space)
      const fdk::Mat4d& camWorld2Eye() const { return _camW2L; }

      //! Render camera eye-to-world xform (camera local-space coords to world-space)
      const fdk::Mat4d& camEye2World() const { return _camL2W; }

      //! Render camera origin (camera rays originate here).
      //! Same as camEye2World().getTranslation(), but cached for speed.
      const fdk::Vec3d& camP() const { return _camP; }

      //! Hero-view camera origin in world space - for specular copying
      const fdk::Vec3d& camHeroP() const { return _camHeroP; }

      bool  isPixel(int32_t px, int32_t py) const { return (px == _pxy.x && py == _pxy.y); }
      bool  isPixelX(int32_t px) const { return (px == _pxy.x); }
      bool  isPixelY(int32_t py) const { return (py == _pxy.y); }
      bool  isSample(int32_t si) const { return (si == _si); }

      bool  isDebugPixel() const { return (_pxy.x == _debugpxy.x && _pxy.y == _debugpxy.y); }
      bool  isDebugSample() const { return (_si == _debugsi); }


    protected:
      friend class SlrShadingContext;
      friend class RenderEngine;
      friend class IntersectedSurfaces;
      friend class RenderSpan;
      uint32_t              _index;
      uint32_t              _nThreads;
      DD::Image::ThreadId   _threadId;
      //
      SlrShadingContextList _stxList;                 //!< Ray segment context list - index 0 is always the primary(camera) ray
      //
      // Rendering info updated for current time the thread is handling:
      fdk::Mat4d            _camL2W;                  //!< Camera local-to-world xform (eye-space coords to world-space)
      fdk::Mat4d            _camW2L;                  //!< Camera world-to-local xform (world-space coords to eye-space)
      fdk::Vec3d            _camP;                    //!< Camera origin in world space (same as l2w.getTranslation())
      fdk::Vec3d            _camHeroP;                //!< Hero-view camera origin in world space - for specular copying
      //
      fdk::Mat4dList        _lensContextTransforms;   //!< Camera projection context matrices (TODO: switch to lens shader)
      DD::Image::CameraLensContext   _lensContext;    //!< Camera projection context (TODO: switch to lens shader)
      DD::Image::LensProjectionFunc* _lensFunc;       //!< Camera projection function to use (TODO: switch to lens shader)
      //
      fdk::Vec2i            _pxy;                     //!< Current output screen coords
      fdk::Vec2f            _sxy;                     //!< Current output subpixel screen coords
      int32_t               _si;                      //!< Current subsample index
      float                 _shutterOffset;           //!< Shutter offset from time0 reference (i.e. -0.5, -0.35, 0.0, +0.5, etc)
      uint32_t              _shutterStep;             //!< Global-context shutter-step index for the context's time
      //
      fdk::Vec2i            _debugpxy;                //!< For shading debugging, set by renderer
      int32_t               _debugsi;                 //!< For shading debugging, set by renderer
      //
      bool                  _blendBackToFront;        //!< Blending Z order is back-to-front (true) or front-to-back (false)
      bool                  _enableColorChannels;     //!< Shaders should output color layers + alphas
      bool                  _enableDataChannels;      //!< Shaders should output any data channels (Z, etc)
      //
      bool                  _lightingEnabled;         //!< If true, even if no lights the surface shaders should do illum calcs
      bool                  _shadowingEnabled;        //!< 
      bool                  _directLightingEnabled;   //!< 
      bool                  _indirectLightingEnabled; //!< 
      bool                  _useHeroCamForSpec;       //!< If enabled camera ray uses heroV for reflection vector
      fdk::Vec3f            _ambientLightingBias;     //!<
      //
      SlrTraceableBvh*      _objectsBvh;              //!< Raytracing BVH for object intersections
      SlrTraceableBvh*      _lightVolumesBvh;         //!< Raytracing BVH for light volume intersections
      SlrLightInfoList*     _activeLights;            //!< Currently-active lights
      //
      SlrIntersectionInfo*  _intersectionInfo;        //!< Intersection info working buffer
      //
      DD::Image::Pixel      _textureColor;            //!< For sampling texture map Iops - defaults to RGBA
      DD::Image::Pixel      _bindingColor;            //!< For Connection sampling calls - defaults to RGBA
      DD::Image::Pixel      _illumColor;              //!< For LightShader evaluation - defaults to RGB
      DD::Image::Pixel      _shadowColor;             //!< For LightShader shadow transmission evaluation - defaults to RGB (no depth!)
      DD::Image::Pixel      _volumeColor;             //!< For VolumeShader color evaluation - defaults to RGB

    private:
      // Disabled copy ctor & operator
      SlrThreadContext(const SlrThreadContext&) = delete;
      SlrThreadContext& operator=(const SlrThreadContext&) = delete;

  };

} // namespace slr


#endif
