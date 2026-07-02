// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_ENGINE_GEOMENGINE_H
#define USG_ENGINE_GEOMENGINE_H

#include "usg/engine/api.h"
#include "usg/engine/GeomState.h"
#include "usg/engine/GeomStateTarget.h"
#include "usg/engine/GeomSceneContext.h"
#include "usg/engine/EngineTokens.h"

#include "usg/geom/Stage.h"
#include "usg/geom/GeomTokens.h"
#include "usg/base/ArgSet.h"

#include <string>


namespace usg {

  class GeomSceneOutput;
  using TimeGeomStateMap = std::map<fdk::TimeValue, GeomState>;

  //--------------------------------------------------------------------------------


  /*! Scenegraph processing engine node that can return a usg::Stage.
  */
  class USG_ENGINE_API GeomEngine
  {
  public:
    GeomEngine();
    virtual ~GeomEngine();

    //! Name of the engine. Not saved locally as name can change dynamically. Must implement.
    virtual std::string  name() const =0;

    //----------------------------------------------------------------------

    //! Return the number of GeomEngine inputs. Must implement.
    virtual size_t       nInputs() const =0;

    //! Returns the GeomEngine pointer from input index. Must implement.
    virtual GeomEngine*  inputEngine(uint32_t input) =0;
    virtual GeomEngine*  inputEngine0() { return inputEngine(0); }
    virtual GeomEngine*  inputEngine1() { return inputEngine(1); }

    //----------------------------------------------------------------------
    // GeomState change targets
    //----------------------------------------------------------------------

    /*! Assign a change target to engine. It's assumed the \a target object is a
        local variable in this class and will not disappear or become invalid!
        Not thread safe.
    */
    void  assignTarget(GeomStateTarget& target);

    /*! Add a change target to engine, parented to a parent change target group.
        It's assumed the \a target object is a local variable in this class and
        will not disappear or become invalid!
        Not thread safe.
    */
    void  connectTargetToParent(GeomStateTarget&  target,
                                const usg::Token& parent_id);
    /*! Add a change target to engine, parented to an existing parent target.
        Not thread safe.
    */
    void  connectTargetToParent(GeomStateTarget& target,
                                GeomStateTarget& parent_target);

    // GeomStateTarget convenience functions:
    void  assignDefineGeometryTarget(GeomStateTarget& target) { connectTargetToParent(target, usg::EngineTokens.target.defineGeometry); }
    void  assignEditLayerTarget(GeomStateTarget& target)      { connectTargetToParent(target, usg::EngineTokens.target.editLayer); }
    void  assignModifyValuesTarget(GeomStateTarget& target)   { connectTargetToParent(target, usg::EngineTokens.target.modifyValues); }
    void  assignEditStageTarget(GeomStateTarget& target)      { connectTargetToParent(target, usg::EngineTokens.target.editStage); }


    //! Retrieve a named change target. Not thread safe.
    GeomStateTarget*  getTarget(const usg::Token& target_id) const;

    /*! Retrieve the set of change targets assigned to this engine.
        Not thread safe.
    */
    const GeomStateTargetPtrMap&  targets() const { return _changeTargetMap; }


    //! Pre-assigned target that affects GeomState::defineGeometryState
    const GeomStateTarget&  defineGeometryTarget() const { return _defineGeometryTarget; }

    //! Pre-assigned target that affects GeomState::editLayerState
    const GeomStateTarget&  editLayerTarget() const { return _editLayerTarget; }

    //! Pre-assigned target that affects GeomState::modifyValuesState
    const GeomStateTarget&  modifyValuesTarget() const { return _modifyValuesTarget; }

    //! Pre-assigned target that affects GeomState::editStageState
    const GeomStateTarget&  editStageTarget() const { return _editStageTarget; }

    //! Pre-assigned target that affects GeomState::affectsStereoState
    const GeomStateTarget&  affectsSteroTarget() const { return _affectsStereoTarget; }


    //----------------------------------------------------------------------
    // Scenegraph State
    //----------------------------------------------------------------------

    /*! Current modification version for this engine. This number should be incremented by an
        engine sublass whenever the local modification state of the engine needs to change,
        which is usually because some parameter or value changed which affects what the engine
        is producing.
    */
    int32_t  modificationVersion() const { return _modificationVersion; }

    //! Increment the modification version, returning the new value.
    int32_t  bumpModificationVersion() { return ++_modificationVersion; }

    /*! Has the global modification version number changed since the last getScenegraph() pass?

        Changing the global \a _modificationVersion indicates that parameters contributing to the
        engine's output may have changed value and the engine needs re-evaluation.

        The first call to getScenegraph() after a \a _modificationVersion change will cause the
        status of the engine's cached Layers to be checked against the current GeomState
        and the appropriate action applied, such as clearing \a _rootLayer and \a _editLayer if
        the defineGeometryState changed. In addition all current processed-time markers will be
        cleared then processScenegraph() is called so the engine can perform Layer defines, etc.

        After the initial getScenegraph() pass with a changed \a _modificationVersion, repeat
        visits with the same version number will only result in processScenegraph() getting
        called if the GeomSceneContext contains process times not already present, and the
        \a _processedVersion number getting incremented each time.
    */
    bool     hasModificationVersionChanged() const;


    //! Read access to geometry states calculated in validateGeomState().
    const GeomState&  getInputState()  const { return _inputState; }
    const GeomState&  getLocalState()  const { return _localState; }
    const GeomState&  getOutputState() const { return _outputState; }

    /*! Does the GeomEngine need to re-process?
        This could be because the inputs GeomState changed or the local GeomState changed.
        Note that this only indicates that the engine -may- need to reprocess.
        Check the other dirty indicators to determine what specifically needs updating.
    */
    bool  isDirty() const;
    //!
    bool  isDefineGeometryStateDirty() const;
    bool  isModifyValuesStateDirty() const;
    bool  isEditStageStateDirty() const;


    //! Input GeomEngine(s) have changed.
    bool  isInputStateDirty() const;
    //! Input GeomEngine's define-geometry state has changed.
    bool  isInputDefineGeometryStateDirty() const;
    //! Input GeomEngine's modify-values state has changed.
    bool  isInputModifyValuesStateDirty() const;
    //! Input GeomEngine's edit-stage state has changed.
    bool  isInputEditStageStateDirty() const;

    //! This GeomEngine's local state has changed.
    bool  isLocalStateDirty() const;
    //! This GeomEngine's local Layer define-geometry state has changed.
    bool  isLocalDefineGeometryStateDirty() const;
    //! This GeomEngine's local Layer modify-values state has changed.
    bool  isLocalModifyValuesStateDirty() const;
    //! This GeomEngine's local edit-stage state has changed.
    bool  isLocalEditStageStateDirty() const;


    //! Return true if the output define-geometry state != the input define-geometry state.
    //! Does not indicate if the engine needs to process the scene, only that the engine will
    //! be run when dirty.
    bool  mayAffectComposedStage() const;
    //! Return true if the output modify-values state != the input modify-values state.
    //! Does not indicate if the engine needs to process the scene, only that the engine will
    //! be run when dirty.
    bool  mayModifyAttributeValues() const;
    //! Return true if the output edit-stage state != the input edit-stage state.
    //! Does not indicate if the engine needs to process the scene, only that the engine will
    //! be run when dirty.
    bool  mayEditStage() const;


    /*! GeomEngine may affect prim/attribute Layer definitions, overrides, etc
        in processScenegraph(). Since these changes are stored in the local
        \a _editLayer they can be exported to a .usd file.

        This is the same as checking mayAffectComposedStage() &&
        isLocalDefineGeometryStateDirty(), and so will only be true on the first
        getScenegraph() processing pass.
    */
    bool  definesEditLayer() const;

    /*! GeomEngine may affect locally-defined Layer attribute values in
        processScenegraph(). Since these changes are stored in the local
        \a _editLayer they can be exported to a .usd file.

        This is effectively the same as checking mayModifyAttributeValues() since even
        after the first processing pass we want to know if the engine -intends- to
        modify attribute values on each subsequent process pass.
    */
    bool  modifiesEditLayerValues() const;

    /*! GeomEngine may affect the local composed Stage dynamically (applying edits)
        in editComposedStage().
        These changes are intended to be transitory/temporary and not saveable
        to a .usd file.

        An example of this would be for extra stage metadata used internally to
        affect the Hydra delegates, etc, or to manipulate any non-Stage data in
        the GeomSceneOutput.

        This is effectively the same as checking mayEditStage() since even after
        the first processing pass we want to know if the engine -intends- to edit
        stage values on each subsequent process pass.
    */
    bool  editsComposedStage() const;


    //----------------------------------------------------------------------
    // Request / Configuration State
    //----------------------------------------------------------------------


    /*! Rounds the time value to three decimal places (ie .001, a thousandth of a frame)
        to make it more useful as a key value for things like std::map, etc.
    */
    static fdk::TimeValue  trimFrame(const fdk::TimeValue& time);

    /*! The 'request' ArgSet can be used during engine evaluation to influence
        its behavior, typically set by the system managing the GeomEngine network.
        The args are not used by the GeomEngine base class.
    */
    const usg::ArgSet&  requestArgs() const { return _requestArgs; }
    //! Add request args to the engine.
    void                addRequestArgs(const usg::ArgSet& args);
    //! Empty the arg set.
    void                clearRequestArgs() { _requestArgs.clear(); }


    //----------------------------------------------------------------------
    // Scenegraph Processing
    //----------------------------------------------------------------------

    /*! Return the edit layer for the engine. Base class returns the local \a _editLayer var.
        This layer is where all local prim/attribute defines & edits are stored.
    */
    virtual usg::LayerRef  editLayer();

    /*! Return the root layer for the engine. Base class returns the local \a _rootLayer var.
        If there are local edits present in the \a _editLayer after processScenegraph() has
        run the \a edit_layer will be inserted into \a _rootLayer as the highest-priority
        sublayer (layer 0.)
    */
    virtual usg::LayerRef  rootLayer();

    /*! Called in getScenegraph() when defineGeometryState is dirty, to clear out _editLayer
        in preparation for defining prims, attribs. A subclass can implement this to perform
        custom initialization if the default clearing of _editLayer is not desired, such as
        an importer which caches the input layer, or to defer clearing until processScenegraph()
        is called.
        GeomSceneContext is provided in case request scope args are required.
        Base class implementation calls _editLayer->clear().
    */
    virtual void           initializeEditLayer(GeomSceneContext& context);

    /*! Called in getScenegraph() when defineGeometryState is dirty, to clear out _rootLayer
        in preparation for defining prims, attribs, or layering operations. A subclass can
        implement this to perform custom initialization if the default clearing of _rootLayer
        is not desired, or to defer clearing until processScenegraph() is called.
        GeomSceneContext is provided in case request scope args are required.
        Base class implementation calls _rootLayer->clear().
    */
    virtual void           initializeRootLayer(GeomSceneContext& context);

    /*! Return the local output stage for the engine. Base class returns the local \a _stage var.
        If \a force_reopen is true and the engine has an active edit layer then the Stage will
        be (re)opened by calling Stage::Create() with this engine's current root layer.
    */
    virtual usg::StageRef  stage(bool force_reopen = false);

    /*! Return the composed output stage for the engine.
        If this engine has an active edit layer then the local stage() will be
        returned.
        If this engine does not affect stage composition it should request the
        composedStage() from an input, if it has one.

        Default implementation tests for the edit layer and if none calls
        composedStage() on inputEngine0().
    */
    virtual usg::StageRef  composedStage();

    /*! Return the companion arbitrary output data for this engine.
        Base class returns the local \a _sceneOutputData var.
    */
    virtual usg::GeomSceneOutput*  sceneOutputData();


    /*! Calculate the input and output GeomStates and update the dirty status
        of the GeomEngine.
        A subclass can manipulate the state vars in a _validateGeomState()
        implementation.
    */
    void          validateGeomState();


    /*! Current processing version for this engine. This number is incremented
        each time this engine runs processScenegraph and reset to zero when
        \a _modificationVersion changes and the define-geometry state is dirty.

        A change in this number is a good indicator that a modification may
        have been made to a Layer or Stage attribute value contents, even if the
        overall dirty state of the engine has not changed.
    */
    int32_t       processedVersion() const { return _processedVersion; }

    /*! Returns the processing state of the engine which can be used to track if
        processing changes may have been applied to the engine's layers or stage.
        This hash is a combination of the outputState and the current modification
        and processed version numbers.
    */
    fdk::Hash     getProcessedState() const;

    //! Concatenated set of times passed to getScenegraph() - used for cache testing
    const TimeGeomStateMap&  processedTimes() const { return _processedTimes; }

    /*! Add a single processed time to the \a _processedTimes set. If \a state is
        provided then its value is stored otherwise the engine's \a _outputState is.
    */
    void          addProcessedTime(const fdk::TimeValue& time,
                                   const GeomState*      state = nullptr);

    //! Has the engine processed this time already?
    bool          hasProcessedTime(const fdk::TimeValue& time) const;

    /*! Retrieve the GeomState of the processed time, returning true if there was
        an existing GeomState entry for the time.
    */
    bool          getProcessedTime(const fdk::TimeValue& time,
                                   GeomState&            state) const;

    /*! Returns the set of TimeValues to process for this GeomEngine, which is the
        TimeValues from the \a times set minus the ones in the \a _processedTimes
        that match the engine's current \a _outputState.

        In other words this method removes the already processed times from the
        passed-in set.
    */
    fdk::TimeValueSet  getActiveProcessingTimes(const fdk::TimeValueSet& times) const;
    //! Version that takes a time/GeomState map.
    TimeGeomStateMap   getActiveProcessingTimes(const TimeGeomStateMap& times) const;

    /*! Same as getActiveProcessingTimes() but returns true if all times in \a times
        are already in _processedTimes.
    */
    bool          hasProcessedTimes(const fdk::TimeValueSet& times) const;
    //! Version that takes a time/GeomState map.
    bool          hasProcessedTimes(const TimeGeomStateMap& times) const;

    //! Add times to the processed set.
    void          addProcessedTimes(const fdk::TimeValueSet& times);
    //! Add times and GeomStates to the processed set.
    void          addProcessedTimes(const TimeGeomStateMap& times);

    //! Clear all the processed times.
    void          clearProcessedTimes();


    /*! Process the scene, filling in the Layers this GeomEngine owns and
        either producing a unique Stage or applying edits to a Stage owned
        by a GeomEngine below this one.

        This method examines the dirty states of the GeomEngine to determine
        whether to call the virtual processScenegraph() method so the engine
        subclass can perform any work defined by the dirty state.

        When this method completes the engine's dirty state is cleared and
        the 'last' GeomStates updated to the current ones.
    */
    void          getScenegraph(GeomSceneContext& context);

    /*! Directly runs processScenegraph() for all TimeValues in \a times, which
        is used instead of the times in \a src_context. A new GeomSceneContext
        is constructed with the new TimeValueSet and passed to processScenegraph().
        After processing the times are added to the current processed set.

        This method does not test any states and is equivalent to calling the
        following code:
          GeomSceneContext new_context(src_context, times);
          processScenegraph(new_context);
          addProcessedTimes(times);
    */
    void          processScenegraphForTimes(const GeomSceneContext&  src_context,
                                            const fdk::TimeValueSet& times);

    /*! Merges the layers from \a inputs into the \a _rootLayer.
        These are merged in reverse order.
    */
    virtual void  mergeFromInputs(GeomSceneContext&           context,
                                  const std::vector<int32_t>& inputs);


    //----------------------------------------------------------------------


  protected:
    /*! Subclass portion of validateGeomState().

        GeomEngine subclasses can mess with the local GeomState here to take
        into account input parameters, internal calculations, etc.

        Default implementation does nothing.
    */
    virtual void         _validateGeomState() {}


    /*! GeomEngine subclass can implement to return a root layer for a specific
        engine input.
        Does not call getScenegraph()!
    */
    virtual usg::LayerRef rootLayerFromInput(uint32_t input =0);

    /*! If the input is a GeomEngine return the composed Stage from it.
        Does not call getScenegraph()!
    */
    virtual usg::StageRef stageFromInput(uint32_t input =0,
                                         bool     force_reopen =false);


    /*! Subclass-specific engine implementation called by getScenegraph().

        Default implementation gets the root layer from inputEngine0() by
        calling getScenegraph() on it, then copies the SubLayer paths from
        the root layer.
    */
    virtual void         processScenegraph(GeomSceneContext& context);


    /*! Subclass-specific engine implementation called by getScenegraph(),
        after processScenegraph(), and only if affectsComposedStage()==true.

        Default implementation does nothing.
    */
    virtual void         editComposedStage(GeomSceneContext& context) {}


    //----------------------------------------------------------------------


  protected:
    //----------------------------------------------------------------------
    // Local cache data
    usg::LayerRef         _rootLayer;       //!< Cached local root Layer for input layer stack
    usg::LayerRef         _editLayer;       //!< Cached local edit Layer
    usg::StageRef         _stage;           //!< Cached local output Stage
    usg::GeomSceneOutput* _sceneOutputData; //!< Cached local output arbitrary scene data

    //----------------------------------------------------------------------
    // Current/pending state
    int32_t          _modificationVersion;  //!< A subclass should increment this when the engine's modification state changes
    GeomState        _inputState;           //!< Concatenation of all input GeomEngine output states
    GeomState        _localState;           //!< Local changes to geometry
    GeomState        _outputState;          //!< Concatenated state includes input and local states
    usg::ArgSet      _requestArgs;          //!< Provides additional request context for _validateGeomState()
    int32_t          _processedVersion;     //!< Incremented each time this engine runs processScenegraph()
    TimeGeomStateMap _processedTimes;       //!< Which times have been processed since the last dirty define-prims pass

    //----------------------------------------------------------------------
    // Previous state to detect change requirements, updated after getScenegraph() has been called:
    GeomState   _lastInputState;            //!< Concatenation of all previous input GeomEngine output states
    GeomState   _lastLocalState;            //!< Previous local changes to geometry
    GeomState   _lastOutputState;           //!< Concatenated state includes previous input and local states
    int32_t     _lastModificationVersion;   //!< Previous modification version

    //----------------------------------------------------------------------
    // Change targets:
    GeomStateTargetPtrMap _changeTargetMap; //!< What the engine intends to affect

    // Pre-defined change targets bound to hashes in GeomState:
    GeomStateTarget  _defineGeometryTarget; //!< Pre-assigned target that affects GeomState::defineGeometryState
    GeomStateTarget  _modifyValuesTarget;   //!< Pre-assigned target that affects GeomState::modifyValuesState
    GeomStateTarget  _editLayerTarget;      //!< Pre-assigned target that affects GeomState::editLayerState
    GeomStateTarget  _editStageTarget;      //!< Pre-assigned target that affects GeomState::editStageState
    GeomStateTarget  _affectsStereoTarget;  //!< Pre-assigned target that affects GeomState::affectsStereoState

  };



  //--------------------------------------------------------------------------------
  // Simple utility class to print debug spaces
  struct USG_ENGINE_API printSpaces
  {
    void indent2()  { nSpaces += 2; }
    void indent3()  { nSpaces += 2; }
    void indent4()  { nSpaces += 4; }
    void outdent2() { nSpaces -= 2; }
    void outdent3() { nSpaces -= 2; }
    void outdent4() { nSpaces -= 4; }
    printSpaces& operator += (int n) { nSpaces += n; return *this; }
    printSpaces& operator -= (int n) { nSpaces -= n; return *this; }
    printSpaces() : nSpaces(0) {}
    int nSpaces;
  };

  USG_ENGINE_API std::ostream& operator << (std::ostream& o, const printSpaces& b);

} // namespace usg


#endif

// Copyright (c) 2022 The Foundry Visionmongers Ltd.  All Rights Reserved.
