// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef SLR_RENDERCONTEXT_H
#define SLR_RENDERCONTEXT_H

#include "DDImage/SlrShadingContext.h"

#include "DDImage/Iop.h"
#include "usg/geom/Stage.h"

namespace DD {
  namespace Image {
    class Sampler;
  }
}
namespace ndk {
  class Channel;
  class ChannelManager;
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

    //! Is renderer operating in 'legacy' mode? ie using the old ScanlineRender1 architecture?
    virtual bool  isSlr1LegacyMode() const = 0;

    //! ChannelManager for shading system. For internal use only.
    virtual ndk::ChannelManager*  shadingChannelManager() = 0;


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
    virtual void  assignSlrPrimInfoMaterial(SlrPrimInfo&        primInfo,
                                            SlrSurfaceMaterial* slr_material) = 0;


    //! Find the SlrLightMaterial bound to a material path, if any.
    virtual SlrLightMaterial*    getSlrLightInfoMaterial(const usg::Path& material_path) const = 0;

    //! Assign a SlrLightMaterial to a path. If an existing material exists for the path it's deleted and replaced.
    virtual void  assignSlrLightInfoMaterial(SlrLightInfo&     lightInfo,
                                             SlrLightMaterial* slr_material) = 0;


    /*! Retrieves the -first- Channel index assigned to the named primvar layer
        in the shading channel manager. The shader should have declared its desire
        to sample GPrim primvars via SlrShader::requestedInputPrimvars().

        Note that \a primvar_name is the name of the primitive source attribute
        -WITHOUT- the leading 'primvars:' namespace. For example 'displayColor', not
        'primvars:displayColor' which is the full name of the source attribute.

        An SlrShader should call this from _requestResources() method.

        The channel count is required to make sure the primvar layer name matches the
        expected channel and corresponding data type size. ie 2 channels for a Vec2 or
        double, 3 channels for a Vec3, and 4 channels for a Vec4.

        Returns ndk::Chan_Empty if the retrieval fails, which can happen if no GPrims
        provide the named primvar or the matched layer does not have the required
        channel count. Shaders should be prepared to handle this failure case by
        falling back to default values.

        Some primvars are automatically mapped from the original source primvar name
        to a predefined layer & channel. For example 'displayColor' is mapped to layer
        'rgb' and Chan_R, while 'displayOpacity' is mapped to layer 'alpha' and
        Chan_A.
    */
    virtual ndk::Channel  retrievePrimvarLayer(const usg::Token& primvar_name,
                                               int32_t           nChans) const = 0;
    //! Same as retrievePrimvarLayer but returns a Dual2Chans struct with the
    //! dx/dy differential channels assigned, if they exist, otherwise they are set
    //! to ndk::Chan_Empty.
    virtual Dual2Chans    retrieveDual2PrimvarLayer(const usg::Token& primvar_name,
                                                    int32_t           nChans) const = 0;

    /*! Retrieves the -first- Channel index assigned to the named writable shading layer
        in the shading channel manager. The shader should have declared its desire
        to write into these layers via SlrShader::requestedWritableShadingLayers().

        An SlrShader should call this from _requestResources() method.

        The channel count is required to make sure the layer name matches the expected
        channel and corresponding data type size. ie 2 channels for a Vec2 or double,
        3 channels for a Vec3, and 4 channels for a Vec4.

        Returns ndk::Chan_Empty if the retrieval fails, which can happen if the shading
        layer name was not requested or the matched layer does not have the required
        channel count. Shaders should be prepared to handle this failure case by
        falling back to default values.
    */
    virtual ndk::Channel  retrieveShadingLayer(const usg::Token& layer_name,
                                               int32_t           nChans) const = 0;
    //! Same as retrieveShadingLayer but returns a Dual2Chans struct with the
    //! dx/dy differential channels assigned, if they exist, otherwise they are set
    //! to ndk::Chan_Empty.
    virtual Dual2Chans    retrieveDual2ShadingLayer(const usg::Token& layer_name,
                                                    int32_t           nChans) const = 0;

    /*! Request a texture DD::Image::Sampler for an Iop and a set of channels.
        An SlrShader should call this from _requestResources() method.
        The DD::Image::Sampler will be created after all shaders have done their
        requests so that the combined channels are handled by the same Sampler.
    */
    virtual void  requestTextureSampler(DD::Image::Iop*              iop,
                                        const DD::Image::ChannelSet& requested_channels) = 0;

    //! Returns a texture DD::Image::Sampler, if one exists, for the Iop source.
    virtual DD::Image::Sampler*  getTextureSampler(DD::Image::Iop* iop) const = 0;


  protected:
    virtual ~SlrRenderContext() {}
    DD::Image::Op* _renderOp;         //!< Render Op that owns this context
    int32_t        _renderVersion;    //!< Incremented each time scene is re-rendered (re-validated)
    int32_t        _requestVersion;   //!< Incremented each time renderer is requested

  };

} // namespace slr


#endif
