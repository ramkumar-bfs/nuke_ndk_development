// ComplexLightOp.h
// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_ComplexLightOp_h
#define DDImage_ComplexLightOp_h

#include "DDImage/LightOp.h"

namespace DD {
  namespace Image {

    /*!
     * Light source base class that provides either a point source with falloff
     * (as per LightOp), but also adds the options of a directional light and
     * a spotlight
     */
    class DDImage_API ComplexLightOp : public LightOp
    {
    protected:
      double _coneAngleNotClamp;
      double _conePenumbraAngleNotClamp;
      double _coneFalloffNotClamp;
      float _coneAngle;
      float _conePenumbraAngle;
      float _coneFalloff;
      float _coneOuterCos, _iconeCos, _coneInnerCos;
      int _lightType;
      double _intensityScale;

    public:
      ComplexLightOp(Node* node);

      //! Horizontal field of view
      double hfov() const;

      using LightOp::get_color;

      // virtual
      int lightType() const { return _lightType; }

      //! This light has a delta distribution (a cone in 3D)
      bool is_delta_light() const;

      //! Calculate a normalized direction vector L and distance D to surface point P.
      void get_L_vector(LightContext& ltx, const Vector3& P, const Vector3& N,
                        Vector3& L, float& D) const;

      //! Returns the amount of light stiking the current surface point from the light.
      void get_color(LightContext& ltx, const Vector3& P, const Vector3& N,
                     const Vector3& L, float D, Pixel& out);

      //! This will set the OpenGL lighting settings.
      void shade_GL(ViewerContext* ctx, int light_num);

      //! Draws the light handle.
      void draw_handle(ViewerContext* ctx);

      //! This will validate the light node
      void _validate(bool for_real);

      void knobs(Knob_Callback f);
    };

  }
}

#endif
