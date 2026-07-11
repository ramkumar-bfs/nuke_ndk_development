//------------------------------------------------------------------------------
// DDImage/SlrRenderContext.h
//
// Copyright (c) 2021 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef SLR_RENDERCONTEXT_H
#define SLR_RENDERCONTEXT_H

#include "DDImage/SlrPrimInfo.h"
#include "DDImage/SlrLightInfo.h"
#include "DDImage/SlrShadingContext.h" // convenience for SlrShader plugins
#include "DDImage/SlrScene.h"          // convenience for SlrShader plugins

#include "DDImage/Pixel.h"             // convenience for SlrShader plugins

#include "fdk/math/Mat4.h"

#include <thread>
#include <map>
#include <unordered_map>

namespace DD {
  namespace Image {
    class Sampler;
  }
}

namespace slr
{
  class SlrThreadContext;

  using SlrPrimInfoMaterialMap  = std::unordered_map<uint64_t, SlrSurfaceMaterial*>;
  using SlrLightInfoMaterialMap = std::unordered_map<uint64_t, SlrLightMaterial*>;

  using SlrTextureRequestMap = std::unordered_map<fdk::Hash, std::pair<DD::Image::Iop*, DD::Image::ChannelSet> >;
  using SlrTextureSamplerMap = std::unordered_map<DD::Image::Iop*, DD::Image::Sampler*>;

  /*! 
  */
  class DDImage_API SlrRenderContext
  {
  public:
    usg::StageRef   stage;        //!< Stage containing scene geometry

    // Object & Light caches, shared between all motion segments and scenes
    SlrPrimInfoList                   renderable_prims;           //!< Global list of objects that -may- be rendered
    SlrPrimInfoMaterialMap            priminfo_material_map;      //!< SlrSurfaceMaterial mapped by SlrPrimInfo object_index
    std::vector<SlrSurfaceMaterial*>  active_surface_materials;   //!< All allocated SlrSurfaceMaterials
    DD::Image::ChannelSet             surface_material_channels;  //!< Intersected output channels of all active surface materials

    SlrLightInfoList                  renderable_lights;          //!< Global list of lights that -may- be contributing
    SlrLightInfoMaterialMap           lightinfo_material_map;     //!< SlrLightMaterial mapped by SlrLightInfo light_index
    std::vector<SlrLightMaterial*>    active_light_materials;     //!< All allocated SlrLightMaterials
    DD::Image::ChannelSet             light_material_channels;    //!< Intersected output channels of all active light materials

    SlrTextureRequestMap              texture_request_map;        //!< Textures Iops and their requested channels
    SlrTextureSamplerMap              texture_sampler_map;        //!< Set of currently-allocated Samplers keyed by Iop


  public:
    //! Pass in parent Op that owns the context
    SlrRenderContext(DD::Image::Op* op);

    //! Calls destroyMaterials().
    ~SlrRenderContext();

    //! Returns the Render Op that owns this context.
    DD::Image::Op*  renderOp() const { return _renderOp; }

    /*! Checks if parent renderOp has abort flag was set.
        Use this in loops to check if the user has changed the GUI and
        the render should be stopped.
    */
    bool  renderAborted() const;


    /*! Find the SlrSurfaceMaterial bound to the SlrPrimInfo's id index. Note that
        multiple SlrPrimInfos may be bound to the same SlrSurfaceMaterial.
    */
    SlrSurfaceMaterial*  getSlrPrimInfoMaterial(uint32_t object_index) const;

    /*! Find the SlrLightMaterial bound to the SlrLightInfo's id index. Note that
        multiple SlrLightInfos may be bound to the same SlrLightMaterial.
    */
    SlrLightMaterial*    getSlrLightInfoMaterial(uint32_t light_index) const;


    /*! Request a texture DD::Image::Sampler for an Iop and a set of channels.
        An SlrShader should call this from _requestTextures() method.
        The DD::Image::Sampler will be created after all shaders have done their
        requests so that the combined channels are handled by the same Sampler.
    */
    void  requestTextureSampler(DD::Image::Iop*              iop,
                                const DD::Image::ChannelSet& requested_channels);

    //! Returns a texture DD::Image::Sampler, if one exists, for the Iop source.
    DD::Image::Sampler*  getTextureSampler(DD::Image::Iop* iop) const;

    //! Deletes all the DD::Image::Samplers created by all the SlrShaders.
    void  destroyTextureSamplers();


  public:
    //! Deletes all the SlrSurfaceMaterials and SlrLightMaterials in the active materials list.
    void  destroyMaterials();

    //! Deletes all the SlrSurfaceMaterials in the active surface materials list.
    void  destroySurfaceMaterials();

    //! Deletes all the SlrLightMaterials in the active light materials list.
    void  destroyLightMaterials();


  private:
    DD::Image::Op*  _renderOp;    //!< Render Op that owns this context

  };

} // namespace slr


#endif
