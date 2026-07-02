// Copyright (c) 2025 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_API_STAGESUBTREECONTROLLER_H
#define USG_API_STAGESUBTREECONTROLLER_H

#include "usg/api.h"
#include "usg/geom/Layer.h"
#include "usg/geom/Path.h"
#include "usg/geom/Prim.h"
#include "usg/geom/Stage.h"

namespace usg {

  // Returns a unique path by appending a number suffix to the given path
  // if it already exists according to the provided 'exists' function.
  usg::Path getUniquePathBy(const usg::Path& path, std::function<bool(const usg::Path&)> exists);


  // A Class which contains a usg::Layer, and a basePath. It can be updated with
  // a rootlayer to create a stage with the provided rootLayer, and then the
  // content layer as the session layer of that stage. The base path can then
  // be altered and will move the prims within the contentLayer around to match
  // that base path.  The methods on this class will accept paths in two forms
  // either the full absolute path containing the basePath, this is useful when
  // simply using USD API to getPath from an existing prim, without having to
  // cut out the basePath.  Secondarily, if the path does not start with the
  // basePath, we will treat it as a relative path, and prepend the base path
  // to the method in question.  This should allow for easier handling of
  // sub prims within the provided base path, especially for UI's or interfaces
  // that hide away the ancestor or sibling prims of the base path itself, and
  // only show/work with the sub prim hierarchy.
  // Prims should not be created outside of the basePath on the contentLayer.
  // They can exist on the Stage, but not the contentLayer, since that would
  // break the ability to move the base path.
  class USG_API StageSubtreeController
  {
  private:
    Path m_basePath;
    LayerRef m_contentLayer;
    StageRef m_stage;
    Prim::PrimSpecifier m_basePathPrimSpecifier;

    // Ensure basePath is absolute and valid
    void validateBasePath();

    // Create prim with given path and type. Default primSpecifier is Define.
    // By default, makes `path` unique by appending a number to it.
    // For e.g. if `/root/prim1` is already present in the stage and this method is called with `/root/prim1`,
    // it will create `/root/prim2` and not overwrite the prim at `/root/prim1`
    usg::Prim _createPrim(const usg::Path& absolutePath, const usg::Token& primType,
                          Prim::PrimSpecifier specifier, bool enumerate = true);

    // Ensures that prims exist at all the parents of the provided path. Creates Over prims
    // if they do not.
    void ensurePrimExistsForPath(const usg::Path& absolutePath);

  public:
    // Constructors
    StageSubtreeController(Path basePath, LayerRef contentLayer,
                           const Prim::PrimSpecifier basePathPrimSpecifier = Prim::PrimSpecifier::Def);
    // Constructs a new anonymous layer with the provided layer name.
    StageSubtreeController(Path basePath, const std::string& layerName,
                           const Prim::PrimSpecifier basePathPrimSpecifier = Prim::PrimSpecifier::Def);

    // Clears the pointer to the stage to free up any layers the stage
    // may be referencing, without clearing the contentLayer itself.
    void flush();

    // Accessors
    LayerRef getContentLayer() const;
    StageRef getStage() const;

    // Updates the stage held by this class, and adds the contentLayer as the
    // sessionLayer such that it's changes override the stage.
    bool updateIncomingStage(const usg::LayerRef incomingRootLayer);
    
    // Get absolute prim path in content layer from given path.
    // e.g., if basePath is /root and path is child or /root/child, it returns /root/child.
    Path getAbsoluteContentLayerPath(const Path& path) const;

    // Get a Prim from the stage
    // if stage is not present, return invalid prim
    usg::Prim getStagePrim(const usg::Path& path) const;
    // Get Prims from the stage
    // if stage is not present, return an empty list
    std::vector<usg::Prim> getStagePrims(const usg::PathArray& paths) const;

    // Get a Prim from the contentLayer
    usg::Prim getContentLayerPrim(const usg::Path& primPath) const;
    // Get Prims from the contentLayer
    std::vector<usg::Prim> getContentLayerPrims(const PathArray& primPaths) const;

    // Change the member variable basePath to the specified destination and move the corresponding
    // prim in the contentLayer only if it succeeds. Returns success or failure.
    bool changeBasePath(const Path& destination);

    // Updates the prim specifier for the basePath and it's parents.
    void updateBasePathPrimSpecifier(const Prim::PrimSpecifier specifier);

    // Import all prims in the specified layer to contentLayer under basePath. If paths is
    // specified, import only the specified prims.
    bool importFromLayer(const LayerRef& layer, const PathArray& paths = {},
                         const Path& dest = { "." });

    // Export all prims in contentLayer to the specified layer. If paths is specified, export only
    // the specified prims. If flattenHierarchy is true, export prims to the root of the layer. If
    // fromStage is true, export prims on the stage rather than in the contentLayer.
    bool exportToLayer(LayerRef& layer, const PathArray& paths = {}, bool flattenHierarchy = true,
                       bool fromStage = false) const;

    // Create prim with given path and type. Default primSpecifier is Define.
    // By default, makes `path` unique by appending a number to it.
    // For e.g. if `/root/prim1` is already present in the stage and this method is called with `/root/prim1`,
    // it will create `/root/prim2` and not overwrite the prim at `/root/prim1`
    usg::Prim createPrim(const usg::Path& path, const usg::Token& primType = usg::Token(),
                         Prim::PrimSpecifier specifier = Prim::PrimSpecifier::Def,
                         bool enumerate = true);

    // Remove prim at the given path. If unsuccessful return false.
    bool removePrim(const usg::Path& path);

    // Returns whether the prim was created only on the contentLayer, and not from the incoming
    // stage. We check that the only contributing layer is the contentLayer
    bool isPrimLocalOnly(const usg::Path& path) const;

    // Get the path used to refer to the root location of the content layer.
    const Path& getBasePath() const;

    /// Sets the base prim path.
    ///
    /// \warning The function does not perform any validation; it is expected that the provided path
    /// is valid and absolute.
    ///
    /// Unlike \c changeBasePath(), which reparents any prim from the old base to the new base, this
    /// method only sets the internal variable without any further operation. It is used in cases
    /// where the content layer has been operated externally.
    ///
    /// \param basePath The new base prim path to set, which should be a valid, absolute prim path.
    void setBasePath(const Path& basePath);

    // Get a unique path for the given path, by appending a number to it or by incrementing an
    // already existing numerical suffix if necessary.
    usg::Path getUniquePath(const usg::Path& path) const;
  };

} // namespace usg
#endif
