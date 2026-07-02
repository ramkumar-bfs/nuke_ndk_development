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

#include <thread>

namespace slr
{

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
      //------------------------------------------------------------------
      // Thread-safe scratch data to be used during ray-tracing & shading.
      //
      // Warning - since these are shared variables they should only be
      // used at the end of a shader chain, for example when sampling
      // textures or light values. If passed up a shader graph there's
      // a good chance of conflict with another shader attempting to use
      // the same var.

      DD::Image::Pixel texture_color;   //!< For sampling texture map Iops - defaults to RGBA
      DD::Image::Pixel binding_color;   //!< For Connection sampling calls - defaults to RGBA
      DD::Image::Pixel illum_color;     //!< For LightShader evaluation - defaults to RGB
      DD::Image::Pixel shadow_color;    //!< For LightShader shadow transmission evaluation - defaults to RGB (no depth!)
      DD::Image::Pixel volume_color;    //!< For VolumeShader color evaluation - defaults to RGB


    public:
      SlrThreadContext();
      ~SlrThreadContext();

      //! Assign the thread info.
      void setThreadID(uint32_t            index,
                       uint32_t            nThreads,
                       DD::Image::ThreadId Id);

      uint32_t  index() const { return _index; }
      uint32_t  nThreads() const { return _nThreads; }
      DD::Image::ThreadId threadId() const { return _threadId; }

      //! Returns a reference to the shader context list.
      const std::vector<SlrShadingContext>&  shaderContextList() const { return _stx_list; }

      /*! Assigns an InterestRatchet to all the Pixels. This InterestRatchet var is usually
          a local var sitting in the renderer's engine() method, which is thread safe.
      */
      void  setInterestRatchet(DD::Image::InterestRatchet* interest);


      //-------------------------------------------------------
      // SlrShaderContext management:

      //! Clear the shader context list but keep the memory allocation.
      void  clearShaderContexts() { _stx_list.clear(); }

      //! Get the current SlrShaderContext (the last in the list.)
      SlrShadingContext&  currentShaderContext() { assert(_stx_list.size() > 0); return _stx_list[_stx_list.size()-1]; }

      //! Get the shader context for index i.
      SlrShadingContext&  getShaderContext(uint32_t i) { assert(i < _stx_list.size()); return _stx_list[i]; }

      //! Add a SlrShaderContext to the end of the list, and return it, copying 'stx' if it's not null.
      SlrShadingContext&  pushShaderContext(const SlrShadingContext* src_stx = nullptr);

      //! Add a SlrShaderContext to the end of the list, copying the source context, returning a reference to it.
      SlrShadingContext&  pushShaderContext(const SlrShadingContext& src_stx,
                                            const fdk::Vec3f&        Rdir,
                                            double                   minT,
                                            double                   maxT,
                                            uint32_t                 ray_type,
                                            uint32_t                 sides_mode,
                                            const RayDifferentials*  Rdif = nullptr);

      //! Add a SlrShaderContext to the end of the list, copying the source context, returning a reference to it.
      SlrShadingContext&  pushShaderContext(const SlrShadingContext& src_stx,
                                            const RayContext&        Rtx,
                                            uint32_t                 ray_type,
                                            uint32_t                 sides_mode,
                                            const RayDifferentials*  Rdif = nullptr);

      //! Remove a SlrShaderContext from the end of the list, and return the new index or -1 if empty.
      int  popShaderContext();


    private:
      uint32_t             _index;
      uint32_t             _nThreads;
      DD::Image::ThreadId  _threadId;
      std::vector<SlrShadingContext> _stx_list;   //!< Ray segment context list - index 0 is always the primary(camera) ray


    private:
      // Disabled copy ctor & operator
      SlrThreadContext(const SlrThreadContext&) = delete;
      SlrThreadContext& operator=(const SlrThreadContext&) = delete;

  };

} // namespace slr


#endif
