// Copyright (c) 2021 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef SLR_LIGHTSHADER_H
#define SLR_LIGHTSHADER_H

#include "DDImage/SlrShader.h"
#include "DDImage/SlrCoShader.h"
#include "DDImage/SlrLightInfo.h"

namespace slr
{

  /*! \class DD::Image::SlrLightShader
  */
  class DDImage_API SlrLightShader : public SlrShader
  {
  public:
    /*! Input values set by local knobs or input connections.
    */
    struct BaseInputParams
    {
      fdk::Vec3f  color     = fdk::Vec3f(1.0f); //!< Color of the emitted light
      float       intensity = 1.0f;             //!< Scale of the emitted light, multed with color

      BaseInputParams() : color(1.0f), intensity(1.0f) {}
      BaseInputParams(const fdk::Vec3f& _color,
                      float             _intensity) : color(_color), intensity(_intensity) {}
    };

    DD::Image::Pixel _color;              //!< Pre-calculated global output color - usually color*intensity


  protected:
    fdk::TimeValueList _motion_times;     //!< Time for each motion-sample
    fdk::Mat4dList     _motion_xforms;    //!< Motion xform matrices
    fdk::Mat4dList     _motion_ixforms;   //!< Inverse motion xform matrices created from _motion_xforms
    //
    bool               _enabled;          //!< Light can illuminate scene (usually cacl'd from _color > 0)


  public:
    //!
    SlrLightShader(const char* path = "");
    //!
    SlrLightShader(const InputKnobList&  inputs,
                   const OutputPortList& outputs);
    //!
    SlrLightShader(const fdk::TimeValueList& motion_times,
                   const fdk::Mat4dList&     motion_xforms);
    //!
    SlrLightShader(const InputKnobList&      inputs,
                   const OutputPortList&     output,
                   const fdk::TimeValueList& motion_times,
                   const fdk::Mat4dList&     motion_xforms);

    //---------------------------------------------------------

    //! Returns the class name, must implement.
    const char*      shaderClass() const override { return "SlrLightShader"; }
    //!
    SlrLightShader*  asLightShader() override { return this; }

    //!
    static SlrLightShader*  create(const char* slr_shader_class);
    static SlrLightShader*  create(const usg::ShaderDesc* shader_desc);

    //---------------------------------------------------------

    //! Convenience to get a SlrCoShader input coshader. Calls getConnection() and tests its shader type.
    SlrCoShader*   getCoShaderForInput(const std::string& input_name) const;

    /*! Build a SlrLightShaderGroup from a ShaderDescGroup.

        The conversion will start at the ndk_shader_group's 'output' ShaderDesc and
        recursively convert all defined input connections, so unconnected ShaderDesc's
        in the group they will be ignored.

    */
    static size_t  buildLightShaderGroup(const SlrRenderContext&     slrtx,
                                         const usg::ShaderDescGroup& shader_desc_group,
                                         SlrLightShaderGroup&        slr_shader_group_out);

    /*! Update the properties of a SlrLightShaderGroup built from a ShaderDescGroup.
    */
    static bool    updateLightShaderGroup(const SlrRenderContext&     slrtx,
                                          const usg::ShaderDescGroup& shader_desc_group,
                                          SlrLightShaderGroup&        slr_shader_group_out);

    //---------------------------------------------------------

    //! Called either on shader creation or shader value update.
    void  updateProperties(const SlrRenderContext& slrtx,
                           const usg::ShaderDesc&  shader_desc) override;

    //! Assumes updateProperties() has already updated local vars.
    void  _validateShader(const SlrRenderContext& slrtx) override;


    /*! Quick test if light can illuminate scene.
        Usually valid only after updateProperties() has been called.
    */
    bool  isEnabled() const { return _enabled; }

    //---------------------------------------------------------

    /*! Assign the motion transforms matrices. The inverses will be automatically created.
        Subclasses can add add'l transforms before storage.
    */
    virtual void       setMotionXforms(const fdk::TimeValueList& motion_times,
                                       const fdk::Mat4dList&     motion_xforms);

    // Access to list of motion times and xforms. They should have the same count!
    uint32_t           numMotionTimes()  const { return (uint32_t)_motion_times.size(); }
    uint32_t           numMotionXforms() const { return (uint32_t)_motion_xforms.size(); }

    //! Get a time from the list of motion times. No range checking!
    double             getMotionTime(uint32_t index) const { return _motion_times[index]; }
    //! Get a matrix from the list of motion xforms. No range checking!
    const fdk::Mat4d&  getMotionXform(uint32_t index) const { return _motion_xforms[index]; }


    //! Get a possibly-interpolated matrix at 'time'.
    fdk::Mat4d         getMotionXformAt(const fdk::TimeValue& time) const;
    //!
    fdk::Mat4d         getInverseMotionXformAt(const fdk::TimeValue& time) const;
    //! Get both at once (saves a motion step calculation.)
    void               getMotionXformsAt(const fdk::TimeValue& time,
                                         fdk::Mat4d&           xform,
                                         fdk::Mat4d&           ixform) const;


  public:
    //--------------------------------------------------------------------
    // Shading methods
    //--------------------------------------------------------------------

    /*! Whether the light has a delta distribution (point/spot/direct lights)
        Must implement.
    */
    virtual bool    isDeltaLight() const = 0;

    /*! Cone angle used for shadow map projection, valid between 0..~89 degrees. 
        Defaults to -1 to disable shadow map generation.
    */
    virtual float   coneAngle() const { return -1.0f; }


    /*! Evaluate the light's contribution to a surface intersection.
        Returns false if light does not contribute to surface illumination.
        Must implement.

        stx.PW() is the world-space point being illuminated, usually a surface.

        \a illumRayOut is built by the SlrLightShader and normally points from surface
        point to light origin and can be used for specular angle, etc.

        \a illumRayOut.minT' should be set to 0 or an epsilon bias off the light's
        'surface', clamped to the light's near value, if it has one.

        \a illumRayOut.maxT' should be the distance between surface point and light,
        clamped to the light's far value, if it has one.

        \a directPdfWeightOut is the direct-lighting power distribution function
        weight of the light for illumRayOut. Should -not- include shadow weighting.

        \a colorOut is the color(s) of light without any shadowing or falloff
        effects applied. i.e. it's the 'raw' output color of the light at the
        nominal 'surface' of the light for the angle described by illumRayOut.

        \a shadowTransmissionOut is the amount of transmission (shadowing/occlusion)
        between the light and the surface point. This is kept separate from \a colorOut
        and \a directPdfWeightOut so that per-channel shadowing contribution can be
        output from a shader separated from the illumination color and falloff.

        Final illumination of the light is typically calculated by multiplying
        (colorOut.rgb() * shadowTransmissionOut.rgb()) * directPdfWeightOut,
        bearing in mind that DD::Image::Pixels can be multi-channel/layer!
    */
    virtual bool    illuminate(const SlrShadingContext& stx,
                               const SlrLightInfo&      ltInfo,
                               RayContext&              illumRayOut,
                               float&                   directPdfWeightOut,
                               DD::Image::Pixel&        colorOut,
                               DD::Image::Pixel&        shadowTransmissionOut) const = 0;


  private:
    //! Disabled copy constructor.
    SlrLightShader(const SlrLightShader&) = delete;
    //! Disabled assignment operator.
    SlrLightShader& operator = (const SlrLightShader&) = delete;

  };

  using SlrLightShaderList = std::vector<SlrLightShader*>;
  using SlrLightShaderMap  = std::unordered_map<std::string, SlrLightShader*>;


  //-----------------------------------------------------------------------------


  /*! Entry point for SlrLightShader groups.

      There's one of these created for each enabled SlrLightInfo in the primary render
      scene.

      They're stored in the SlrRenderContext active_light_materials list, owned by the
      renderer, and are are static for the entire render session so any texture bindings
      remain stable and thread safe through the entirety of the render.
  */
  class DDImage_API SlrLightMaterial
  {
  protected:
    SlrRenderContext*     _slrtx;               //!< Used to build OutputContext for shaders, if needed

  public:
    SlrLightShaderGroup   light_group;          //!< Slr(ScanlineRender) Shader group - managed externally
    //
    DD::Image::Hash       hash;                 //!< Global hash of object material
    DD::Image::ChannelSet texture_channels;     //!< All the channels from all texture samplers
    DD::Image::Channel    shadow_channel;       //!< Channels to write shadow weight to
    DD::Image::ChannelSet output_channels;      //!< All the channels this material outputs
    //
    SlrShader::InputBindingList texture_bindings; //!< List of all texture bindings used in material


  public:

    //!
    SlrLightMaterial(SlrRenderContext* slrtx);

    //! Releases the SlrShader allocations in the SlrLightShaderGroup
    ~SlrLightMaterial();



    /*! Fills in/updates the SlrRenderContext with the SlrShaders created from the
        enabled SlrLightInfos.

        The lightinfo will be updated to point at any created or existing material.
        This is usually called by the renderer in its _validate() method as all the
        shaders are required to know what input textures and channels the renderer
        needs to output and request.

        The SlrRenderContext holds the shared resources that are pointed to by the
        SlrLightInfos stored in each SlrScene time sample.
    */
    static void  translateLightPrim(const usg::StageConstRef& stage,
                                    const fdk::TimeValue&     time,
                                    const usg::LightPrim&     ltPrim,
                                    SlrLightInfo&             ltInfo,
                                    SlrRenderContext&         slrtx);



  public:
    //!
    SlrLightShader*   light() const { return light_group.output; }


    //------------------------------------------------------------
    // Shading methods redirect to 'light_shaders' group.
    //------------------------------------------------------------

    //! Calls the output shader, if assigned, and sets the material channels.
    void  validateShader(const SlrRenderContext& slrtx);

    //! Calls requestTextures() on the set of input shaders.
    void  requestTextures(const DD::Image::ChannelSet& request_channels,
                          SlrRenderContext&            slrtx);

    //! Calls initializeForShading() on the set of input shaders.
    void   initializeForShading(uint32_t          num_shading_threads,
                                SlrRenderContext& slrtx);


    //! Redirects to \a light_group.output, if valid.
    bool  illuminate(const SlrShadingContext& stx,
                     const SlrLightInfo&      ltInfo,
                     RayContext&              illumRayOut,
                     float&                   directPdfWeightOut,
                     DD::Image::Pixel&        colorOut,
                     DD::Image::Pixel&        shadowTransmissionOut) const;

  };

  using SlrLightMaterialList = std::vector<SlrLightMaterial>;


} // namespace slr


#endif
