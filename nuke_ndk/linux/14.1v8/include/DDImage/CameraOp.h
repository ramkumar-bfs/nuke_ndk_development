//------------------------------------------------------------------------------
// DDImage/CameraOp.h
//
// Copyright (c) 2021 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef DDIMAGE_CAMERAOP_H
#define DDIMAGE_CAMERAOP_H

#include "DDImage/AxisOp.h"

#include "fdk/math/Vec2.h"
#include "fdk/math/Mat4.h"
#include "fdk/math/types.h" // for Mat4dList

#include "DDImage/Shutter.h"
#include "DDImage/Vector2.h"

namespace slr
{
  class SlrVertex;
}

namespace DD
{
  namespace Image
  {

    class Scene;
    class Format;
    class MatrixArray;
    class CameraLensContext;
    // For backwards-compatibility:
    class Scene;
    class VArray;

    //----------------------------------------------------------------------

    /*! Predefined lens projection modes.
    */
    enum class LensProjection : int32_t
    {
      // Linear projections:
      PERSPECTIVE = 0,                  //!< Standard perspective projection
      ORTHOGRAPHIC,                     //!< Standard orthographic projection
      // Non-linear projections:
      UV,                               //!< Replace XY with UV coordinate
      SPHERICAL,                        //!< 360x180 world spherically projected around camera
      //
      RENDER_CAMERA,                    //!< Renderer uses this option to get lens projection from input camera
      USER_CAMERA = RENDER_CAMERA + 1   //!< Camera user projections start here
    };

    // TODO - this should become a lens shader Op created by the CameraOp type or plugin.
    //
    // TODO - this perhaps needs to be more generic and read/write a float data array,
    //        but for lens projection I think we really only need to output a Vec4 per vert.
    //
    typedef void (LensProjectionFunc)(const CameraLensContext& context,
                                      slr::SlrVertex*          vert_array,
                                      uint32_t                 nVerts,
                                      void*                    user_data);

    //----------------------------------------------------------------------

    /*! Contains information used for camera lens projection functions.

        Info like the camera xform, projection matrix, and their inverses
        are now passed in this struct so the function prototype remains
        static and this struct can be expanded in scope.

        TODO: what does this need for stereo support? etc
    */
    class DDImage_API CameraLensContext
    {
    public:
      LensProjection   mode;            //!< Which projection mode the context is configured for
      fdk::Mat4d*      transforms;      //!< Mat4d array matching slr::SlrMatrixDefaults order & count (primarily for Slr2 use)
      //
      const Format*    format;          //!< Output pixel format
      int              formatWrapU;     //!< How to handle coordinate wrapping at left/right edge of format
      int              formatWrapV;     //!< How to handle coordinate wrapping at top/bottom edge of format
      //
      double           distortA;        //!< Distortion a polynomial
      double           distortB;        //!< Distortion b polynomial
      double           distortC;        //!< Distortion c polynomial
      double           distortD;        //!< Distortion D polynomial derived from a, b & c


    public:
      CameraLensContext(LensProjection _mode = LensProjection::PERSPECTIVE) :
        mode(_mode),
        transforms(nullptr),
        format(nullptr),
        formatWrapU(0/*none*/),
        formatWrapV(0/*none*/),
        distortA(0.0),
        distortB(0.0),
        distortC(0.0),
        distortD(0.0)
      {}


      /*! Returns true if projection in context can be described linearly with a
          4x4 matrix, based on the context's current settings. This is usually only
          true for perspective and orthographic modes if no lens distortion in
          being done.
      */
      bool  isLinearProjection() const;
    };


    /*! Returns a standard (built in) lens function appropriate for the LensProjection mode.
        CameraLensContext is passed so the mode, wrapping and format can be configured.
    */
    DDImage_API
    LensProjectionFunc*  LensProjectionFunction(LensProjection     mode,
                                                const Format*      format,
                                                CameraLensContext& context);

    //----------------------------------------------------------------------

    /*! \class DD::Image::CameraOp
       Op that defines a 3D camera.
     */
    class DDImage_API CameraOp : public AxisOp
    {
    protected:
#if kDDImageVersionMajorNum <= 15
      //----------------------------------------------------------------------------
      // Deprecation unions to retain backward-compatibility in Nuke 14
      // The old var names will disappear in Nuke 15!
      //----------------------------------------------------------------------------
      union {
        double         _focalLength;            //!< Focal length of lens (*** in millimeters ***)
        mFnDeprecatedInNuke14("focal_length_ has been deprecated, use _focalLength instead.")
        double focal_length_;    //!< Use _focalLength instead
      };
      union {
        double         _horizontalAperture;     //!< Horizontal aperture size (film width) (*** in millimeters ***)
        mFnDeprecatedInNuke14("haperture_ has been deprecated, use _horizontalAperture instead.")
        double haperture_;       //!< Use _horizontalAperture instead
      };
      union {
        double         _verticalAperture;       //!< Vertical aperture size (film height) (*** in millimeters ***)
        mFnDeprecatedInNuke14("vaperture_ has been deprecated, use _verticalAperture instead.")
        double vaperture_;       //!< Use _verticalAperture instead
      };
      union {
        double         _fStop;                  //!< Lens aperture diameter (*** unitless ratio tied to focal-length ***)
        mFnDeprecatedInNuke14("fstop_ has been deprecated, use _fStop instead.")
        double fstop_;           //!< Use _fStop instead
      };
      //
      union {
        double         _nearPlaneDistance;      //!< Near clipping plane distance (*** in world units ***)
        mFnDeprecatedInNuke14("near_ has been deprecated, use _nearPlaneDistance instead.")
        double near_;            //!< Use _nearPlaneDistance instead
      };
      union {
        double         _farPlaneDistance;       //!< Far clipping plane distance (*** in world units ***)
        mFnDeprecatedInNuke14("far_ has been deprecated, use _farPlaneDistance instead.")
        double far_;             //!< Use _farPlaneDistance instead
      };
      union {
        double         _focusDistance;          //!< Focus point distance (*** in world units ***)
        mFnDeprecatedInNuke14("focal_point_ has been deprecated, use _focusDistance instead.")
        double focal_point_;     //!< Use _focusDistance instead
      };
      //
      union {
        fdk::Vec2f     _windowTranslate;        //!< Window coords translate XY
        mFnDeprecatedInNuke14("win_translate_ has been deprecated, use _windowTranslate instead.")
        Vector2 win_translate_;  //!< Use _windowTranslate instead
      };
      union {
        fdk::Vec2f     _windowScale;            //!< Window coords scale XY
        mFnDeprecatedInNuke14("win_scale_ has been deprecated, use _windowScale instead.")
        Vector2 win_scale_;      //!< Use _windowScale instead
      };
      union {
        double         _windowRotateZ;          //!< Window coords rotate Z
        mFnDeprecatedInNuke14("win_roll_ has been deprecated, use _windowRotateZ instead.")
        double win_roll_;        //!< Use _windowRotateZ instead
      };
      union {
        LensProjection _lensProjectionMode;     //!< Lens projection mode
        mFnDeprecatedInNuke14("projection_mode_ has been deprecated, use _lensProjectionMode instead.")
        int projection_mode_;    //!< Use _lensProjectionMode instead
      };
#endif
      //
      ShutterControls  _shutter;                //!< Shutter controls which may be referenced by a renderer
      double           _shutterBias;            //!< Weights the shutter samples towards shutter close with a power function
      //
      double           _worldToMeters;          //!< World to meters - used to convert lens mm values to world scale
      //
      bool             _glWorldSizeIcon;        //!< Always draw the camera icon at the 'real' world size (defined by _worldToMeters)
      bool             _glSolidFrustum;         //!< Draw the camera frustum as solid
      bool             _glInsideFrustum;        //!< Indicated whether to draw the frustum as an inside or outside box


      // Lens distortion parameters:
      bool    use_vertical_lock_;
      double  lens_distort_a_;
      double  lens_distort_b_;
      double  lens_distort_c_;
      double  lens_distort_D_;
      double  lens_distortion_;
      double  lens_power_;
      bool    lens_distort_enabled_;

      fdk::Mat4d  _projectionMatrix;            //!< Camera's projection matrix (only valid for linear projections)


    protected:
      class DDImage_API CameraEngine : public AxisEngine
      {
      public:
        CameraEngine(Op* parent);

        //!
        CameraOp* cameraOp() const { return op_cast<CameraOp*>(_firstOp); }

        //!
        void  processScenegraph(usg::GeomSceneContext& context) override;


      protected:
        friend class CameraOp; // allow CameraOp to set engine vals

      };


    public:
      CameraOp(Node*                 node,
               GeomOpEngine::Builder engine_ctor);

#if kDDImageVersionMajorNum <= 15
      //! Backwards-compatible ctor.
      CameraOp(Node* node);
#else
#error "Remove the above code block in Nuke 16 as they are deprecated in Nuke 14."
#endif

      //! Cast to a CameraOp. This is much cheaper and safer than using dynamic_cast.
      const CameraOp* cameraOp() const override { return this; }
            CameraOp* cameraOp()       override { return this; }

      /*! Return a default camera that can be used when the there is no camera
          attached to your renderer. This camera is at the origin, looking along
          -Z, and has a lens ratio focal-length/film-width of 1.
      */
      static CameraOp*  default_camera();

      //! Return hints to control when and how this op will be evaluated by the top-down system.
      OpHints opHints() const override;

      //!
      unsigned node_color() const override { return 0x71C67100; }

      //!
      void _validate(bool for_real) override;

      //-----------------------------------------------------------------

      /*! Default knobs puts the transform controls on the front tab, and
          the projection, shutter, and lens controls on tab 'Projection'.
      */
      void knobs(Knob_Callback) override;

      /*! AxisOp::knobs() calls this after addTransformKnobs(), inserting the
          'Projection' tab ahead of the 'Display' tab.
          The 'Projection' tab contains the projection, shutter, and lens controls.
      */
      void  addExtraFrontPanelKnobs(Knob_Callback f) override;

      //! Adds the frustum visualization controls.
      void  addDisplayOptionsKnobs(Knob_Callback f) override;

      //! Adds the default projection controls.
      virtual void  addProjectionKnobs(Knob_Callback);

      //! Adds the default shutter controls.
      virtual void  addShutterKnobs(Knob_Callback);

      //! Adds the default lens controls.
      virtual void  addLensKnobs(Knob_Callback);

      //-----------------------------------------------------------------

      /*! Sets \a _horizontalAperture using field-of-view angle **in degrees**.
          Focal length remains the same.
      */
      void    setHorizontalFov(double angle_in_degrees);

      double  horizontalAperture() const { return _horizontalAperture; }
      void    setHorizontalAperture(double w) { _horizontalAperture = w; }

      double  verticalAperture() const { return _verticalAperture; }
      void    setVerticalAperture(double h) { _verticalAperture = h; }

      double  focalLength() const { return _focalLength; }
      void    setFocalLength(double f) { _focalLength = f; }

      double  fStop() const { return _fStop; }
      void    setfStop(double f) { _fStop = f; }

      double  nearPlaneDistance() const { return _nearPlaneDistance; }
      void    setNearPlaneDistance(double n) { _nearPlaneDistance = n; }

      double  farPlaneDistance() const { return _farPlaneDistance; }
      void    setFarPlaneDistance(double f) { _farPlaneDistance = f; }

      double  focusDistance() const { return _focusDistance; }
      void    setFocusDistance(double f) { _focusDistance = f; }

      const fdk::Vec2f&  windowTranslate() const { return _windowTranslate; }
      const fdk::Vec2f&  windowScale()     const { return _windowScale; }
      double             windowRotateZ()   const { return _windowRotateZ; }

      //!
      LensProjection  projectionMode() const { return _lensProjectionMode; }
      void            setProjectionMode(LensProjection mode) { _lensProjectionMode = mode; }

      //-----------------------------------------------------------------

      static const char* projection_modes[];
      static const char* projection_modes_renderer[];

      /*! Returns a transformation to an output image due to the camera lens.
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

          For most uses you want to multiply projectionMatrix()*xform()
          and pass the result through to_format() to get the transformation
          from world space to output pixels.
       */
      const fdk::Mat4d&   projectionMatrix() const { return _projectionMatrix; }

      /*! Return a linear camera projection matrix for the projection mode arg
          using the CameraOp's current lens values (ie its focal_length,
          horizontal_aperture, etc.)

          Only the predefined LensProjection enums are implemented in the base
          class implementation, but a subclass can override this method to
          implement custom projection calculations.

          This method should be used in conjunction with isLinearProjection() to
          determine how to use the returned matrix, as not all projection modes
          can be supported by a linear matrix. For example uv and spherical, plus
          perspective/ortho projections become non linear if lens distortion is
          enabled.
          In non-linear cases the matrix returned is normally considered the 'base'
          projection for the camera based on its current lens settings.
          In other words although the projection mode may be 'perspective' and the
          matrix returned is a valid perspective projection, if lens distortion is
          enabled then the final projection is a combination of both the projection
          matrix and the distortion.
      */
      virtual fdk::Mat4d  projectionModeMatrix(LensProjection mode) const;

      //! Same as projectionMatrix() except it evaluates the matrix at an arbitrary OutputContext.
      virtual fdk::Mat4d  projectionModeMatrixAt(LensProjection       mode,
                                                 const OutputContext& context) const;

      /*! Returns true if projection mode arg can be described linearly with a
          4x4 matrix, based on the camera's current settings. This is usually only
          true for perspective and orthographic modes if no lens distortion is
          being done.

          A subclass should override this method if it's implementing custom
          projection calculations in projectionModeMatrix().

          See projectionModeMatrix() for more info.
      */
      virtual bool        isLinearProjection(LensProjection mode) const;

      //-----------------------------------------------------------------

      double lens_distort_a() const { return lens_distort_a_; }
      double lens_distort_b() const { return lens_distort_b_; }
      double lens_distort_c() const { return lens_distort_c_; }
      double lens_distort_D() const { return lens_distort_D_; }
      double lens_distortion() const { return lens_distortion_; }
      double lens_power() const { return lens_power_; }
      bool lens_distort_enabled() const { return lens_distort_enabled_; }


      /*! Returns the current lens function used to distort vertices during
          software rendering.
          CameraLensContext is passed so the camera can configure the projection
          parameters.
          Base class default implementation calls LensProjectionFunction() for the
          standard (built-in) lens functions.
       */
      virtual LensProjectionFunc*  lensProjectionFunction(LensProjection     mode,
                                                          const Format*      format,
                                                          CameraLensContext& context) const;

      //-----------------------------------------------------------------

      /*! Try to import camera prim data from the provided stage. Will return 1(error) if
          stage is invalid. If \a prim_path is empty then -1 will be returned indicating
          no change has occured, otherwise if \a prim_path is invalid or not present in
          stage then 1(error) will be returned.

          This will call AxisOp::importScenePrim() to import the XformablePrim data
          then import the CameraPrim attributes.

          Return codes:
          -1 - no prim data was loaded and no error, usually due to empty prim path
           0 - prim data was loaded with no error
           1 - there was an error and no prim data was loaded
      */
      int  importScenePrim(const usg::StageRef& stage,
                           const std::string&   prim_path) override;

      //-----------------------------------------------------------------

      /*! Multiply the passed matrix by the necessary transformation to put
          x=-1 at the left edge of the \a format, +1 at the right, and to
          center it vertically, and compensate for any pixel aspect. The
          output z and w are left unchanged.

          Most Iops will need to do this using their output format to get the
          actual transformation from input space to output space. You would
          do this first, then multiply by the projection*local*parent.
       */
      static void toFormat(fdk::Mat4f&, const Format*);

      /*! Opposite transformation of to_format(). Multiplies the passed
          matrix by a transformation that scales the input format to go from
          x = -1 to x = 1 and to center it vertically around y = 0, and to
          remove any pixel aspect ratio. You multiply by this matrix last.

          The 3DCard iop scales further so the image covers a range so that
          if the camera lens and "taking lens" are equal the image fills the
          field of view.
       */
      static void fromFormat(fdk::Mat4f&, const Format*);

      //-----------------------------------------------------------------

      //!
      HandlesMode  doAnyHandles(ViewerContext* ctx) override;

      //!
      void  build_handles(ViewerContext*) override;

      //! Draws a lovely camera, I think it is a Mitchell.
      void  draw_handle(ViewerContext*) override;

      //! Draw the camera's frustum outline solid or wireframe, dashed if 'dashed_lines'=true.
      virtual void  drawCameraIcon(bool solid,
                                   bool dashed_lines=false);


      //! Draw the node name at position 0,0,0 which will be the local center.
      void          drawNodeName(const char* subcam_name = "");

      //! Draw the node name at an xyz position \a P relative to the local center.
      void          drawNodeName(const fdk::Vec3d& P,
                                 const char*       subcam_name = "");

      //! Draw the aperture rectangle in millimeters, centered on xyz position \a P.
      void          drawAperture(const fdk::Vec3d& P,
                                 bool              dashed_lines = false);

      //! Draw the camera's frustum.
      void          drawSolidFrustum(const fdk::Vec3d& near_corner,
                                     const fdk::Vec3d& far_corner);
      //! Draw the camera's frustum outline, dashed if 'dashed_lines'=true.
      void          drawWireFrustum(const fdk::Vec3d& near_corner,
                                    const fdk::Vec3d& far_corner,
                                    bool              dashed_lines = false);

      //-----------------------------------------------------------------


#if kDDImageVersionMajorNum <= 15
    protected:
      //----------------------------------------------------------------------------
      // Deprecated single-precision matrices - these are derived from the
      // new double-precision matrices. Will disappear in Nuke 15!
      //----------------------------------------------------------------------------
      mFnDeprecatedInNuke14("projection_ has been deprecated, use Mat4d _projectionMatrix instead.")
        Matrix4 projection_;     //!< Camera's projection matrix - deprecated

    public:
      //----------------------------------------------------------------------------
      // Deprecated functions - will disappear in Nuke 15!
      //----------------------------------------------------------------------------
      enum
      mFnDeprecatedInNuke14("Enumeration deprecated, use enum class LensProjection instead.")
      {
        LENS_PERSPECTIVE = 0, //!< Standard perspective projection
        LENS_ORTHOGRAPHIC, //!< Standard orthographic projection
        LENS_UV,          //!< Replace screen XY with UV coordinate
        LENS_SPHERICAL,   //!< 360x180 world spherically projected around camera
        //
        LENS_RENDER_CAMERA, //!< Renderer uses this option
        LENS_USER_CAMERA  = LENS_RENDER_CAMERA + 1//!< Camera user projection
      };
      //-----------------------------
      mFnDeprecatedInNuke14("Class() has been deprecated, implement on subclass plugin instead.")
        const char* Class() const override { return "Camera"; }
      mFnDeprecatedInNuke14("node_help() has been deprecated, implement on subclass plugin instead.")
        const char* node_help() const override { return ""; }
      mFnDeprecatedInNuke14("projection_knobs() has been deprecated, use addProjectionKnobs() instead.")
        virtual void projection_knobs(Knob_Callback f) { addProjectionKnobs(f); }
      mFnDeprecatedInNuke14("lens_knobs() has been deprecated, use addDistortionKnobs() instead.")
        virtual void lens_knobs(Knob_Callback f) { addLensKnobs(f); }
      //-----------------------------
      mFnDeprecatedInNuke14("projection() has been deprecated, use projectionMatrix() instead")
        const Matrix4& projection() const;
      mFnDeprecatedInNuke14("hfov(double) has been deprecated, use setHorizontalFov() instead")
        void hfov(double v) { setHorizontalFov(v); }
      mFnDeprecatedInNuke14("film_width() has been deprecated, use horizontalAperture() instead")
        double  film_width() const { return _horizontalAperture; }
      mFnDeprecatedInNuke14("film_height() has been deprecated, use verticalAperture() instead")
        double  film_height() const { return _verticalAperture; }
      mFnDeprecatedInNuke14("focal_length() has been deprecated, use focalLength() instead")
        double  focal_length() const { return _focalLength; }
      mFnDeprecatedInNuke14("Near() has been deprecated, use nearPlaneDistance() instead")
        double  Near() const { return _nearPlaneDistance; }
      mFnDeprecatedInNuke14("Far() has been deprecated, use farPlaneDistance() instead")
        double  Far() const { return _farPlaneDistance; }
      mFnDeprecatedInNuke14("focal_point() has been deprecated, use focusDistance() instead")
        double  focal_point() const { return _focusDistance; }
      mFnDeprecatedInNuke14("fstop() has been deprecated, use fStop() instead")
        double  fstop() const { return _fStop; }
      mFnDeprecatedInNuke14("film_width(double) has been deprecated, use setHorizontalAperture() instead")
        void film_width(double v) { _horizontalAperture = v; }
      mFnDeprecatedInNuke14("film_height(double) has been deprecated, use setVerticalAperture() instead")
        void film_height(double v) { _verticalAperture = v; }
      mFnDeprecatedInNuke14("focal_length(double) has been deprecated, use setFocalLength() instead")
        void focal_length(double v) { _focalLength = v; }
      mFnDeprecatedInNuke14("Near(double) has been deprecated, use setNearPlaneDistance() instead")
        void Near(double v) { _nearPlaneDistance = v; }
      mFnDeprecatedInNuke14("Far(double) has been deprecated, use setFarPlaneDistance() instead")
        void Far(double v) { _farPlaneDistance = v; }
      mFnDeprecatedInNuke14("focal_point(double) has been deprecated, use setFocusDistance() instead")
        void focal_point(double v) { _focusDistance = v; }
      mFnDeprecatedInNuke14("fstop(double) has been deprecated, use setfStop() instead")
        void fstop(double v) { _fStop = v; }
      mFnDeprecatedInNuke14("win_translate() has been deprecated, use windowTranslate() instead")
        const Vector2& win_translate() const { return reinterpret_cast<const Vector2&>(_windowTranslate); }
      mFnDeprecatedInNuke14("win_scale() has been deprecated, use windowScale() instead")
        const Vector2& win_scale() const { return reinterpret_cast<const Vector2&>(_windowScale); }
      mFnDeprecatedInNuke14("win_roll() has been deprecated, use windowRotateZ() instead")
        double win_roll() const { return _windowRotateZ; }
      mFnDeprecatedInNuke14("projection_mode() has been deprecated, use projectionMode() instead")
        int projection_mode() const { return static_cast<int>(_lensProjectionMode); }
      mFnDeprecatedInNuke14("projection_mode(int) has been deprecated, use setProjectionMode() instead")
        void projection_mode(int i) { _lensProjectionMode = static_cast<LensProjection>(i); }
      mFnDeprecatedInNuke14("projection_is_linear(int) has been deprecated, use isProjectionModeLinear(LensProjection) instead")
        virtual bool projection_is_linear(int mode) const;
      mFnDeprecatedInNuke14("projection(int) has been deprecated, use projectionModeMatrix(LensProjection) instead")
        virtual Matrix4 projection(int mode) const;
      mFnDeprecatedInNuke14("projectionAt(OutputContext) has been deprecated, use projectionModeMatrixAt(LensProjection, OutputContext) instead")
        virtual Matrix4 projectionAt(const OutputContext&);
      //-----------------------------
      typedef void (LensFunc)(Scene*, CameraOp*, MatrixArray* transforms,
                              const VArray& v, Vector4& out, void* data);
      mFnDeprecatedInNuke14("lens_function(int mode) has been deprecated.")
        virtual LensFunc* lens_function(int mode) const;

      typedef void (LensNFunc)(Scene*, CameraOp*, MatrixArray* transforms,
                               VArray* vert_array, int n, void* data);
      mFnDeprecatedInNuke14("lensNfunction(int mode) has been deprecated.")
        virtual LensNFunc* lensNfunction(int mode) const;

      mFnDeprecatedInNuke14("to_format has been deprecated, use toFormat instead.")
      static void to_format(Matrix4&, const Format*);

      mFnDeprecatedInNuke14("from_format has been deprecated, use fromFormat instead.")
      static void from_format(Matrix4&, const Format*);

      //----------------------------------------------------------------------------
      //----------------------------------------------------------------------------
#else
#error "Remove the above code block in Nuke 16 as they are deprecated in Nuke 14."
#endif
    };

  }
}

#endif
