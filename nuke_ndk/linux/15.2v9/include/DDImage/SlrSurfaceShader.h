// Copyright (c) 2021 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef SLR_SURFACESHADER_H
#define SLR_SURFACESHADER_H

#include "DDImage/SlrShader.h"
#include "DDImage/SlrCoShader.h"
#include "DDImage/SlrLightShader.h"

namespace slr
{

  //-----------------------------------------------------------------------------


  /*! \class DD::Image::SlrSurfaceShader

      Surface shaders apply lighting calculations and are the final shading before
      fragment integration. There should be only one surface shader per shading
      shading network which may contain an arbitrarily-large coshader input network
      feeding the surface shader.
  */
  class DDImage_API SlrSurfaceShader : public SlrShader
  {
  public:
    //! Visibility traits of the surface.
    struct VisibilityTraits
    {
      ndk::RayContext::SidesMode sidesMode;     //!< Which side this material applies to (default is SIDES_BOTH)
      ndk::RayContext::TypeMask  rayVisMask;    //!< Which ray types the material is visible to

      VisibilityTraits()
        : sidesMode(ndk::RayContext::SIDES_BOTH)
        , rayVisMask(ndk::RayContext::typeBits())
      {
        //
      }
    };

    //! Sample blending operations.
    //! TODO ndk14: this enum is duplicated on SurfaceShaderOp! We need a 'common' shader
    //!             include headers that let us share these definitions since the schemas
    //!             also need the same values.
    enum class BlendOp {
      None, Replace,
      // ones needing alpha which only affect color channels are first:
      Over, Stencil, Mask,
      // cutout operations also use presence*opacity and affect all channels including data:
      Cutout, Reveal,
      // the rest don't need alpha and treat all color channels equally:
      Plus, Max, Min
    };
    typedef void (*BlendOpFunc)(const DD::Image::Pixel&       surface_sample,
                                const DD::Image::ChannelPack& color_chans,
                                DD::Image::Pixel&             out);


  public:
    //!
    SlrSurfaceShader(const char* path = "");
    //!
    SlrSurfaceShader(const InputKnobList&  inputs,
                     const OutputPortList& outputs,
                     void*                 inputs_storage_data=nullptr,
                     size_t                inputs_storage_size=0);

    //---------------------------------------------------------

    //! Returns the class name, must implement.
    const char*        shaderClass() const override { return "SlrSurfaceShader"; }

    //!
    SlrSurfaceShader*  asSurfaceShader() override { return this; }

    //! Default to always output Mask_RGBA channels, in addition to any input channels.
    void  _validateShader() override;

    //!
    static SlrSurfaceShader*  create(const char* slr_shader_class);
    static SlrSurfaceShader*  create(const usg::ShaderDesc* shader_desc);

    //---------------------------------------------------------

    //! Convenience to get a SlrCoShader input coshader. Calls getConnection() and tests its shader type.
    SlrCoShader*   getCoShaderForInput(const std::string& input_name) const;


    /*! Build a SlrSurfaceShaderGroup from a ShaderDescGroup.

        The conversion will start at the ndk_shader_group's 'output' ShaderDesc and
        recursively convert all defined input connections, so unconnected ShaderDesc's
        in the group they will be ignored.

    */
    static size_t  buildSurfaceShaderGroup(const SlrRenderContext&     slrtx,
                                           const usg::ShaderDescGroup& shader_desc_group,
                                           SlrSurfaceShaderGroup&      slr_shader_group_out);

    /*! Update the properties of a SlrSurfaceShaderGroup built from a ShaderDescGroup.
    */
    static bool    updateSurfaceShaderGroup(const SlrRenderContext&     slrtx,
                                            const usg::ShaderDescGroup& shader_desc_group,
                                            SlrSurfaceShaderGroup&      slr_shader_group_out);


    //! Calls updateCommonSurfaceProperties().
    void           updateProperties(const usg::ShaderDesc& shader_desc) override;

    /*! Update the common surface properties. A subclass can call this from
        within its updateProperty() method to update the common visibility
        and blending parameters.
    */
    virtual void   updateCommonSurfaceProperties(const usg::ShaderDesc& shader_desc);


  public:
    //--------------------------------------------------------------------
    // Slr2 surface-shading specific methods
    //
    // These are similar to the legacy Iop::fragment_shader() method (see
    // Iop.h for explanation of those methods) but are explicitly -only-
    // for surface shading which can include lighting calculations and
    // fragment merging.
    //
    // See SlrCoShader for passing general Pixel values between shaders.
    //--------------------------------------------------------------------

    /*! Calculate the shaded color of a surface, possibly with lighting.

        The \a stx shading context contains all the shade-time information
        the shader should need including interpolated vertex attribute
        values (a limited, fixed-size, pre-determined set) and references to
        scene information like the camera xforms.

        The color opacity and aovs of the surface are returned in the \a out
        Pixel while the surface 'presence' weight is the method's return value.

        Usually 1.0 (solid) should be returned from this call unless the surface
        shader knows what its presence weight is. Note that presence is explicitly
        different than transparency/opacity and communicates how physically solid the
        surface is, not how opaque the surface is to light transmission.
        For example a glass surface is solid and yet transparent to light, so a
        shader representing glass would set out.opacity() to 0.0 or some thin value,
        and yet return 1.0 for presence since the glass surface is physically solid.
        Note that a fog or similar participating-medium atmospheric effect is considered
        'physically solid' for the purposes of presence/coverage since the effect usually
        covers the entirety of the subpixel sample and thus is 'fully there', but is not
        optically opaque and thus would affect light transmission.

        Coverage/presence can be between 'not there'(0) and 'fully there'(1) typically
        when filtered texture values are used as the presence argument. For example a
        texture representing the pattern of a fine black mesh could have a uniform albedo
        color of 0,0,0, a uniform opacity of 1.0, and one channel of the texture used to
        modulate the returned sample's presence weight rather than sample's opacity.

        Why does this matter? When subsampled surfaces are merged (stacked) together the
        presence weight is used to determine if the sample should be composited (>0) or
        just ignored (0). Total pixel coverage is calculated by adding up all the
        composited subpixel presence weights.
    */
    virtual float  surfaceShader(const SlrShadingContext& stx,
                                 DD::Image::Pixel&        out) = 0;


    /*! Combine the results from the surfaceShader() method with the current value
        of the fragment output. The stx.blendBackToFront indicates whether \a fragment_out
        is being built in back-to-front (the default) or front-to-back Z order.
        Front-to-back order (stx.blendBackToFront == false) is typically only used for
        typical alpha blending, while back-to-front can do more arbitrary operations since
        the \a shaded_surface channels always overlay the previous \a fragment_out's value.

        Default implementation does either an over or under of all color channels
        depending on stx.blendingOrder.
    */
    virtual void   blendSurface(const SlrShadingContext& stx,
                                const DD::Image::Pixel&  shaded_surface,
                                DD::Image::Pixel&        fragment_out);

    /*! Whether the surface shader can produce an opaque (alpha = 1) result after blending is
        performed.
        Certain blend modes like Plus, Min, Max will never combine to an opaque result even if
        the alpha of the result is >= 1 because the final result is expected to be divided by
        the total number of blended samples.
    */
    virtual bool   canBlendToOpaque() const;

    /*! Blending mode the surface shader is set to. Because ScanlineRender is a utility renderer
        if can do special combining between samples that production renderers usually avoid or
        support via hacks, like additive mode.
        Integrators need to know the shader's blend mode to properly handle combining channels
        and whether to stop at the first solid surface. For example if the shader is set to
        BlendOp::Plus or BlendOp::Max then the integrator cannot stop shading at the first
        solid sample.
    */
    BlendOp        blendOpMode() const { return _blendOpMode; }

    /*! The blend function this shader is using. Provided externally so the func can be directly
        called by an integrator, otherwise the blendSurface() virtual method should be used.
    */
    BlendOpFunc    blendOpFunc() const { return _blendOpFunc; }


    //! Visibility traits of the surface.
    const VisibilityTraits&  visibilityTraits() const { return _visibility; }

    /*! Can this surface change its presence weight per sample or is it constant (solid)
        across the entire surface? If true is returned this indicates this shader needs to
        be run when calculating occlusion rays since the shadow silhouette of the surface
        may be affected by the per-sample surfaceShader() calculations.

        Note that this is only an efficiency hint to the renderer to save on surface shading
        during occlusion (shadow) testing, it will not affect integration of the surface
        which uses the presence weight returned from surfaceShader().

        Default implementation returns false indicating constant 100% presence across the
        entire surface - ie it's entirely solid.
    */
    virtual bool   hasVaryingPresence() const { return false; }


  protected:
    VisibilityTraits _visibility;     //!< Visibility traits of the surface
    BlendOp          _blendOpMode;    //!< Which blend operation to use when combining this surface sample
    BlendOpFunc      _blendOpFunc;    //!< The blending function called by the default blendSurface(), assigned in _validateShader().


  private:
    //! Disabled copy constructor.
    SlrSurfaceShader(const SlrSurfaceShader&) = delete;
    //! Disabled assignment operator.
    SlrSurfaceShader& operator = (const SlrSurfaceShader&) = delete;

  };

  using SlrSurfaceShaderList = std::vector<SlrSurfaceShader*>;
  using SlrSurfaceShaderMap  = std::unordered_map<std::string, SlrSurfaceShader*>;


  //-----------------------------------------------------------------------------
  //-----------------------------------------------------------------------------


  /*! \class DD::Image::SlrGeometryShader

      Displacement shader
  */
  class DDImage_API SlrGeometryShader : public SlrShader
  {
  public:
    //!
    SlrGeometryShader(const char* path = "");
    //!
    SlrGeometryShader(const InputKnobList&  inputs,
                      const OutputPortList& outputs,
                      void*                 inputs_storage_data = nullptr,
                      size_t                inputs_storage_size = 0);


    //! Returns the class name, must implement.
    const char*         shaderClass() const override { return "SlrGeometryShader"; }

    //!
    SlrGeometryShader*  asGeometryShader() override { return this; }

    //!
    static SlrGeometryShader*       create(const char* slr_shader_class);
    static SlrGeometryShader*       create(const usg::ShaderDesc* shader_desc);


  public:
    //---------------------------------------------------------
    //
    // Slr compatible shader methods to implement if a surface shader.
    // Refer to Iop.h for explanation of methods as these are intended
    // to match the legacy shading methods for backwards-compatibility.
    //
    //---------------------------------------------------------

#if 0
    //!
    virtual void    displacementShader(const SlrShadingContext& stx,
                                       SlrShadingParams&        out) = 0;
#endif

    //!
    virtual float   displacementBound() { return 0.0f; }

  private:
    //! Disabled copy constructor.
    SlrGeometryShader(const SlrGeometryShader&) = delete;
    //! Disabled assignment operator.
    SlrGeometryShader& operator = (const SlrGeometryShader&) = delete;

  };

  using SlrGeometryShaderList = std::vector<SlrGeometryShader*>;
  using SlrGeometryShaderMap  = std::unordered_map<std::string, SlrGeometryShader*>;


  //-----------------------------------------------------------------------------
  //-----------------------------------------------------------------------------


  /*! Entry point for SlrSurfaceShader and SlrGeometryShader groups.

      There's one of these created for each enabled SlrPrimInfo in the primary render
      scene.

      They're stored in the SlrRenderContext, owned by the renderer, and are are static
      for the entire render session so any texture bindings remain stable and thread safe
      through the entirety of the render.
  */
  class DDImage_API SlrSurfaceMaterial
  {
  protected:
    int32_t                _renderVersion;        //!< Updated each time material is refreshed

  public:
    SlrSurfaceShaderGroup  surfaceGroup;          //!< Slr(ScanlineRender) Shader group - managed externally
    SlrGeometryShaderGroup displacementGroup;     //!< Slr(ScanlineRender) Shader group - managed externally
    //
    DD::Image::Hash        hash;                  //!< Global hash of object material
    DD::Image::ChannelSet  textureChannels;       //!< All the channels from all texture samplers
    DD::Image::ChannelSet  outputChannels;        //!< All the channels this material outputs
    DD::Image::Format      outputFormat;          //!< Nominal Format if the material represents a texture
    //
    SlrShader::InputBindingList textureBindings;          //!< List of all texture bindings used in material
    //
    bool                   displacementEnabled;           //!< Is displacement enabled?
    int                    displacementSubdivisionLevel;  //!< What recursion level to subdivide to
    fdk::Vec3f             displacementBounds;            //!< Displacement bounds scaled by local-to-world matrix


  public:

    //!
    SlrSurfaceMaterial();

    //! Releases the SlrShader allocations in the SlrSurfaceShaderGroup
    ~SlrSurfaceMaterial();

    //! Current render version the material has been updated at.
    int32_t  renderVersion() const { return _renderVersion; }
    void     setRenderVersion(int32_t v) { _renderVersion = v; }

    //! Return the path of the surface_group output shader, if there is one. Used mainly for debugging.
    const std::string& path() const;

    /*! Fills in/updates the SlrSurfaceMaterial with the SlrShaders created from the
        enabled SlrPrimInfo and its assigned material prim path.

        The priminfo will be updated to point at any created or existing material.
        This is usually called by the renderer in its _validate() method as all the
        shaders are required to know what input textures and channels the renderer
        needs to output and request.

        The SlrRenderContext holds the shared resources that are pointed to by the
        SlrPrimInfos.
    */
    static SlrSurfaceMaterial* translateUsgMaterial(SlrRenderContext&     slrtx,
                                                    const fdk::TimeValue& time,
                                                    const usg::Path&      material_path,
                                                    SlrSurfaceMaterial*   existing_slr_material);


  public:
    //!
    SlrSurfaceShader*   surface() const { return surfaceGroup.output; }
    //!
    SlrGeometryShader*  displacement() const { return displacementGroup.output; }

    //! The list of primvars this material needs from the Gprim it's assigned to.
    //! At tessellation time this list is used to build the array of values read
    //! from prim.
    const ShadingLayerReqSet&  inputPrimvars() const;

    //! Return the primary 'st' primvar name this material uses for parametric surface
    //! evaluation. Note that this is usually the same primvar that's used for
    //! texture coordinates, but there can be different texture primvars used in each
    //! texture sampler, so it's not obvious what is the 'primary' parametric primvar.
    const ShadingLayerReq      parametricStPrimvar() const;

    //!
    const ShadingLayerReqSet&  writableShadingLayers() const;

    //! Visibility traits from the surface shader.
    const SlrSurfaceShader::VisibilityTraits&    visibilityTraits() const;

    //------------------------------------------------------------
    // Shading methods redirect to either 'surface_group' or
    // 'displacement_group'.
    //------------------------------------------------------------

    //! Calls the output shader, if assigned, and sets the material channels.
    void   validateShader(SlrRenderContext& slrtx);

    //! Calls requestResources() on the set of input shaders.
    void   requestResources(const DD::Image::ChannelSet& request_channels,
                            SlrRenderContext&            slrtx);

    //! Calls initializeForShading() on the set of input shaders.
    void   initializeForShading(uint32_t          num_shading_threads,
                                SlrRenderContext& slrtx);


    //! Redirects to \a surface_group.output, if valid.
    float  surfaceShader(const SlrShadingContext& stx,
                         DD::Image::Pixel&        out);

#if 0
    //! Redirects to \a surface_group.output, if valid.
    void   displacementShader(const SlrShadingContext& stx,
                              SlrShadingParams&        out);
#endif

    //!
    float  displacementBound();

  };

  using SlrSurfaceMaterialSet = std::unordered_set<SlrSurfaceMaterial*>;


} // namespace slr


#endif
