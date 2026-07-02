// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_GEOMSTAGE_H
#define USG_GEOM_GEOMSTAGE_H

#include "usg/geom/Layer.h"
#include "usg/geom/PrimRange.h"

#include "fdk/base/Hash.h"
#include "fdk/math/Box3.h"

#include <memory>
#include <string>


namespace usg
{
  using StageRef = std::shared_ptr<Stage>;
  using StageConstRef = std::shared_ptr<const Stage>;


  //! class Stage
  //! The wrapper class for a reference to a stage.
  //! When passing stage references, use LayerRef instead of Layer as that
  //! provides a pointer-like interface with access via the -> operator.
  class USG_API Stage
  {
  public:
    class Impl; //!< Wrapper abstraction for implementation type
    class Handle; //!< Abstraction handle for casting to underlying implementation type

    virtual ~Stage();

    //! Creates a new Stage from the session layer.
    //! If \a compose_state is provided it's copied into the Stage::Impl and
    //! retrievable via Stage::composeState().
    static StageRef Create(const LayerRef&  sessionLayer,
                           const fdk::Hash* composeState = nullptr,
                           bool             loadAll = true);

    //! Creates a new Stage from the given root layer and session layer.
    //! If \a compose_state is provided it's copied into the Stage::Impl and
    //! retrievable via Stage::composeState().
    static StageRef Create(const LayerRef&  rootLayer,
                           const LayerRef&  sessionLayer,
                           const fdk::Hash* composeState = nullptr,
                           bool             loadAll = true);

    //! Creates a new stage from the given root layer limiting its population
    //! to the prims provided by \a maskedPaths. If \a composeState is provided
    //! it's copied into the Stage::Impl and retrievable via Stage::composeState().
    static StageRef CreateMasked(const LayerRef& sessionLayer,
                                 const PathArray& maskedPaths,
                                 const fdk::Hash* composeState = nullptr,
                                 bool loadAll = true);
    //! Creates an empty Stage.
    static StageRef CreateInMemory();

    //! Creates a new Stage from the given file path
    static StageRef Open(const std::string& filePath);

    //------------------------------------------------------

    //! True if Stage::Impl is valid (has a valid object binding.)
    bool                    isValid() const;
    operator                bool()    const { return this->isValid(); }

    //! Return a pointer to the underlying UsdStage object if the usd_version
    //! passed in matches the Usd version this library was built with,
    //! otherwise return null.
    //!
    //! The version must match the one returned by usg::usdAPIVersion().
    //!
    //! Caller should cast the returned pointer to a UsdStageRefPtr* and
    //! possibly double-check its validity since a Stage can be created
    //! without a valid binding. However, if Stage::isValid() returns
    //! true then the underlying binding will also be valid.
    Stage::Handle*        getUsdStageRefPtr(int usd_version);
    const Stage::Handle*  getUsdStageRefPtr(int usd_version) const;

    //------------------------------------------------------

    //! Sets the edit target layer of the stage
    void   setEditTarget(const LayerRef& edit_layer);


    //! Returns the root layer of the stage
    LayerRef  getRootLayer() const;

    //! Returns the session layer of the stage
    LayerRef  getSessionLayer();

    //! Returns the root prim of the stage
    Prim      getRootPrim() const;

    //! Returns the prim at \a path
    Prim      getPrimAtPath(const Path& path) const;

    //! Returns all prim paths in the stage.
    //! Caution - this can potentially be a very expensive operation on large stages!
    size_t getPrimPaths(PathArray& prim_paths) const;

    //! Given a list of paths, iterates through them and optionally their descendents,
    //! returning a list of all prims matching the type T.
    template<class T>
    PathArray findPrimsOfType(const PathArray& primPaths,
                              bool             matchDescendents = true) const;

    //! Given a list of paths, iterates through them and their descendents, returning
    //! a list of all prims of type T.
    template<class T> std::vector<T> findDescendentPrims(const PathArray& primPaths) const;

    //! Set the population mask for the stage. Use this to limit the prims that are composed.
    //! This is useful for isolating prims.
    void setPopulationMask(const PathArray& primPaths);

    //! Build a PrimRange traverser using the Prim::DefaultPredicate mask (ie Prim::IsActive &&
    //! Prim::IsDefined && Prim::IsLoaded && -Prim::IsAbstract).
    PrimRange traverse() const;

    //! Build a PrimRange traverser using a specific predicate mask.
    PrimRange traverse(const Prim::FlagsPredicate& mask) const;

    //! Build a PrimRange traverser using the Prim::AllPrimsPredicate (ie no filtering.)
    PrimRange traverseAll() const;

    //------------------------------------------------------

    //! Compute the world-space bounding-box for the entire stage at a specific time (required.)
    fdk::Box3d  getWorldBbox(const fdk::TimeValue& time) const;

    //! Set the metersPerUnit metadata tag on the stage.
    void    setMetersPerUnit(double metersPerUnit);

    //! Is the metersPerUnit metadata tag authored on the stage?
    bool    hasMetersPerUnit() const;

    /*! Return the metersPerUnit metatada tag if authored, otherwise 0.1, the USD default
        of centimeters.
        Note the default unit for Nuke is meters so always check hasMetersPerUnit() to
        avoid getting centimeters by mistake.
    */
    double  getMetersPerUnit() const;

    //------------------------------------------------------
    //! Returns the flattened stage as a single anonymous layer
    usg::LayerRef flatten(bool addComment = true);

    //! Returns the flattened stage as a single anonymous layer. Unlike 'flatten', this preserves variants and some composition arcs.
    usg::LayerRef flattenLayerStack();

    //! Exports the flattened stage to a file
    bool  exportToFile(const std::string&    filepath,
                       bool                  addComment = true,
                       const FileFormatArgs& fileFormatArgs = FileFormatArgs());

    //! Exports the flattened stage to a string
    bool  exportToString(std::string& text,
                         bool         addComment = true);

    //! Return the attribute representing the 'default parent' from the top-most (strongest) layer.
    Path  getDefaultParentPath() const;

    //------------------------------------------------------

    //! Start of active time sample range.
    void            setStartTimeCode(fdk::TimeValue time);
    fdk::TimeValue  getStartTimeCode();

    //! End of active time sample range.
    void            setEndTimeCode(fdk::TimeValue time);
    fdk::TimeValue  getEndTimeCode();

    //! Falls back to framesPerSecond, then to a default value of 24.0
    void    setTimeCodesPerSecond(double timeCodesPerSecond);
    double  getTimeCodesPerSecond() const;
    //! 
    void    setFramesPerSecond(double fps);
    double  getFramesPerSecond() const;


    void   printReport(std::ostream& o,
                       bool          print_contents = true) const;


    //------------------------------------------------------
    // For internal use only....
    //------------------------------------------------------

    //! Create a stage wrapper from a Stage::Impl.
    Stage(const Stage::Impl&);

    Stage::Impl*        impl()       { return _pImpl.get(); }
    const Stage::Impl*  impl() const { return _pImpl.get(); }

    //! Set the compose state hash explicitly (usually passed to the latest stage Create()).
    void       setComposeState(const fdk::Hash& compose_state);
    //! Get the compose state hash currently assigned to the stage.
    fdk::Hash  composeState() const;

    //! Set hashes to help track the lastest modify state affecting the stage.
    void       setProcessState(const fdk::Hash& process_state);
    fdk::Hash  processState() const;

    //! Set hashes to help track the latest edit state of the stage.
    void       setEditState(const fdk::Hash& edit_state);
    fdk::Hash  editState() const;

    //! Set a state hash to help track the root layer's sublayer list state. Normally this
    //! is used to quickly test sublayer list status.
    void       setRootState(const fdk::Hash& root_state);
    fdk::Hash  rootState() const;

    //! Update Stage and cache Layer custom metadata with the cache layer info to
    //! be used during geometry_engine() edit calls. For internal use only.
    void   setPreEditCacheMetadata(const std::string& cache_layer_id,
                                   uint64_t           cache_hash);
    //! Retrieve the current cache layer metadata values, returning true on success. For internal use only.
    bool   getPreEditCacheMetadata(std::string& cache_layer_id,
                                   uint64_t&    cache_layer_hash);

    //! Copy property values from the Layer to the Stage, normally used
    //! to restore cached 'pre-edited' values prior to possibly applying
    //! a new edit.
    //!
    //! This iterates through all properties defined in the Layer.
    //! For internal use only.
    void   retrievePreEditCacheProperties(const LayerRef&       cache_layer,
                                          const fdk::TimeValue& time = fdk::defaultTimeValue());

  private:
    Stage() = delete;
    Stage(const Stage&) = delete;
    Stage& operator=(const Stage&) = delete;

    std::unique_ptr<Stage::Impl> _pImpl;
    LayerRef _sessionLayer;
  };


  //-----------------------------------------------------------------------------------------


  //! class StageNotifications
  //! A class for receiving notifications when changes are made to a stage.
  //! The default implementatin just keeps a changed flag.
  class USG_API StageNotifications {
  public:
    class ChangedObjects {
    public:
      virtual ~ChangedObjects();
      virtual PathArray getChangedPaths() const = 0;
      virtual PathArray getChangedInfoPaths() const = 0;
      virtual TokenArray getChangedFields(const Path& path) const = 0;
    };

    StageNotifications(const StageRef& stage);
    virtual ~StageNotifications();

    virtual void handleObjectsChanged(const ChangedObjects& changedObjects);
    virtual void handleContentsChanged() { _changed = true; }
    virtual void handleLayerMutingChanged() { _changed = true; }
    virtual void handleEditTargetChanged() { _changed = true; }

    void clearChanged() { _changed = false; }
    bool isChanged() const { return _changed; }


  private:
    std::unique_ptr<class StageNotificationsImpl> _pImpl;
    bool _changed = false;
  };

} // namespace usg

#endif
