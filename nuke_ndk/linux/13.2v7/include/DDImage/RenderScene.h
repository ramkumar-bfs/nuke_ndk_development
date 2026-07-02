// RenderScene.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_RenderScene_H
#define DDImage_RenderScene_H

#include "DDImage_API.h"

#include "Render.h"
#include "LightContext.h"

namespace DD
{
  namespace Image
  {

    //! 3D image renderer parent class
    class DDImage_API RenderScene : public Render
    {
    private:
      bool transparency_;
      bool ztest_enabled_;
      int    zblend_mode_;
      double zblend_range_;

      Vector3 ambient_;        //!< User set ambient RGB color

      double focal_jitter_; //!< User set jittering scaling in focus

      //! These operations can check aborted states during build_handles, when the op may not be in another
      //! tree such as a viewer or write.  Because of this it needs to maintain its own additional tree.
      OpTree* _pOpTree;

      //! internal helper for camera_matrix
      static Matrix4 CameraMatrix(CameraOp* cam, int sample, const Intersection* jitter, float focalJitter);

      //! internal helper for projection_matrix
      static Matrix4 ProjectionMatrix(const Format& f, CameraOp* cam, int mode);

    protected:
      enum { kZBlendNone = 0, kZBlendLinear, kZBlendSmooth };

      RenderScene(Node * node, bool enable_mip_filter = false);
      ~RenderScene();

      /*! Validate the 3D scene renderer.
          We construct one Scene structure per time sample.
          The Scene contains all primitives and lights transformed at that
          moment in time.
       */
      void _validate(bool);

      /*! Request input channels from background and assign output channels.
       */
      void _request(int x, int y, int r, int t, ChannelMask, int);

      void initialize_span_context(Span& span);

      bool uniform_distribution_; //!< If true uniform distribuite the scenes over times
      int  stochastic_samples_;   //!< Number of sample for pixel to use in the stocastic reconstruction

      bool output_shader_vectors;
      Channel P_channel[3];    //!< P(osition) vector to output
      Channel N_channel[3];    //!< N(ormal) vector to output

      /*! Returns the camera attached to input 2(if available,) otherwise 0.
       */
      CameraOp* render_camera(int sample = 0);

      /*! Returns the GeoOp connected to input 1 for \n sample.
       */
      GeoOp* render_geo(int sample = 0);

      /*! Returns the inverse camera matrix for a particular sample.  This
          supports jittering of the camera's position around the focal_point.
       */
      Matrix4 camera_matrix(int sample = 0);

      /*! Returns the camera projection matrix for a particular sample.
         ARRGH! Egg makes this translate from -1..1 rather than pixels...
         This should be able to use CameraOp::to_format().
       */
      Matrix4 projection_matrix(int sample = 0);

    public:
      void knobs(Knob_Callback);
      int  knob_changed(Knob* k);

      Op::HandlesMode doAnyHandles(ViewerContext* ctx);

      /*! Sets 2D viewer to 3D mode to draw any geometry in the input.
         Adds the camera as something that should be snapped to.
       */
      void build_handles(ViewerContext*);

      int minimum_inputs() const { return 3; }
      int maximum_inputs() const { return 3; }

      /*! Return how many op inputs this operator requires for the arrow
         the user sees numbered n. */
      int split_input(int) const;

      /*! Changes the time for input m for temporal sampling. */
      const OutputContext& inputContext(int, int, OutputContext&) const;

      bool test_input(int input, Op* op) const;
      Op* default_input(int input) const;
      const char* input_label(int input, char* buffer) const;

      bool transparency() const { return transparency_; }
      bool ztest_enabled() const { return ztest_enabled_; }
      double zblend_range() { return zblend_range_; }
      int zblend_mode() { return zblend_mode_; }

      virtual void detach();
      virtual void attach();
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
