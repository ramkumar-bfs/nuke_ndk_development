// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_IMAGING_STORMRENDERINTERFACE_H
#define USG_IMAGING_STORMRENDERINTERFACE_H

#include "fdk/base/Hash.h"
#include "usg/imaging/RenderInterface.h"

#include "fdk/base/Time.h"

namespace usg
{
  using StageRef = std::shared_ptr<class Stage>;


  //!
  class USG_API StormRenderInterface : public RenderInterface
  {
  public:
    class GLContext; // Abstract, castable to PXR_NS::GlfGLContext
    using GLContextSharedPtr = std::shared_ptr<class GLContext>;


    StormRenderInterface();

    //!
    //void  prepareRender(const DelegateParams& params);

    //!
    void  executeRenderer();

    //!
    void  finalizeRender();


    //----------------------------------------------------------

    //! Convert a DataFormat enumeration to the appropriate GL data format values.
    static void  getGLBufferFormat(const DataFormat& inFormat,
                                   uint32_t*         outGLFormat,
                                   uint32_t*         outGLType,
                                   uint32_t*         outGLInternalFormat = nullptr);

    //!
    static GLContextSharedPtr  getCurrentGLContext();
    //!
    static GLContextSharedPtr  getSharedGLContext();
    //!
    static void                makeCurrentGLContext(const GLContextSharedPtr& glcontext);


  protected:
    //! Starts up the Hydra GL instance. Call this before using the GLContext.
    virtual void  _initHydraGL();

    //!
    bool  _initRenderer(const Token& rendererId) override;

    //! Temp!
    void  _configureScene(const StageRef stage, fdk::TimeValue frame);

    //! Enables or disables the viewer camera light
    void _enableCameraLight(bool enable);

    //! Sets the viewer camera light position
    void _setCameraLightPosition(const fdk::Vec3f &pos);

    //! Enables or disables the dome light that will be added if the stage has no lights
    //! and the camera light is disabled.
    //! Note that a texture path must be configured for the domelight for HdStorm.
    void _enableDefaultDomeLight(bool enable);

    //! Sets the texture for the default dome texture that will be used when no lights are present in the stage
    //! and the camera light is disabled.
    void _setDefaultDomeLightTexturePath(const std::string& path);

    //! Notifies Hydra that the textures need to be reloaded
    void _reloadTextures(const std::vector<std::string>& textures);

  protected:
    friend class StormRenderInterfaceHandle;
    // Render delegate params:
    bool       _flipFrontFacing;              //!<
    bool       _applyRenderState;             //!<
    bool       _enableIdRender;               //!<
    bool       _enableSampleAlphaToCoverage;  //!<
    bool       _cameraLightEnabled;           //!<
    fdk::Vec3f _cameraLightPosition;          //!<
    bool        _defaultDomeLightEnabled;     //!< Whether the default dome light is enabled when no lights are present in the stage.
    std::string _defaultDomeTexturePath;      //!< Path to the texture for the default dome light.
    uint64_t   _sampleCount;                  //!<
    int32_t    _drawMode;                     //!<
    fdk::Hash  _lastStageProcessHash;         //!< Previous process hash of the stage to be rendered.
  };


} // usg

#endif
