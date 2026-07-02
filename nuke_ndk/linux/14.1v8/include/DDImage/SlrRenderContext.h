// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef SLR_RENDERCONTEXT_H
#define SLR_RENDERCONTEXT_H

#include "DDImage/Iop.h"
#include "usg/geom/Stage.h"

namespace DD {
  namespace Image {
    class Sampler;
  }
}

namespace slr
{
  class SlrPrimInfo;
  class SlrSurfaceMaterial;
  class SlrLightInfo;
  class SlrLightMaterial;


  /*! 
  */
  class DDImage_API SlrRenderContext
  {
  public:
    //! Pass in parent Op that owns the context
    SlrRenderContext(DD::Image::Op* op);

    //! Returns the Render Op that owns this context.
    DD::Image::Op*  renderOp() const { return _renderOp; }

    /*! Checks if parent renderOp has abort flag was set.
        Use this in loops to check if the user has changed the GUI and
        the render should be stopped.
    */
    bool  renderAborted() const;

    //! Version number incremented each time scene is re-rendered (re-validated)
    int32_t  renderVersion() const { return _renderVersion; }

    //! Version number incremented each time renderer is requested
    int32_t  requestVersion() const { return _requestVersion; }


    //! Get the Stage containing the scene geometry being rendered.
    virtual usg::StageRef  stage() const = 0;

    //!
    virtual int32_t             nRenderablePrims() const = 0;
    //!
    virtual const SlrPrimInfo&  renderablePrim(int32_t i) const = 0;

    //!
    virtual int32_t             nRenderableLights() const = 0;
    //!
    virtual const SlrLightInfo& renderableLight(int32_t i) const = 0;

    //!
    virtual int32_t             nInstancers() const = 0;
    //!
    virtual const SlrPrimInfo&  instancerPrim(int32_t i) const = 0;


    //! Find the SlrSurfaceMaterial bound to a material path, if any.
    virtual SlrSurfaceMaterial*  getSlrPrimInfoMaterial(const usg::Path& material_path) const = 0;

    //! Assign a SlrSurfaceMaterial to a path. If an existing material exists for the path it's deleted and replaced.
    virtual void  assignSlrPrimInfoMaterial(const usg::Path&    material_path,
                                            SlrSurfaceMaterial* slr_material) = 0;


    //! Find the SlrLightMaterial bound to a material path, if any.
    virtual SlrLightMaterial*    getSlrLightInfoMaterial(const usg::Path& material_path) const = 0;

    //! Assign a SlrLightMaterial to a path. If an existing material exists for the path it's deleted and replaced.
    virtual void  assignSlrLightInfoMaterial(const usg::Path&  material_path,
                                             SlrLightMaterial* slr_material) = 0;


    /*! Request a texture DD::Image::Sampler for an Iop and a set of channels.
        An SlrShader should call this from _requestTextures() method.
        The DD::Image::Sampler will be created after all shaders have done their
        requests so that the combined channels are handled by the same Sampler.
    */
    virtual void  requestTextureSampler(DD::Image::Iop*              iop,
                                        const DD::Image::ChannelSet& requested_channels) = 0;

    //! Returns a texture DD::Image::Sampler, if one exists, for the Iop source.
    virtual DD::Image::Sampler*  getTextureSampler(DD::Image::Iop* iop) const = 0;


  protected:
    virtual ~SlrRenderContext() {}
    DD::Image::Op*  _renderOp;        //!< Render Op that owns this context
    int32_t         _renderVersion;   //!< Incremented each time scene is re-rendered (re-validated)
    int32_t         _requestVersion;  //!< Incremented each time renderer is requested

  };

} // namespace slr


#endif
