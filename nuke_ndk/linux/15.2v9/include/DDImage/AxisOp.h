//------------------------------------------------------------------------------
// DDImage/AxisOp.h
//
// Copyright (c) 2021 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef DDIMAGE_AXISOP_H
#define DDIMAGE_AXISOP_H

#include "DDImage/GeomOp.h"

#include "DDImage/Knob.h"
#include "DDImage/ArrayKnobI.h"
#if kDDImageVersionMajorNum <= 15
#  include "DDImage/LookAt.h"
#  include "DDImage/Matrix4.h"
#else
#error "Remove the above code block in Nuke 16 as they are deprecated in Nuke 14."
#endif

#include "fdk/math/Mat4.h"

namespace usg {
  class XformablePrim;
}

namespace DD {
  namespace Image {
    class AxisOp;
    class Axis_KnobI;


    //----------------------------------------------------------------------

#if kDDImageVersionMajorNum <= 15
    class DDImage_API Axis
    {
    public:
      // P is the ray location, X, Y and Z are axis normals:
      Vector3 p, x, y, z;

      mFnDeprecatedInNuke14("Axis class has been deprecated, use Mat4 and its xAxis()/yAxis()/zAxis() methods instead")
      Axis() :
        p(0.0f, 0.0f, 0.0f),
        x(1.0f, 0.0f, 0.0f),
        y(0.0f, 1.0f, 0.0f),
        z(0.0f, 0.0f, 1.0f)  {}

      void print_info(const char* title);
    };
#else
#error "Remove the above code block in Nuke 16 as they are deprecated in Nuke 14."
#endif

    //----------------------------------------------------------------------


    /*! Provides a world-space 3D point.
    */
    class WorldPointProvider : public ArrayKnobI::ValueProvider
    {
      AxisOp* _axisOp;

    public:
      WorldPointProvider(AxisOp* axisOp) : _axisOp(axisOp) {}

      bool provideValuesEnabled(const ArrayKnobI* arrayKnob, const OutputContext&) const override;

      std::vector<double> provideValues(const ArrayKnobI* arrayKnob, const OutputContext&) const override;
    };


    /*! Provides values for the AxisOp \a 'world_matrix' knob.
    */
    class DDImage_API WorldMatrixProvider : public ArrayKnobI::ValueProvider
    {
      AxisOp* _axisOp;

    public:
      WorldMatrixProvider(AxisOp* axisOp) : _axisOp(axisOp) {}

      bool provideValuesEnabled(const ArrayKnobI* arrayKnob, const OutputContext&) const override;

      std::vector<double> provideValues(const ArrayKnobI* arrayKnob, const OutputContext&) const override;
    };


    //---------------------------------------------------------------------


    /*! \class DD::Image::AxisOp

        \brief AxisOp is the base class for all the 3D 'scene' objects like
        locators (usg::XformPrim), cameras (usg::CameraPrim) and lights (usg::tbd).

        It contains a local transform matrix and an optional local-parent transform
        matrix. The local-parent enables an arbitrary parenting hierarchy to be
        decomposed and contained within the AxisOp allowing the node to be disconnected
        or disassociated from a parenting network or scenegraph. The node is
        effectively localized and can be copied, pasted into other scripts, etc.
     */
    class DDImage_API AxisOp : public GeomOp
#if kDDImageVersionMajorNum <= 15
                              , public LookAt
#else
#error "Remove the above code block in Nuke 16 as they are deprecated in Nuke 14."
#endif
    {
    protected:
      std::string   _primPath;                    //!< Path + name for the *output* prim to create (if needed)
      int32_t       _parentPrimType;              //!< The type for missing parent prims we create
      bool          _importEnabled;               //!< If true this node imports prim attributes from scene input or file source
      bool          _passthroughEnabled;          //!< If true this node does not create or change any input scene prims
      bool          _mergeEnabled;                //!< If true any input scenegraph is merged into the output scenegraph
      //
      struct ScenePrimImport
      {
        int32_t     sourceMode;                   //!< File path(0) or scene node input(1)
        const char* filePath;                     //!< External path to scene file to use
        int32_t     nodeInputIndex;               //!< Node input to get stage from. Defaults to -1
        int32_t     version;                      //!< Incremented when reload button is pushed
        //
        std::string primPath;                     //!< Path + name for the prim to import
        //
        bool        lockReadFrame;                //!< Whether to use the manually-set frame number
        double      readFrame;                    //!< Manually set the frame number to read
        double      framesPerSecond;              //!<
        double      frameOffset;                  //!< Offset the incoming frame range (after any frame rate change)
        double      frameOrigin;                  //!< Origin of the incoming frame range
        bool        lockReadView;                 //!< Whether to use the manually-set view
        int32_t     readView;                     //!< Manually set the view to read
        //
        fdk::XformOrder  decomposeXformOrder;     //!< Preferred decompose xform order
        fdk::RotateOrder decomposeRotOrder;       //!< Preferred decompose rotation order
        bool        translationsEnable;           //!< Enable loading of translations
        bool        rotationsEnable;              //!< Enable loading of rotations
        bool        scalesEnable;                 //!< Enable loading of scale
        bool        eulerFilterEnable;            //!< Enable euler filter on rotations
        bool        parentExtractEnable;          //!< Split the parent xform out from the local
        bool        syncLocalParent;              //!< Local-parent read locked
        bool        syncLocalAxis;                //!< Local-axis read locked
        //
        fdk::Hash   previousImportHash;           //!< Does the scene prim need importing?
        bool        error;                        //!< Error was thrown attempting to import a scene prim
        std::string errorMsg;                     //!< Error msg saved
      }             _importer;                    //!< Scene prim importer controls & state
      bool          _importerChanged;             //!< If true any knobs set by the importer need to be re-stored
      //
      fdk::Vec3d    _localParentTranslate;        //!< Local-parent translation knob value
      fdk::Vec3d    _localParentRotate;           //!< Local-parent rotate knob value
      fdk::Vec3d    _localParentScale;            //!< Local-parent scale knob value
      fdk::Mat4d    _localParentMatrix;           //!< Parent transform derived from local parent TRS knobs
      //
      fdk::Mat4d    _localAxisMatrix;             //!< Local matrix that Axis_Knob fills in (no lookat rotations)
      //
      fdk::Mat4d    _parentTransform;             //!< Input-parent & local-parent transform
      fdk::Mat4d    _localTransform;              //!< Local object transform - axis_matrix & lookat rotations
      fdk::Mat4d    _worldTransform;              //!< World transform - _parentTransform & _localTransform
      fdk::Mat4d    _invWorldTransform;           //!< Inverse world transform

      DD::Image::Hash _parentHash;                //!< Does parent path or transform need to be updated?

#if kDDImageVersionMajorNum <= 15
      //----------------------------------------------------------------------------
      // Deprecation unions to retain backward-compatibility in Nuke 14
      // The old var names will disappear in Nuke 15!
      //----------------------------------------------------------------------------
      union {
        bool        _xformInversionValid;         //!< Is _invWorldTransform valid?
        mFnDeprecatedInNuke14("inversion_updated has been deprecated, use _xformInversionValid instead.")
        bool inversion_updated;    //!< Use _xformInversionValid instead
      };
      union {
        Axis_KnobI* _axisKnob;                    //!< Reference to the node Axis_Knob
        mFnDeprecatedInNuke14("axis_knob has been deprecated, use _axisKnob instead.")
        Axis_KnobI* axis_knob;     //!< Use _axisKnob instead
      };
#endif
      WorldMatrixProvider* _worldMatrixProvider;  //!< 'world_matrix' output knob
      //
      Knob*        _localParentTranslateKnob;     //!< If not null apply parent_translate
      Knob*        _localParentRotateKnob;        //!< If not null apply parent_rotate
      Knob*        _localParentScaleKnob;         //!< If not null apply parent_scale

      // TODO: Possibly deprecate as these are also on GeomOp:
      int   display3d_;                           //!< GUI display setting      - deprecate?
      bool  selectable_;                          //!< GUI selectable checkmark - deprecate?
      //
      bool  _editable;                            //!< Can the user change the knobs? (i.e. knobs not disabled)


    protected:
      class DDImage_API AxisEngine : public GeomOpEngine
      {
      public:
        AxisEngine(Op* parent);

        //!
        AxisOp* axisOp() const { return op_cast<AxisOp*>(_firstOp); }

        //! Returns a representative Op for the parent input at no particular time or view.
        //! Use this to directly query Knob objects but -NOT- to retrieve any Op values
        //! derived from validation.
        AxisOp* parentInputOp();

        //! Initializes uniform values.
        void _validateGeomState() override;
        //!
        void  processScenegraph(usg::GeomSceneContext& context) override;

        bool  importEnabled() const { return _importMode; }
        bool  passthroughEnabled() const { return _passthroughMode; }
        bool  mergeSceneEnabled() const { return _mergeScene; }
        int   parentPrimType() const { return _parentPrimType; }


        //! Return the name of the prim created by this node.
        const std::string&  primName() const { return _resolvedPrimPath.name(); }

        //! Return the full path of this prim and all its parents.
        const usg::Path&    primFullPath() const { return _resolvedPrimPath; }


      protected:
        friend class AxisOp; // allow AxisOp to set engine vals

        // Targets are per-engine so they represent layer/stage state
        usg::GeomStateTarget _defineXformTarget;    //!<
        usg::GeomStateTarget _overrideXformTarget;  //!< 'override-xform' - affects the override-geometry target

        bool       _importMode;                 //!<
        bool       _passthroughMode;            //!< If true this node does not create or edit any input scene prims
        bool       _mergeScene;                 //!<
        int        _parentPrimType;             //!<

        usg::Path  _resolvedPrimPath;           //!< Fully expanded path, includes any parent connections
        fdk::Hash  _resolvedPrimPathHash;       //!< Does prim name and path need to be rebuilt?

      };


    public:
      AxisOp(Node*                 node,
             GeomOpEngine::Builder engine_ctor,
             int   defaultLookAtAxis = LookAt::kAxisZPlus);

#if kDDImageVersionMajorNum <= 15
      //! Backwards-compatible ctor.
      AxisOp(Node* node,
             int   defaultLookAtAxis = LookAt::kAxisZPlus);
#else
#error "Remove the above code block in Nuke 16 as they are deprecated in Nuke 14."
#endif

      virtual ~AxisOp();

      //! Cast this Op to an AxisOp. This is much cheaper and safer than using dynamic_cast.
      const AxisOp* axisOp() const override { return this; }
            AxisOp* axisOp()       override { return this; }

      //! Convenience that casts parent input 0 to an AxisOp.
      AxisOp* parentInputOp() const { return op_cast<AxisOp*>(Op::input(0)); }

      //! Convenience that casts input sceneImporterInput() to a GeomOp.
      GeomOp* sceneInputOp() const { return op_cast<GeomOp*>(Op::input(sceneImporterInput())); }

      //! Return the node input to use for the parent axis source. Always defaults to input 0.
      int32_t          parentInput() const { return 0; }

      //! Return the node input to use for the scene importer. Default returns optional_input().
      virtual int32_t  sceneImporterInput() const { return optional_input(); }

      bool can_link() const override { return false; }

      //! Draw a circular node shape.
      const char*  node_shape() const override;
      unsigned     node_color() const override;

      //! Default is 2: axis and scene(optional)
      int          minimum_inputs() const override;
      //! Default is 2: axis and scene(optional)
      int          maximum_inputs() const override;
      //! Optional input index is the scene-importer input. This returns (maximum_inputs()-1).
      int          optional_input() const override;

      //! AxisOp and null works on 0, and GeomOp or null on input 1.
      bool         test_input(int node_input, Op* op) const override;

      //! Returns null for all inputs. Override this so Op::test_input() doesn't produce warnings.
      Op*          default_input(int node_input) const override;

      //!
      const char*  input_label(int node_input, char*) const override;


#if kDDImageVersionMajorNum <= 15
      /*! *** Nuke14 only *** virtual method is used to indicate that the AxisOp
          subclass is a legacy one or not.

          The new GeomOp-based AxisOp plugins return false, changing the behavior
          of the AxisOp base class methods like knobs(), _validate(), test_input(),
          minimum_inputs(), maximum_inputs(), etc so old plugins building against
          AxisOp get the same behavior.

          Existing AxisOp subclass plugins like 'Axis2', 'Axis3', 'Camera2', 'Camera3',
          'Light2', 'Light3', etc will retain their current behaviors through Nuke14,
          but those plugins will be removed in Nuke15, and existing scripts using them
          will no longer work.
      */
      mFnDeprecatedInNuke14("isLegacyAxisOp() has been deprecated, port AxisOp sublass to new Nuke 14 api.")
      bool isLegacyAxisOp() const;
#else
#error "Remove the above code block in Nuke 16 as they are deprecated in Nuke 14."
#endif


      //----------------------------------------------------------------------------
      // Scene Prim handling
      //----------------------------------------------------------------------------

      //! Return the name of the prim created by this node. Stored in AxisEngine.
      const std::string&  primName() const;

      //! Return the full path of this prim and all its parents. Stored in AxisEngine.
      const usg::Path&    primFullPath() const;


      /*! Check if the imported prim needs to be loaded or reloaded.
          If the prim needs reloading then importScenePrim() is called
          with the correct stage and prim path.

          If \a force is true then the prim will always be reloaded.

          Return codes:
          -1 - no prim data was loaded and no error (i.e. no change)
           0 - prim data was loaded with no error
           1 - there was an error and no prim data was loaded

          If error was returned (1) check the error state and message in ScenePrimImport.
      */
      int          updateScenePrim(bool force = false);

      /*! Try to import prim data from the provided stage. Will return 1(error) if
          stage is invalid. If \a prim_path is empty then -1 will be returned indicating
          no change has occured, otherwise if \a prim_path is invalid or not present in
          stage then 1(error) will be returned.

          Return codes:
          -1 - no prim data was loaded and no error, usually due to empty prim path
           0 - prim data was loaded with no error
           1 - there was an error and no prim data was loaded
      */
      virtual int  importScenePrim(const usg::StageRef& stage,
                                   const std::string&   prim_path);

      /*! Retrieve the attribute values at times possibly modified by the importer
          time options. If there are no time samples in the attribute then only a
          single default-time value is returned, unaffected by the time options.
          If the output time of the samples have been affected the \a times array
          reflects the *output* time and not the time the attribute was sampled at.

          If the importer is set to frame lock then only a single value is set,
          sampled at the lock time, regardless of the animation status of the
          attribute.

          This method will always set at least one time and value if the attrib
          is valid.
          Returns the number of value/times set, or 0 if there was a problem.
      */
      template <typename T>
      size_t  getAttribValuesTimeWarped(const usg::Attribute& attr,
                                        double                attr_fps,
                                        fdk::TimeValueList&   times,
                                        std::vector<T>&       values) const;

      /*! Retrieve the attribute values at times possibly modified by the importer time
          options and save them into the target Knob.
          If \a force_keys is true then the Knob will have a keyframe set even if the
          sample attribute only has a default value (i.e. no time sample.)
          This likely only works for ArrayKnobs since they support animation.
          See Knob::set() for explanation of \a channel_start_offset and \a view options.
          Returns true on success.
      */
      template <typename T>
      bool    copyAttribToKnobTimeWarped(const usg::Attribute& attr,
                                         double                attr_fps,
                                         Knob*                 knob,
                                         bool                  force_keys,
                                         int32_t               channel_start_offset,
                                         int32_t               view) const;

       /*! Generate output time values from an input time list using the current importer
          settings to possibly time warp / offset them.
          If no time change needed the returned times will be the input times.
          \a input_fps is only needed if this is changing the frame rate of the input
          samples. If left at the 0.0 default then no time warping is applied.
      */
      fdk::TimeValueList  getOutputSampleTimes(const fdk::TimeValueList& input_times,
                                               double                    input_fps = 0.0) const;

      //----------------------------------------------------------------------------
      // Knobs
      //----------------------------------------------------------------------------

      //!
      void  knobs(Knob_Callback f) override;

      //! create the Axis Op Knobs and set a custom filter to use on the popup
      void createAxisOpKnobs(Knob_Callback f, const std::string& filters = "");

      //! Called by Nuke immediately AFTER Op::knobs(). We use this to update the importer values.
      void  setOutputContext(const OutputContext& context) override;

      //!
      int   knob_changed(Knob* k) override;

      //! Adds the node name to the node hash for invalidation on changes.
      void  appendGeomState(DD::Image::Hash& op_hash) override;

      //! Adds the OpenGL display option controls.
      virtual void  addDisplayOptionsKnobs(Knob_Callback f);

      //! Add the file path and import enable knobs.
      virtual void  addFileImportKnobs(Knob_Callback f);

      //! Adds the create/import, prim path, etc knobs.
      virtual void  addCreateOptionsKnobs(Knob_Callback f, const std::string& filters = "");

      /*! Adds the scene import controls. If \a add_xform_knobs is true the
          knobs to control local parent extraction, matrix decomposition options, etc
          are added.
      */
      virtual void  addScenePrimImporterKnobs(Knob_Callback f,
                                              bool          group_open,
                                              bool          add_time_options,
                                              bool          add_xform_options);

      /*! Add the local-parent TRS knobs. If \a add_sync_options is true the
          import-sync options are added.
      */
      virtual void  addLocalParentingKnobs(Knob_Callback f,
                                           bool          add_sync_options = true);

      /*! Adds the front panel local-axis transform knobs. If \a add_sync_options is
          true the import-sync options are added.
      */
      virtual void  addLocalXformKnobs(Knob_Callback f,
                                       bool          add_sync_options = true);

      /*! Adds the front panel local-parent & local-axis transform knobs. If
          \a add_sync_options is true the import-sync options are added.
          Calls addLocalParentingKnobs() then addLocalXformKnobs(), adding dividers.
      */
      virtual void  addTransformKnobs(Knob_Callback f,
                                      bool          add_sync_options = true);

      //! Return internal 'transform' Axis_Knob for set/get internal transformation.
      Axis_KnobI*   getAxis();

      //! Adds addl front-panel knobs. Called after addTransformKnobs(). Base class adds nothing.
      virtual void  addExtraFrontPanelKnobs(Knob_Callback f);

      //-----------------------------

      /*! Enable or disable axis knobs.
          \param on true to enable the axis knobs ('transform')
      */
      void  enableAxisKnobs(bool on);

      /*! Enable or disable knobs based on array of knobnames.
          \param knobNames knob names to enable or disable
          \param on true to enable the knobs
      */
      void  enableKnobs(const std::vector<std::string>& knobNames, bool on);

      //! Enable/disable knobs filled in by the scene importer.
      virtual void  enableScenePrimImporterKnobs(bool scene_import_enabled);
      virtual void  enableScenePrimImporterExtraKnobs(bool scene_import_enabled);
      virtual void  enableXformExtractKnobs(bool xform_extract_enabled);

      //----------------------------------------------------------------------------
      // Transforms
      //----------------------------------------------------------------------------

      /*! Validate the parent axis first, if any, then concatenate the local transform
          with that.  If the resulting matrix has changed, update the inversion flag.
      */
      void _validate(bool for_real) override;

      // Input-parent transform if connected to a valid parent input source.
      const fdk::Mat4d&   inputParentTransform() const;

      //! Result of the local-parent TRS knobs, if enabled. Calculated in _validate().
      const fdk::Mat4d&   localParentTransform() const { return _localParentMatrix; }

      //! Result of the local-axis TRS Axis_Knob (*without* lookat rotations.) Calculated in knobs().
      const fdk::Mat4d&   localAxisTransform() const { return _localAxisMatrix; }

      //! Parent transform containing the input-parent and local-parent. Calculated in _validate().
      const fdk::Mat4d&   parentTransform() const { return _parentTransform; }

      //! Local object transform containing the local-axis TRS Axis_Knob *with* lookat rotations. Calculated in _validate().
      const fdk::Mat4d&   localTransform() const { return _localTransform; }

      //! World transform containing the input-parent, local-parent, local-axis, and lookat. Calculated in _validate().
      const fdk::Mat4d&   worldTransform() const { return _worldTransform; }

      /*! Return the inverse world transform. If \a _worldTransform has been marked invalid
          the _invWorldTransform will be calculated and updated now.
       */
      const fdk::Mat4d&   invWorldTransform();

      /*! Calculate the world-space transform of this node at a specific OutputContext.
          Calls Knob::store() on all the transform knobs but discards the results, so this
          method can be more expensive than creating an AxisOp at the OutputContext and
          reusing it.
      */
      virtual fdk::Mat4d  worldTransformAt(const OutputContext& context) const;

      /*! Replace the output matrix derived in _validate(), also invalidating the inverse
          world transform. Note this only persists until the next validate() pass
          overwrites the derived matrices.
      */
      void                setWorldTransform(const fdk::Mat4d&);

      //----------------------------------------------------------------------------

      int display3d() const { return display3d_; }
      int display3d()       { return display3d_; }
      // only constructors should call this, unless knobs() is not called:
      void setDisplay3d(int mode) { display3d_ = mode; }

      bool selectable() const override { return selectable_; }
      void setSelectable(bool v) { selectable_ = v; }

      bool editable() const { return _editable; }
      void setEditable(bool v) { _editable = v; }

      //! Return true if a particular knob transformation is modifiable by user interaction in gui.
      virtual bool isGuiInteractive(Axis_KnobI::Transformation knob) const;

      //----------------------------------------------------------------------------

      //!
      HandlesMode  doAnyHandles(ViewerContext*) override;

      /*! This default version will always cause draw_handle() to be called
          when in 3D mode. Subclasses (i.e. the camera) should replace draw_handle
          with their own version to draw what they want.
      */
      void         build_handles(ViewerContext*) override;

      /*! Draws any geometry attached to this axis.
          Note that the Axis knob will draw the 3-arrow axis control at its
          pivot-translate location.
      */
      void         draw_handle(ViewerContext*) override;

      //----------------------------------------------------------------------------

#if kDDImageVersionMajorNum <= 15
    protected:
      /*! *** Nuke14 only *** virtual method is used to indicate that the AxisOp
          subclass is a legacy one or not.
      */
      mFnDeprecatedInNuke14("isLegacyAxisOp() has been deprecated, port AxisOp sublass to new Nuke 14 api.")
      virtual bool _isLegacyAxisOp() const { return true; }

    protected:
      //----------------------------------------------------------------------------
      // Deprecated single-precision matrices - these are derived from the
      // new double-precision matrices. Will disappear in Nuke 15!
      //----------------------------------------------------------------------------
      mFnDeprecatedInNuke14("localtransform_ has been deprecated, use Mat4d _localTransform instead.")
        Matrix4 localtransform_;    //!< Local 'axis' matrix that Axis_Knob fills in - deprecated
      mFnDeprecatedInNuke14("local_ has been deprecated, use Mat4d _localAxisMatrix instead.")
        Matrix4 local_;             //!< Local matrix after look at performed - deprecated
      mFnDeprecatedInNuke14("matrix_ has been deprecated, use Mat4d _worldTransform instead.")
        Matrix4 matrix_;            //!< Object matrix - local&parent - deprecated
      mFnDeprecatedInNuke14("imatrix_ has been deprecated, use Mat4d _invWorldTransform instead.")
        Matrix4 imatrix_;           //!< Inverse object matrix - deprecated

    public:
      //----------------------------------------------------------------------------
      // Deprecated functions - will disappear in Nuke 15!
      //----------------------------------------------------------------------------
      mFnDeprecatedInNuke14("Class() has been deprecated, implement on subclass plugin instead.")
        const char*  Class() const override { return "Axis"; }
      mFnDeprecatedInNuke14("node_help() has been deprecated, implement on subclass plugin instead.")
        const char*  node_help() const override { return ""; }
      //-----------------------------
      mFnDeprecatedInNuke14("selectable(bool mode) has been deprecated, use setSelectable() instead.")
        void  selectable(bool v) { setSelectable(v); }
      mFnDeprecatedInNuke14("display3d(int mode) has been deprecated, use setDisplay3d() instead.")
        void  display3d(int mode) { setDisplay3d(mode); }
      //-----------------------------
      mFnDeprecatedInNuke14("input0() has been deprecated, use parentInput() instead.")
        AxisOp*        input0() const { return op_cast<AxisOp*>(Op::input(0)); }
      mFnDeprecatedInNuke14("lookAtEnabled() has been deprecated")
        virtual bool   lookAtEnabled() const;
      mFnDeprecatedInNuke14("lookat_input() has been deprecated")
        Op*            lookat_input() const override;
      //-----------------------------
      mFnDeprecatedInNuke14("matrix(const Matrix4&) has been deprecated, use setWorldTransform() instead.")
        void            matrix(const Matrix4&);
      mFnDeprecatedInNuke14("local() has been deprecated, use localTransform() instead.")
        const Matrix4&  local() const;
      mFnDeprecatedInNuke14("matrix() has been deprecated, use worldTransform() instead.")
        const Matrix4&  matrix() const;
      mFnDeprecatedInNuke14("imatrix() has been deprecated, use invWorldTransform() instead.")
        const Matrix4&  imatrix();
      mFnDeprecatedInNuke14("matrixAt() has been deprecated, use worldTransformAt() instead.")
        virtual void    matrixAt(const OutputContext&, Matrix4&) const;
      mFnDeprecatedInNuke14("print_info() has been deprecated")
        virtual void    print_info(std::ostream&) const {}
      //----------------------------------------------------------------------------
      //----------------------------------------------------------------------------
#else
#error "Remove the above code block in Nuke 16 as they are deprecated in Nuke 14."
#endif

      //! Not implemented in the library but you can define it
      friend std::ostream& operator<<(std::ostream&, const AxisOp&);
    };



    template <typename T>
    inline size_t
    AxisOp::getAttribValuesTimeWarped(const usg::Attribute& attr,
                                      double                attr_fps,
                                      fdk::TimeValueList&   times,
                                      std::vector<T>&       values) const
    {
      times.clear();
      values.clear();
      if (!attr.isValid()) {
        return 0;
      }
      if (_importer.lockReadFrame) {
        times.push_back(_importer.readFrame);
        values.resize(1);
        attr.getValue(values[0], _importer.readFrame);
        return 1;
      }
      const size_t n = attr.getTimeSamples(times);
      if (n == 0) {
        // No time samples, get default value:
        fdk::setNotAnimated(times); // sets 1 default time entry
        values.resize(1);
        attr.getValue(values[0]);
        return 1;
      }
      values.resize(n);
      for (size_t i=0; i < n; ++i) {
        const double sample_time = times[i];
        attr.getValue(values[i], sample_time); // sample at original time
        // Possibly time-warp output time:
        const double input_time  = (sample_time - _importer.frameOrigin) / _importer.framesPerSecond;
        const double output_time = (input_time * attr_fps) + _importer.frameOrigin + _importer.frameOffset;
        times[i] = output_time;
      }
      return n;
    }


    template <typename T>
    inline bool
    AxisOp::copyAttribToKnobTimeWarped(const usg::Attribute& attr,
                                       double                attr_fps,
                                       Knob*                 knob,
                                       bool                  force_keys,
                                       int32_t               channel_start_offset,
                                       int32_t               view) const
    {
      if (knob) {
        fdk::TimeValueList times;
        std::vector<T> values;
        if (getAttribValuesTimeWarped(attr, attr_fps, times, values) > 0) {
          if (force_keys && times.size() == 1 && fdk::isDefaultTimeValue(times[0])) {
            times[0] = _importer.frameOrigin; // force knob to create a keyframe
          }
          return knob->set(true/*clear_keys*/, values, times, channel_start_offset, view); // copy values into Knob
        }
      }
      return false;
    }

  }
}
#endif
