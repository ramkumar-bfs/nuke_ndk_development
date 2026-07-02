// CameraOp.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_CameraOp_h
#define DDImage_CameraOp_h

#include "AxisOp.h"
#include "Vector2.h"

namespace DD
{
  namespace Image
  {

    class Scene;
    class Format;
    class VArray;
    class MatrixArray;

    //----------------------------------------------------------------------

    //----------------------------------------------------------------------

    /*! \class DD::Image::CameraOp
       Op that defines a 3D camera.
     */
    class DDImage_API CameraOp : public AxisOp
    {
    protected:
      double focal_length_;
      double haperture_, vaperture_;
      double near_, far_;    //!< Near and far Z clipping planes

      Vector2 win_translate_; //!< Window coords translate XY
      Vector2 win_scale_;    //!< Window coords scale XY
      double win_roll_;     //!< Window coords rotate Z

      Matrix4 projection_;    //!< Camera's projection matrix
      int projection_mode_;    //!< Camera lens projection mode

      double focal_point_;    //!< Focal point of camera
      double fstop_;          //!< camera fstop

      //bool draw_frustum;        //!< Draw the frustum in OpenGL

      // Lens distortion parameters:
      bool use_vertical_lock_;
      double lens_distort_a_;
      double lens_distort_b_;
      double lens_distort_c_;
      double lens_distort_D_;
      double lens_distortion_;
      double lens_power_;
      bool lens_distort_enabled_;

    public:
      CameraOp(Node * node);

      void _validate(bool for_real) override;
      void knobs(Knob_Callback) override;
      virtual void projection_knobs(Knob_Callback);
      virtual void lens_knobs(Knob_Callback);

      unsigned node_color() const override { return 0x71C67100; }
      //void validate_bbox();

      /*! Draws a lovely camera, I think it is a Mitchell. */
      void draw_handle(ViewerContext*) override;

      void hfov(double v);
      double film_width() const { return haperture_; }
      void film_width(double v) { haperture_ = v; }
      double film_height() const { return vaperture_; }
      void film_height(double v) { vaperture_ = v; }
      double focal_length() const { return focal_length_; }
      void focal_length(double v) { focal_length_ = v; }
      double Near() const { return near_; }
      void Near(double v) { near_ = v; }
      double Far() const { return far_; }
      void Far(double v) { far_ = v; }
      double focal_point() const { return focal_point_; }
      void focal_point(double v) { focal_point_ = v; }
      double fstop() const { return fstop_; }
      void fstop(double v) { fstop_ = v; }

      // Lens projection modes:
      enum {
        LENS_PERSPECTIVE = 0, //!< Standard perspective projection
        LENS_ORTHOGRAPHIC, //!< Standard orthographic projection
        LENS_UV,          //!< Replace screen XY with UV coordinate
        LENS_SPHERICAL,   //!< 360x180 world spherically projected around camera
        //
        LENS_RENDER_CAMERA, //!< Renderer uses this option
        LENS_USER_CAMERA  = LENS_RENDER_CAMERA + 1//!< Camera user projection
      };
      int projection_mode() const { return projection_mode_; }
      void projection_mode(int i) { projection_mode_ = i; }
      static const char* projection_modes[];
      static const char* projection_modes_renderer[];

      /*! Returns true if current projection mode can be described linearly.
          This only true for perspective and orthographic modes if no
          lens distortion in being done.
       */
      virtual bool projection_is_linear(int mode) const;

      /*! Lens Projection/Distortion Function Type */
      typedef void (LensFunc)(Scene*, CameraOp*, MatrixArray* transforms,
                              const VArray& v, Vector4& out, void* data);

      /*! Returns the current lens function used to distort vertices during
          software rendering.  Single VArray version returns a Vector4.
       */
      virtual LensFunc* lens_function(int mode) const;
      typedef void (LensNFunc)(Scene*, CameraOp*, MatrixArray* transforms,
                               VArray* vert_array, int n, void* data);

      /*! Returns the current lens function used to distort vertices during
          software rendering.  Array function version that accepts a pointer
          to an array of VArray structures and thier number.
       */
      virtual LensNFunc* lensNfunction(int mode) const;

      double lens_distort_a() const { return lens_distort_a_; }
      double lens_distort_b() const { return lens_distort_b_; }
      double lens_distort_c() const { return lens_distort_c_; }
      double lens_distort_D() const { return lens_distort_D_; }
      double lens_distortion() const { return lens_distortion_; }
      double lens_power() const { return lens_power_; }
      bool lens_distort_enabled() const { return lens_distort_enabled_; }

      const Vector2& win_translate() const { return win_translate_; }
      const Vector2& win_scale() const { return win_scale_; }
      double win_roll() const { return win_roll_; }

      /*! Multiply the passed matrix by the necessary transformation to put
          x=-1 at the left edge of the \a format, +1 at the right, and to
          center it vertically, and compensate for any pixel aspect. The
          output z and w are left unchanged.

          Most Iops will need to do this using their output format to get the
          actual transformation from input space to output space. You would
          do this first, then multiply by the projection*local*parent.
       */
      static void to_format(Matrix4&, const Format*);

      /*! Opposite transformation of to_format(). Multiplies the passed
          matrix by a transformation that scales the input format to go from
          x = -1 to x = 1 and to center it vertically around y = 0, and to
          remove any pixel aspect ratio. You multiply by this matrix last.

          The 3DCard iop scales further so the image covers a range so that
          if the camera lens and "taking lens" are equal the image fills the
          field of view.
       */
      static void from_format(Matrix4&, const Format*);

      /*! \fn const Matrix4& CameraOp::projection() const

          Returns a transformation to an output image due to the camera lens.
          This will transform points at the left edge of the film to an x
          value of -1, and points at the right to an x value of +1. Vertically
          it will produce a square transform, ie for most film shapes the +/-1
          value for y is outside the top or bottom of the rectangular frame.

          The z output of this transformation is set to match the OpenGL
          near/far clipping planes at -1 and +1. These planes may be chosen
          by Nuke to make a nice display with OpenGL, and you should avoid
          having them have any effect on the output image. If you want the
          actual distance to an object from the camera, the w output of this
          transformation will give you that.

          For most uses you want to multiply projection()*xform()
          and pass the result through to_format() to get the transformation
          from world space to output pixels.
       */
      const Matrix4& projection() const { return projection_; }

      /*! Return camera projection matrix for a particular projection mode.
          It needs to be overridden if subclasses implement a different logic to calculate
          the projection matrix.
      */
      virtual Matrix4 projection(int mode) const;

      virtual Matrix4 projectionAt(const DD::Image::OutputContext&);

      const char* Class() const override;
      const char* node_help() const override;
      static const Description description;

      HandlesMode doAnyHandles(ViewerContext* ctx) override;

      void build_handles(ViewerContext*) override;

      /*! Return a default camera that can be used when the there is no
         camera attached to your renderer. This camera is at the origin, looking
         along -z, and has a lens ratio focal_length/film_width of 1.
       */
      static CameraOp* default_camera();

      /*!
         Return hints to control when and how this op will be evaluated by the top-down system.
      */
      OpHints opHints() const override;
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
