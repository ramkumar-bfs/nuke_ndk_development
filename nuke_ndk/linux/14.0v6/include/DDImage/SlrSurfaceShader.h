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
    void  _validateShader(const SlrRenderContext& slrtx) override;

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


  public:
    //--------------------------------------------------------------------
    // Slr2 surface-shading specific methods
    //
    // These are similar to the legacy Iop::fragment_shader() method (see
    // Iop.h for explanation of those methods) but are explicitly -only-
    // for surface shading which can include lighting calculations and
    // fragment merging.
    //
    // See SlrCoShader for passing general values between shaders.
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
    virtual float  surfaceShader(SlrShadingContext& stx,
                                 DD::Image::Pixel&  out) = 0;


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

    //!
    virtual void    displacementShader(const SlrShadingContext& stx,
                                       SlrVertex&               out) = 0;

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

      They're stored in the SlrRenderContext active_surface_materials list, owned by the
      renderer, and are are static for the entire render session so any texture bindings
      remain stable and thread safe through the entirety of the render.
  */
  class DDImage_API SlrSurfaceMaterial
  {
  protected:
    SlrRenderContext*     _slrtx;                 //!< Used to build OutputContext for shaders, if needed

  public:
    SlrSurfaceShaderGroup  surface_group;         //!< Slr(ScanlineRender) Shader group - managed externally
    SlrGeometryShaderGroup displacement_group;    //!< Slr(ScanlineRender) Shader group - managed externally
    //
    DD::Image::Hash        hash;                            //!< Global hash of object material
    DD::Image::ChannelSet  texture_channels;                //!< All the channels from all texture samplers
    DD::Image::ChannelSet  output_channels;                 //!< All the channels this material outputs
    DD::Image::Format      output_format;                   //!< Nominal Format if the material represents a texture
    //
    SlrShader::InputBindingList texture_bindings;           //!< List of all texture bindings used in material
    //
    bool                   displacement_enabled;            //!< Is displacement enabled?
    int                    displacement_subdivision_level;  //!< What recursion level to subdivide to
    fdk::Vec3f             displacement_bounds;             //!< Displacement bounds scaled by local-to-world matrix


  public:

    //!
    SlrSurfaceMaterial(SlrRenderContext* slrtx);

    //! Releases the SlrShader allocations in the SlrSurfaceShaderGroup
    ~SlrSurfaceMaterial();

    //! Return the path of the surface_group output shader, if there is one.
    const std::string& path() const;


    /*! Fills in/updates the SlrRenderContext with the SlrShaders created from the
        enabled SlrPrimInfos assuming the source Prims in the stage have materials
        assigned.

        The priminfo will be updated to point at any created or existing material.
        This is usually called by the renderer in its _validate() method as all the
        shaders are required to know what input textures and channels the renderer
        needs to output and request.

        The SlrRenderContext holds the shared resources that are pointed to by the
        SlrPrimInfos stored in each SlrScene time sample.
    */
    static void  translatePrimMaterial(const usg::StageConstRef& stage,
                                       const fdk::TimeValue&     time,
                                       const usg::Prim&          prim,
                                       SlrPrimInfo&              primInfo,
                                       SlrRenderContext&         slrtx);



  public:
    //!
    SlrSurfaceShader*   surface() const { return surface_group.output; }
    //!
    SlrGeometryShader*  displacement() const { return displacement_group.output; }

    //------------------------------------------------------------
    // Shading methods redirect to either 'surface_group' or
    // 'displacement_group'.
    //------------------------------------------------------------

    //! Calls the output shader, if assigned, and sets the material channels.
    void   validateShader(const SlrRenderContext& slrtx);

    //! Calls requestTextures() on the set of input shaders.
    void   requestTextures(const DD::Image::ChannelSet& request_channels,
                           SlrRenderContext&            slrtx);

    //! Calls initializeForShading() on the set of input shaders.
    void   initializeForShading(uint32_t          num_shading_threads,
                                SlrRenderContext& slrtx);


    //! Redirects to \a surface_group.output, if valid.
    float  surfaceShader(SlrShadingContext& stx,
                         DD::Image::Pixel&  out);

    //! Redirects to \a surface_group.output, if valid.
    void   displacementShader(const SlrShadingContext& stx,
                              SlrVertex&               out);
    //!
    float  displacementBound();

  };

  using SlrSurfaceMaterialList = std::vector<SlrSurfaceMaterial>;


} // namespace slr


#endif
