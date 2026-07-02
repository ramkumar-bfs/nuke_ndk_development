// LightOp.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_LightOp_h
#define DDImage_LightOp_h

#include "CameraOp.h"
#include "Pixel.h"
#include "TextureFilter.h"

namespace DD
{
  namespace Image
  {
    class LightContext;
    class rPrimitive;

    /*! \class DD::Image::LightOp
        \brief LightOp defines a 3D light.
     */
    /*!
       Light base class which defaults to point source lighting with falloff.
     */
    class DDImage_API LightOp : public CameraOp
    {
    public:
      LightOp(Node * node);

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

      //! Calculate direction vector and distance from this light to \b P.
      /*! Calculate a normalized direction vector \b L and distance \b D
          to surface point \b P.  Surface normal \b N is passed to allow
          lights like area lights to simulate a large emission surface.
       */
      virtual void get_L_vector(LightContext& ltx,
                                const Vector3& P, const Vector3& N,
                                Vector3& L, float& D) const;
                                
      /*! \brief Return the amount of shadowing the light creates at surface point \b P.
          and optionally copies the shadow mask to the output pixel
      */
      /*! Project surface point \b P into light's view aperture and test if
          it's further in Z than the shadow buffer sample.  Jittered multi-
          sampling of the shadow buffer is supported when samples is greater
          than 1 which produces soft edged shadows.
       */
      virtual float get_shadowing(const LightContext& ltx,
                                  const VertexContext& vtx,
                                  const Vector3&      P,
                                  Pixel& out
                                  );

      //! Return the color of this light. The light vector and distance are passed.
      /*! Returns the color of the light (possibly) using the current
          surface point and normal to calculate attenuation and penumbra.
          The base class returns a point source attenuated by the inverse
          squared distance.
       */
      virtual void get_color(LightContext& ltx,
                             const Vector3& P, const Vector3& N,
                             const Vector3& L, float D,
                             Pixel& out);

      /*! field-of-view methods. */
      virtual double hfov() const { return 90.0; }
      virtual double vfov() const { return 90.0; }
      virtual double aspect() const { return hfov() / vfov(); }

      virtual int lightType() const { return ePointLight; } 

    public:
      /* virtual */ void _validate(bool for_real);

      /*! Handle channel requests.  Base class does nothing, but Lights
          that read imagery such as environment maps will need to
          implement this.
       */
      virtual void request(ChannelMask, int count);

      /* virtual */ void knobs(Knob_Callback);
      /* virtual */ int  knob_changed(Knob* k);
      void color_knobs(Knob_Callback);
      void attenuation_knobs(Knob_Callback);
      void shadow_knobs(Knob_Callback);

      /* virtual */ unsigned node_color() const { return 0xFFFF9A00; }
      /* virtual */ void build_handles(ViewerContext*);

      /*! Draws a picture of a point light.
       */
      /* virtual */ void draw_handle(ViewerContext*);

      /*! Point light OpenGL settings. */
      virtual void shade_GL(ViewerContext*, int light_num);

      //! This is the light node help.
      const char* node_help() const;

      //! This is the node display name
      const char* displayName() const;

      /* virtual */ const char* Class() const;
      static const Description description;

    protected:
      Pixel color_;               //!< Color of the light (can be >3 channels!)
      float intensity_;           //!< Global intensity
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

    };

  }
}
#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
