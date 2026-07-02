// AxisOp.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_AxisOp_h
#define DDImage_AxisOp_h

#include "Op.h"
#include "Box3.h"
#include "Knob.h"
#include "Matrix4.h"
#include "ArrayKnobI.h"
#include "LookAt.h"
namespace DD
{
  namespace Image
  {

    //----------------------------------------------------------------------

    class DDImage_API Axis
    {
    public:
      // P is the ray location, X, Y and Z are axis normals:
      Vector3 p, x, y, z;

      Axis() :
        p(0.0f, 0.0f, 0.0f),
        x(1.0f, 0.0f, 0.0f),
        y(0.0f, 1.0f, 0.0f),
        z(0.0f, 0.0f, 1.0f)  {}

      void print_info(const char* title);
    };

    class WorldMatrixProvider;

    //---------------------------------------------------------------------

    /*! \class DD::Image::AxisOp

        \brief AxisOp is the base class for all the 3D objects.  It simply
        contains a transform matrix.
     */
    class DDImage_API AxisOp : public Op, public LookAt
    {
    protected:
      Matrix4 localtransform_;    //!< Local matrix that Axis_Knob fills in
      Matrix4 local_;     //! Local matrix after look at performed
      Matrix4 matrix_;    //!< Object matrix - local&parent
      Matrix4 imatrix_;    //!< Inverse object matrix
      bool inversion_updated; //!< Whether imatrix_ is valid
      Axis_KnobI* axis_knob;  //!< reference to the transformation knob
      WorldMatrixProvider* _worldMatrixProvider;

      int display3d_;    //!< GUI display setting
      bool selectable_;    //!< GUI selectable checkmark

      /*! Validate our parent axis first, then concatenate the
          local transform with that.  If the resulting matrix has
          changed, update the inversion flag and XYZ vectors.
       */
      void _validate(bool for_real);

    public:
      AxisOp(Node*, int defaultLookAtAxis = kAxisZPlus );
      virtual ~AxisOp();

      int minimum_inputs() const { return lookAtEnabled() ? 2 : 1;}
      int maximum_inputs() const { return lookAtEnabled() ? 2 : 1;}

      virtual const char* input_label(int input, char*) const;

      /// override from LookAt to add the look at pipe to input 1
      virtual Op* lookat_input() const { return lookAtEnabled() ? (Op*)(Op::input(1)) : nullptr; }

      /// by default all the axis look at pipe is disabled
      // return true from this to enable the pipe

      // If you enable the look at pipe you should also do the following
      // Add the lookat knobs by calling LookAt::knobs()
      // Call LookAt::knobChanged so that the knobs are disabled when the pipe is not connected
      virtual bool lookAtEnabled() const { return false; }
      /*! enable or disable axis knobs
       * \param on true to enable the axis knobs (transform)
       */
      void enableAxisKnobs(bool on);
      /*! enable or disable knobs based on array of knobnames
       * \param knobNames knob names to enable or disable
       * \param on true to enable the knobs
       */
      void enableKnobs(const std::vector<std::string>& knobNames, bool on);
      /*! Only AxisOp and NULL work. */
      bool test_input(int, Op*) const;
      AxisOp* input0() const { return (AxisOp*)(Op::input0()); }

      int display3d() const { return display3d_; }
      bool selectable() const { return selectable_; }
      // only constructors should call this, unless knobs() is not called:
      void display3d(int v) { display3d_ = v; }
      void selectable(bool v) { selectable_ = v; }

      //! return true if a particular knob transformation is modifiable by user interaction in gui
      virtual bool isGuiInteractive(Axis_KnobI::Transformation knob) const;

      // get internal axis transformation for set/get internal transformation
      Axis_KnobI* getAxis();

      const char* node_shape() const;
      unsigned node_color() const;
      void knobs(Knob_Callback);

      HandlesMode doAnyHandles(ViewerContext*);

      /*! This default version will always cause draw_handle() to be called
         when in 3D mode. Subclasses (ie the camera) should replace draw_handle
         with their own version to draw what they want.
       */
      void build_handles(ViewerContext*);

      /*! Draws any geometry attached to this axis. Note that the Axis knob
         will draw the 3-arrow axis control in the center.
       */
      void draw_handle(ViewerContext*);

      //const Box3& bbox() const {return bbox_;}

      int display3d() { return display3d_; }

      const Matrix4& local()  const { return local_; }
      void matrix(const Matrix4& m) { matrix_ = m;
                                      inversion_updated = false; }
      const Matrix4& matrix() const { return matrix_; }

      /*! Return the inverse object matrix.  If the inversion has been
          marked invalid, calculate it now, otherwise return the current one.
       */
      const Matrix4& imatrix();
      virtual void matrixAt(const DD::Image::OutputContext& context, DD::Image::Matrix4& matrix) const;

      const char* Class() const;
      const char* node_help() const;
      static const Description description;
      virtual void print_info(std::ostream& o) const;

      /*! Not implemented in the library but you can define it */
      friend std::ostream& operator<<(std::ostream&, const AxisOp&);
    };

  }
}
#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
