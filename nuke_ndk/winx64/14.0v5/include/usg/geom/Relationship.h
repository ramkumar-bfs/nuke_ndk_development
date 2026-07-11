// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_RELATIONSHIP_H
#define USG_GEOM_RELATIONSHIP_H

#include "usg/geom/Attribute.h"

#include <memory>

namespace usg
{

  /*! Relationships are always attached to Prims so use the Prim interface to create
      or get access to them.

      To create a new Relationship use Prim::createIntrinsicRelationship() or
      Prim::createCustomRelationship().
      To access an existing Relationship use Prim::getRelationship().

      TODO: We're only supporting a single relationship target currently!
            Need multiple targets for PointInstancer, maybe other classes
  */
  class USG_API Relationship
  {
  public:
    class Impl; //!< Wrapper abstraction for implementation type
    class RelationshipSpecHandle; //!< Abstraction handle for casting to SdfRelationshipSpecHandle
    class UsdRelationshipHandle; //!< Abstraction handle for casting to UsdRelationship


    //! Default ctor builds an empty (invalid) relationship.
    Relationship();
    //! Copy constructors
    Relationship(const Relationship&);
    Relationship& operator = (const Relationship&);

    // Dtor in implementation required for std::unique_ptr to work
    ~Relationship();

    //------------------------------------------------------

    //! True if RelationshipHandle is valid (has a valid object binding.)
    bool              isValid() const;
    operator          bool()    const { return this->isValid(); }

    //! If the Relationship is bound to an SdfRelationshipSpecHandle return a pointer to
    //! the underlying SdfRelationshipSpecHandle object if the usd_version passed in
    //! matches the Usd version this library was built with, otherwise return null.
    //!
    //! The version must match the one returned by usg::usdAPIVersion().
    //!
    //! Caller should cast the returned pointer to an SdfRelationshipSpecHandle* and
    //! possibly double-check its validity since a Relationship can be created
    //! without a valid binding. However, if Relationship::isValid() returns
    //! true then the underlying binding will also be valid.
    class Relationship::RelationshipSpecHandle*        getSdfRelationshipSpec(int usd_version);
    const class Relationship::RelationshipSpecHandle*  getSdfRelationshipSpec(int usd_version) const;

    //! If the Relationship is bound to a UsdRelationship return a pointer to the
    //! underlying UsdRelationship object if the usd_version passed in matches
    //! the Usd version this library was built with, otherwise return null.
    //!
    //! The version must match the one returned by usg::usdAPIVersion().
    //!
    //! Caller should cast the returned pointer to an UsdRelationship* and
    //! possibly double-check its validity since a Relationship can be created
    //! without a valid binding. However, if Relationship::isValid() returns
    //! true then the underlying binding will also be valid.
    class Relationship::UsdRelationshipHandle*        getUsdRelationship(int usd_version);
    const class Relationship::UsdRelationshipHandle*  getUsdRelationship(int usd_version) const;

    //------------------------------------------------------

    //!
    const std::string&  getName() const;
    //! Full path including the Prim parent.
    Path                getPath() const;


  public:
    //------------------------------------------------------
    // Target path management:
    //------------------------------------------------------

    //! Sets the relationship's target path to a single path.
    void      setTarget(const Path& targetPath);

    //! Retrieve the first target path.
    Path      getTarget() const;

    //! Sets the relationship's target paths.
    void      setTargets(const PathArray& targetPaths);

    //! Retrieves the relationship's target paths.
    PathArray getTargets() const;


  public:
    //------------------------------------------------------
    // For internal use only....
    //------------------------------------------------------

    Relationship(const Relationship::Impl&);
    Relationship& operator = (const Relationship::Impl&);

    //! Returns true if resulting Relationship is valid.
    bool              setFromImpl(const Relationship::Impl& b);

    //!
    Relationship::Impl*       impl()       { return _pImpl.get(); }
    const Relationship::Impl* impl() const { return _pImpl.get(); }


  protected:
    std::unique_ptr<Relationship::Impl> _pImpl;
  };


  //--------------------------------------------------------------------------------------------------


  /*----------------------------------*/
  /*        Static operations         */
  /*----------------------------------*/

  USG_API  std::ostream& operator << (std::ostream& o, const Relationship&);



} // namespace usg

#endif
