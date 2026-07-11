// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_IMAGING_RENDERINTERFACE_H
#define USG_IMAGING_RENDERINTERFACE_H

#include "usg/base/ArgSet.h"
#include "usg/geom/Attribute.h"

#include "fdk/math/Mat4.h"
#include "fdk/math/Box2.h"

namespace usg
{

  //--------------------------------------------------------------------------------------------------

  class RenderInterfaceHandle;      //!< Wrapper abstraction for implementation type
  class RenderOutputLayerHandle;    //!< Wrapper abstraction for implementation type


  //!
  class USG_API RenderInterface
  {
  public:
    //-------------------------------------------------------------

    using DataFormat = uint8_t;

    // UNorm8 - unsigned normalized byte representing a float between 0 and 1:
    static DataFormat  FormatUNorm8;
    static DataFormat  FormatUNorm8Vec2;
    static DataFormat  FormatUNorm8Vec3;
    static DataFormat  FormatUNorm8Vec4;
    // SNorm8 - signed normalized byte representing a float between -1 and 1:
    static DataFormat  FormatSNorm8;
    static DataFormat  FormatSNorm8Vec2;
    static DataFormat  FormatSNorm8Vec3;
    static DataFormat  FormatSNorm8Vec4;
    //
    static DataFormat  FormatFloat16;
    static DataFormat  FormatFloat16Vec2;
    static DataFormat  FormatFloat16Vec3;
    static DataFormat  FormatFloat16Vec4;
    //
    static DataFormat  FormatFloat32;
    static DataFormat  FormatFloat32Vec2;
    static DataFormat  FormatFloat32Vec3;
    static DataFormat  FormatFloat32Vec4;
    //
    static DataFormat  FormatUInt16;
    static DataFormat  FormatUInt16Vec2;
    static DataFormat  FormatUInt16Vec3;
    static DataFormat  FormatUInt16Vec4;
    //
    static DataFormat  FormatInt32;
    static DataFormat  FormatInt32Vec2;
    static DataFormat  FormatInt32Vec3;
    static DataFormat  FormatInt32Vec4;
    //
    static DataFormat  FormatFloat32UInt8;  //!< Depth-stencil format
    //
    static DataFormat  FormatInvalid;
    static DataFormat  FormatCount;

    //-------------------------------------------------------------

    //!
    class USG_API OutputLayer
    {
    public:
      //! Default ctor builds an empty (invalid) OutputLayer.
      OutputLayer();
      //! Copy constructors
      OutputLayer(const OutputLayer&);
      OutputLayer& operator = (const OutputLayer&);
      OutputLayer(const RenderOutputLayerHandle&);

      // Dtor in implementation required for std::unique_ptr to work
      virtual ~OutputLayer();


      //! True if RenderOutputLayerHandle is valid (has a valid object binding.)
      bool      isValid() const;
      operator  bool()    const { return this->isValid(); }


      //! Resolve the buffer so that reads reflect the latest writes.
      void   resolve();

      //! Map(bind) the buffer for reading.
      void*  map();
      //! Unmap(unbind) the buffer. It is no longer safe to read from the buffer.
      void   unmap();

      //-------------------------------------------------------
      // Note - the accessor methods below do not check for a
      //        valid object binding. If called and not valid
      //        they will likely crash. Check isValid() first!
      //-------------------------------------------------------

      //! Return whether the buffer is currently mapped(bound) by anybody.
      bool     isMapped() const;
      //! Is the buffer converged (whether the renderer is still adding samples or not).
      bool     isConverged() const;

      int32_t     width() const;
      int32_t     height() const;
      int32_t     nChans() const;
      DataFormat  dataFormat() const;
      //! Derived from DataFormat.
      int32_t     bytesPerChan() const;
      //! Derived from DataFormat.
      bool        isSignedData() const;
      //! Derived from DataFormat.
      bool        isFloatData() const;


    protected:
      std::unique_ptr<RenderOutputLayerHandle> _pImpl;
    };

    //-------------------------------------------------------------

    struct RenderPluginDesc
    {
      Token       id;
      std::string displayName;
      int         priority;
    };

    //! Return the list of all loaded plugin names.
    static const std::vector<RenderPluginDesc>&  rendererPlugins();

    //! Checks whether the renderer plugin exists.
    static bool rendererPluginExists(Token id, RenderPluginDesc* desc = nullptr);

    //-------------------------------------------------------------

  public:
    RenderInterface();

    // Dtor in implementation required for std::unique_ptr to work
    virtual ~RenderInterface();

    //! True if RenderInterfaceHandle is valid (has a valid object binding.)
    bool              isValid() const;
    operator          bool()    const { return this->isValid(); }

    //-------------------------------------------------------------

    //! Identifier token for this renderer interface - should be unique!
    const Token&  rendererId() const { return _rendererId; }

    //-------------------------------------------------------------

    //!
    ArgSet&  rendererArgs() { return _rendererArgs; }

    //! Initialize the renderer interface, copying the provided args.
    //! This method will then call the virtual subclass method _initRenderer().
    void  initRenderer(const ArgSet& args);

    //! Returns true if the render substructures are allocated and initialized.
    //! Note - check this before calling any accessor methods!
    bool  rendererInitialized() const;

    //-------------------------------------------------------------

    /// -------------------------------------------------------
    /// Render Settings API

    struct RenderSettingsDescriptor
    {
      Token key;
      std::string name;
      Value defaultValue;
    };
    using RenderSettingsDescriptorVector = std::vector<RenderSettingsDescriptor>;

    //! Returns the available render settings
    const RenderSettingsDescriptorVector& renderSettings() const { return _rendererSettings; }

    //! Sets the render setting if it exists
    void setRenderSetting(const Token& key, const Value& v);

    //! Returns whether the key identifies a render setting
    bool isRenderSetting(const Token& key) const;

    //! Returns the render setting for key. If it doesn't exist, returns an empty descriptor
    const RenderSettingsDescriptor& getRenderSetting(const Token& key) const;

#if 0
    /// -------------------------------------------------------
    /// Rendering API

    /// Set the collection to be rendered.
    void setCollection(HdRprimCollection const& collection);

    /// Set the render params. Note: params.viewport will
    /// be overwritten, since it comes from SetRenderViewport.
    /// XXX: For GL renders, HdxTaskController relies on the caller to
    /// correctly set GL_SAMPLE_ALPHA_TO_COVERAGE.
    void setRenderParams(HdxRenderTaskParams const& params);

    /// Set the "view" opinion of the scenes render tags.
    /// The opinion is the base opinion for the entire scene.
    /// Individual tasks (such as the shadow task) may
    /// have a stronger opinion and override this opinion
    void setRenderTags(TfTokenVector const& renderTags);
#endif

    //! Sets the refine level for subdiv prims.
    void setRefineLevel(int level) { _refineLevel = level; }

    //! Sets whether the rendering should be done progressively.
    void setProgressiveRendering(bool progressiveRendering) { _progressiveRendering = progressiveRendering; }

#if 0
    /// -------------------------------------------------------
    /// AOV API

    /// Set which output should be rendered to the viewport. The empty token
    /// disables viewport rendering.
    void setViewportRenderOutput(TfToken const& name);

    /// Get the buffer for a rendered output. Note: the caller should call
    /// Resolve(), as HdxTaskController doesn't guarantee the buffer will
    /// be resolved.
    HdRenderBuffer* getRenderOutput(TfToken const& name);

    /// Set custom parameters for an AOV.
    void setRenderOutputSettings(TfToken const& name,
                                 HdAovDescriptor const& desc);

    /// Get parameters for an AOV.
    HdAovDescriptor getRenderOutputSettings(TfToken const& name) const;

    /// The destination API (e.g., OpenGL, see hgiInterop for details) and
    /// framebuffer that the AOVs are presented into. The framebuffer
    /// is a VtValue that encoding a framebuffer in a destination API
    /// specific way.
    /// E.g., a uint32_t (aka GLuint) for framebuffer object for OpenGL.
    void setPresentationOutput(TfToken const &api, VtValue const &framebuffer);
#endif

    /// -------------------------------------------------------
    /// Lighting API

    //! Enables/disables scene lighting.
    void setEnableLighting(bool enabled) { _enableLighting = enabled; }

    //! Returns whether scene lighting is enabled or disabled.
    bool isLightingEnabled() const { return _enableLighting; }
#if 0
    /// Set the lighting state for the scene.  HdxTaskController maintains
    /// a set of light sprims with data set from the lights in "src".
    /// @param src    Lighting state to implement.
    void setLightingState(GlfSimpleLightingContextPtr const& src);
#endif

    /// -------------------------------------------------------
    /// Camera and Framing API
    
    //!
    void setCameraFraming(const fdk::Box2i& displayWindow,
                          const fdk::Box2i& dataWindow,
                          float             pixelAspectRatio = 1.0f);

    //! Set the view and projection matrices for the free camera.
    //! Note: The projection matrix must be pre-adjusted for the window policy.
    void setFreeCameraMatrices(const fdk::Mat4d& viewMatrix,
                               const fdk::Mat4d& projectionMatrix);


    //! Set pixel w/h of the OutputLayers.
    void setOutputLayerSize(int32_t width,
                            int32_t height);

#if 0
    /// Specifies whether to force a window policy when conforming
    /// the frustum of the camera to match the display window of
    /// the camera framing.
    ///
    /// If set to {false, ...}, the window policy of the specified camera
    /// will be used.
    ///
    /// Note: std::pair<bool, ...> is used instead of std::optional<...>
    /// because the latter is only available in C++17 or later.
    void setOverrideWindowPolicy(const std::pair<bool, CameraUtilConformWindowPolicy>& policy);

    /// -- Scene camera --
    /// Set the camera param on tasks to a USD camera path.
    void SetCameraPath(SdfPath const& id);

    /// Set the viewport param on tasks.
    ///
    /// \deprecated Use SetFraming and SetRenderBufferSize instead.
    void SetRenderViewport(GfVec4d const& viewport);

    /// -- Free camera --
    /// Set the view and projection matrices for the free camera.
    /// Note: The projection matrix must be pre-adjusted for the window policy.
    void SetFreeCameraMatrices(GfMatrix4d const& viewMatrix,
                               GfMatrix4d const& projectionMatrix);
    /// Set the free camera clip planes.
    /// (Note: Scene cameras use clipping planes authored on the camera prim)
    void SetFreeCameraClipPlanes(std::vector<GfVec4d> const& clipPlanes);
#endif

#if 0
    /// -------------------------------------------------------
    /// Selection API

    /// Turns the selection task on or off.
    void SetEnableSelection(bool enable);

    /// Set the selection color.
    void SetSelectionColor(GfVec4f const& color);

    /// Set if the selection highlight should be rendered as an outline around
    /// the selected objects or as a solid color overlaid on top of them.
    void SetSelectionEnableOutline(bool enableOutline);

    /// Set the selection outline radius (thickness) in pixels. This is only
    /// relevant if the highlight is meant to be rendered as an outline (if
    /// SetSelectionRenderOutline(true) is called).
    void SetSelectionOutlineRadius(unsigned int radius);
#endif

    //! Indicates the new set of selected prims.
    void setSelected(const PathSet& newSelection);

    /// -------------------------------------------------------
    /// Shadow API

    //! Turns the shadow task on or off.
    void setEnableShadows(bool enabled) { _enableShadows = enabled; }

    //! Returns whether the shadow task is on or off.
    bool isShadowsEnabled() { return _enableShadows; }

#if 0
    /// Set the shadow params. Note: params.camera will
    /// be overwritten, since it comes from SetCameraPath/SetCameraState.
    void SetShadowParams(HdxShadowTaskParams const& params);
#endif

    /// -------------------------------------------------------
    /// Material API

    //! Enables/disables using scene materials.
    void setEnableMaterials(bool enabled) { _enableSceneMaterials = enabled; }

    //! Returns whether scene materials are enabled/disabled.
    bool isMaterialEnabled() { return _enableSceneMaterials; }

    void setDisplayRender(const bool displayRender) { _displayRender = displayRender; }
    bool isDisplayRender() { return _displayRender; }

    void setDisplayProxy(const bool displayProxy) { _displayProxy = displayProxy; }
    bool isDisplayProxy() { return _displayProxy; }

    void setDisplayGuides(const bool displayGuides) { _displayGuides = displayGuides; }
    bool isDisplayGuides() { return _displayGuides; }

#if 0
    /// -------------------------------------------------------
    /// Color Correction API

    /// Configure color correction by settings params.
    void SetColorCorrectionParams(HdxColorCorrectionTaskParams const& params);
#endif

#if 0
    /// -------------------------------------------------------
    /// Present API

    /// Enable / disable presenting the render to bound framebuffer.
    /// An application may choose to manage the AOVs that are rendered into
    /// itself and skip the task controller's presentation.
    void SetEnablePresentation(bool enabled);
#endif

    //-------------------------------------------------------------

    //! Start the engine execution of the currently assigned rendering tasks.
    //! If fullyConverge = true the method will loop until isConverged() = true
    //! before returning.
    void  executeTasks(bool fullyConverge = false);

    //! Is the buffer converged (whether the renderer is still adding samples or not).
    bool  isConverged() const;

    //-------------------------------------------------------------

    //! Set the list of named OutputLayers to be rendered.
    void  setOutputLayers(const TokenArray& names);

    //! Retrieve a named OutputLayer, if it exists. If not the returned OutputLayer object is invalid.
    //! Warning, no internal state checking performed, assumes rendererInitialized() returned true!
    OutputLayer  getOutputLayer(const Token& name) const;

    //! Set the identifier name for the scene
    void setSceneName(const std::string& sceneName) { _sceneName = sceneName; }
    //! Get the identifier name for the scene
    const std::string& getSceneName() const { return _sceneName; }


  protected:
    //! Subclasses can do any custom initializations in this method.
    virtual bool  _initRenderer(const Token& rendererId) { return false; }


  protected:
    std::unique_ptr<RenderInterfaceHandle> _pImpl;
    Token        _rendererId;       //!< Renderer identifier string
    ArgSet       _rendererArgs;     //!< Arguments to renderer
    std::vector<RenderSettingsDescriptor>       _rendererSettings; //!< Arguments to renderer
    std::string  _sceneName;        //!< Identifier name for scene
    PathSet      _selectedPrims;
    PathSet      _deselectedPrims;
    //
    bool         _rerender;             //!< True if we need a full render rather than a progressive render
    //
    // Render task params:
    bool       _progressiveRendering;   //!< Whether progressive rendering is turned on
    bool       _showViewerGeometry;     //!< True if we show the ViewerContext geometry, otherwise show from inputs
    bool       _enableLighting;         //!<
    bool       _enableSceneMaterials;   //!<
    bool       _enableShadows;          //!<
    bool       _displayRender;          //!< True if we are showing render prims
    bool       _displayProxy;           //!< True if we are showing proxy prims
    bool       _displayGuides;          //!< True if we are showing guide prims
    fdk::Vec4f _wireframeColor;         //!<
    fdk::Vec4f _pointColor;             //!<
    float      _pointSize;              //!<
    int        _refineLevel;            //!< The refine level for subdiv prims.
    //
    bool       _flatShading;            //!< Materials are not lit, they are constant-shaded colors & textures
    int32_t    _cullStyle;              //!< Culling style. Default to Nothing to match Nuke's viewer
    int32_t    _textureSize;            //!<
    //
    fdk::Vec3f _displayColor;           //!<
    fdk::Vec3f _ambientColor;           //!<
  };


} // usg

#endif
