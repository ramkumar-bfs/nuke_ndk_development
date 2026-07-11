// LightOp.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_LightOp_h
#define DDImage_LightOp_h

#include "DDImage/CameraOp.h"
#include "DDImage/Pixel.h"
#include "DDImage/TextureFilter.h"
#include "DDImage/LightMaterialOpI.h"


namespace DD
{
  namespace Image
  {
    class LightContext;
    class rPrimitive;
    class VertexContext;

    /*! \class DD::Image::LightOp
        \brief LightOp defines a 3D light.
     */
    /*!
       Light base class which defaults to point source lighting with falloff.
     */
    class DDImage_API LightOp : public CameraOp,
                                public LightMaterialOpI
    {
    public:
      //! Return this LightOp cast to the LightMaterialOpI interface, if it supports translation.
      virtual LightMaterialOpI*  asLightMaterialOp() { return nullptr; }


    public:
      LightOp(Node*                 node,
              GeomOpEngine::Builder engine_ctor);

#if kDDImageVersionMajorNum <= 14
      //! Backwards-compatible ctor.
      LightOp(Node * node);
#else
#error "Remove the above code block in Nuke 15 as they are deprecated in Nuke 14."
#endif

      //! Cast to a LightOp. This is much cheaper and safer than using dynamic_cast.
      const LightOp* lightOp() const override { return this; }
            LightOp* lightOp()       override { return this; }


      int samples() const { return samples_; }
      void set_samples(int n) { samples_ = n; }
      float sample_width() const { return sample_width_; }
      void set_sample_width(float width) { sample_width_ = width; }

      enum { eNoFalloff, eLinearFalloff, eQuadraticFalloff, eCubicFalloff };

      enum LightType { 
        ePointLight, 
        eDirectionalLight, 
        eSpotLight, 
        eOtherLight 
      };

      enum ShadowMode {
        eSolidShadow,           //!< the light cast shadow considering every geometry fully solid
        eTransparentShadow,     //!< the light cast shadow considering the alpha channel of the texture mapped on the geometry
        eSemiTransparentShadow  //!< the light cast shadow in semi transparent mode
      };

      //--------- Color/Intensity -----------
      const Pixel& color() const { return color_; }
      void color(const Pixel& v) { color_ = v; }
      float intensity() const { return intensity_; }
      void set_intensity(float v) { intensity_ = v; }

      // Obsolete methods for falloff
      bool falloff() const { return falloff_; }
      void set_falloff(bool b) { falloff_ = b; }

      int falloffType() const { return falloffType_; }
      void set_falloffType(int b) { falloffType_ = b; }

      //---------- Shadowing and Depthmap -------------
      bool cast_shadows() const { return cast_shadows_; }
      void set_cast_shadows(bool b) { cast_shadows_ = b; }
      int shadows_mode() const { return shadows_mode_; }
      float alpha_threshold() const { return alpha_threshold_; }
      TextureFilter* shadow_filter() { return &shadow_filter_; }
      float depthmap_bias() const { return depthmap_bias_; }
      void set_depthmap_bias(float v) { depthmap_bias_ = v; }
      float shadow_jitter_scale() const { return shadow_jitter_scale_; }
      void set_shadow_jitter_scale(float v) { shadow_jitter_scale_ = v; }
      int depthmap_width() const;
      void set_depthmap_width(int n) { depthmap_width_ = n; }
      void setShadowMaskChannel(Channel& channel ) { _shadowMaskChannel = channel; }
      Channel getShadowMaskChannel() const { return _shadowMaskChannel; }


      //------------------ Shading Calls ----------------------
      /*! Whether the light has a delta distribution (point/spot/direct lights) */
      virtual bool is_delta_light() const;

      /*! field-of-view methods. */
      virtual double hfov() const { return 90.0; }
      virtual double vfov() const { return 90.0; }
      virtual double aspect() const { return hfov() / vfov(); }

      virtual int lightType() const { return ePointLight; } 

    public:
      void _validate(bool for_real) override;

      /*! Handle channel requests.  Base class does nothing, but Lights
          that read imagery such as environment maps will need to
          implement this.
       */
      virtual void request(ChannelMask, int count);

      void knobs(Knob_Callback) override;
      int  knob_changed(Knob* k) override;
      void color_knobs(Knob_Callback);
      void attenuation_knobs(Knob_Callback);
      void shadow_knobs(Knob_Callback);

      unsigned node_color() const override { return 0xFFFF9A00; }
      void build_handles(ViewerContext*) override;

      /*! Draws a picture of a point light.
       */
      void draw_handle(ViewerContext*) override;

      /*! Point light OpenGL settings. */
      virtual void shade_GL(ViewerContext*, int light_num);

    protected:
      class DDImage_API Engine : public AxisEngine
      {
      public:
        Engine(Op* parent);

        void  processScenegraph(usg::GeomSceneContext& context) override;
      };

      //
      Pixel color_;               //!< Color of the light (can be >3 channels!)
      float intensity_;           //!< Global intensity

      //
      bool  falloff_;             //!< Whether to factor in physical falloff
      int falloffType_;           //!< falloff type
      int samples_;               //!< The number of samples (for area testing)
      float sample_width_;        //!< sample width (for shadowing)

      bool  cast_shadows_;        //!< Whether light casts shadows
      int shadows_mode_;          //!< Shadow casting mode
      TextureFilter shadow_filter_;      //!< Default filter to use in shadow texture sampling

      float scene_epsilon_;       //!< Scene epsilon for shadow comparison
      float depthmap_bias_;       //!< Offset in Z of shadow buffer
      float depthmap_slope_bias_; //!< Offset in Z to be scale in proportion of the local gradient
      float alpha_threshold_;     //!< Alpha threshold for transparent shadow map mode
      float shadow_jitter_scale_; //!< Size of shadow jitter
      int   depthmap_width_;      //!< Size in pixels of shadow buffer
      Channel _shadowMaskChannel; //!< Output the shadow mask to this channel

    private:
      //! Clamped values from knob controls.
      int _samples;
      int _depthmap_width;


#if kDDImageVersionMajorNum <= 14
    public:
      //----------------------------------------------------------------------------
      // Deprecated functions - will disappear in Nuke 15!
      //----------------------------------------------------------------------------
      mFnDeprecatedInNuke14("Class() has been deprecated, implement on subclass plugin instead.")
        const char* Class() const override { return "Light"; }
      mFnDeprecatedInNuke14("node_help() has been deprecated, implement on subclass plugin instead.")
        const char* node_help() const override { return ""; }
      mFnDeprecatedInNuke14("displayName() has been deprecated, implement on subclass plugin instead.")
        const char* displayName() const override { return Class(); }
      //-----------------------------
      mFnDeprecatedInNuke14("get_L_vector() has been deprecated.")
        virtual void  get_L_vector(LightContext&  ltx,
                                   const Vector3& surfP,
                                   const Vector3& lobeN,
                                   Vector3&       L, float& D) const;
      mFnDeprecatedInNuke14("get_color() has been deprecated.")
        virtual void  get_color(LightContext&  ltx,
                                const Vector3& surfP,
                                const Vector3& lobeN,
                                const Vector3& L, float D,
                                Pixel& out);
      mFnDeprecatedInNuke14("get_shadowing() has been deprecated.")
        virtual float get_shadowing(const LightContext&  ltx,
                                    const VertexContext& vtx,
                                    const Vector3&       surfP,
                                    Pixel&               out);
#else
#error "Remove the above code block in Nuke 15 as they are deprecated in Nuke 14."
#endif
    };

  }
}
#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
