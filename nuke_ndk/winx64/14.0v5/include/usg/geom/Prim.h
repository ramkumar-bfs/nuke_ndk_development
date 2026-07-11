// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_PRIM_H
#define USG_GEOM_PRIM_H

#include "usg/base/Token.h"
#include "usg/geom/Attribute.h"
#include "usg/geom/Relationship.h"
#include "usg/geom/GeomTokens.h"

#include <memory>

namespace usg
{
  class Layer;
  class Stage;
  class ShaderDesc;

  using LayerRef = std::shared_ptr<Layer>;
  using LayerConstRef = std::shared_ptr<const Layer>;
  using StageRef = std::shared_ptr<Stage>;
  using StageConstRef = std::shared_ptr<const Stage>;

  enum class ListPosition {
    PrependFront,
    PrependBack,
    AppendFront,
    AppendBack
  };

  //! Wrapper class for a prim.
  class USG_API Prim
  {
  public:
    class Impl; //!< Wrapper abstraction for implementation type
    class SpecPrimHandle; //!< Abstraction handle for casting to SdfPrimSpec
    class UsdPrimHandle; //!< Abstraction handle for casting to UsdPrim


    //! Constructs an empty (invalid) Prim.
    Prim();
    Prim(const Prim&);
    Prim(const Prim::Impl&);
    Prim& operator = (const Prim&);

    virtual ~Prim();

    //! Equality - are the two Prims the same object or not? Compares the underlying Prim::Impls.
    bool operator == (const Prim& b) const;
    bool operator != (const Prim& b) const { return !(*this == b); }

    bool operator < (const Prim& b) const;

    //------------------------------------------------------

    //! Returns true if Prim::Impl is valid (has a valid object binding.)
    bool                isValid() const;
    explicit operator   bool()    const { return this->isValid(); }

    //! Returns true if we're holding a valid type of prim for our class. If not, returns false and invalidates
    //! this prim so that furutre.calls to isValid will return false;
    virtual bool validate();

    //! If the Prim is bound to an SdfPrimSpec return a pointer to the
    //! underlying SdfPrimSpec object if the usd_version passed in matches
    //! the Usd version this library was built with, otherwise return null.
    //!
    //! The version must match the one returned by usg::usdAPIVersion().
    //!
    //! Caller should cast the returned pointer to an SdfPrimSpec* and
    //! possibly double-check its validity since a Prim can be created
    //! without a valid binding. However, if Prim::isValid() returns
    //! true then the underlying binding will also be valid.
    class Prim::SpecPrimHandle*        getSdfPrimSpec(int usd_version);
    const class Prim::SpecPrimHandle*  getSdfPrimSpec(int usd_version) const;

    //! If the Prim is bound to a UsdPrim return a pointer to the
    //! underlying UsdPrim object if the usd_version passed in matches
    //! the Usd version this library was built with, otherwise return null.
    //!
    //! The version must match the one returned by usg::usdAPIVersion().
    //!
    //! Caller should cast the returned pointer to an UsdPrim* and
    //! possibly double-check its validity since a Prim can be created
    //! without a valid binding. However, if Prim::isValid() returns
    //! true then the underlying binding will also be valid.
    class Prim::UsdPrimHandle*        getUsdPrim(int usd_version);
    const class Prim::UsdPrimHandle*  getUsdPrim(int usd_version) const;

    //------------------------------------------------------

    //! Returns the name of the prim
    const std::string&  getName() const;

    //! Returns the path of the prim
    Path                getPath() const;

    //! Returns the name of the type of the prim
    const Token         getTypeName() const;

    //! Return this prim's parent prim. Returns an invalid prim if this is a root prim.
    Prim                getParent() const;

    //! Returns true if the prim is the pseudo root - ie '/'.
    bool                isPseudoRoot() const;

    //! Returns true if the prim is an instance of the given type
    bool isA(const Token& typeName) const;

    //! Returns true if the prim is an instance of a schema
    template<class T> bool isA() const { return T::isValidPrim(*this); }

    //! Returns true if the prim has the given API
    bool hasApi(const Token& typeName) const;

    //! Returns true if the prim has the given multiple-apply schema API
    bool hasApi(const Token& typeName, const Token& instanceName) const;

    //! Returns true if the prim has the given API
    template<class T> bool hasApi() const { return T::isValidPrim(*this); }

    //! Returns true if the prim has the given multiple-apply schema API
    template<class T> bool hasApi(const Token& instanceName) const { return T::isValidPrim(*this, instanceName); }

    //------------------------------------------------------

    void   setActive(bool active);
    bool   isActive() const;

    bool isDefined() const;

    bool hasPayload() const;
    bool isLoaded() const;
    void load() const;
    void unload() const;

    void   setVisible(bool active, fdk::TimeValue time = fdk::defaultTimeValue());
    bool   isVisible(fdk::TimeValue time = fdk::defaultTimeValue()) const;

    bool   addAppliedSchema(const Token& schemaName);

    //------------------------------------------------------

    //! Sets the name of the app that created this prim.
    void   setCreator(const Token& creator);

    //! Gets the name of the app that created this prim.
    Token  creatorApp() const;

    //! True if creator is the prim's creator app.
    bool   isCreator(const Token& creator) const;

    //------------------------------------------------------

    //! Sets the custom data value of given key.
    void   setCustomData(const std::string& key, const Value& value);

    //! Retrieves the custom data value for the key. Returns true if it exists.
    bool   getCustomData(const std::string& key, Value& value) const;

    //! Returns a valid value if the custom data exists for the key.
    Value  getCustomData(const std::string& key) const;

    //! Checks whether the prim has custom data for the key.
    bool   hasCustomData(const std::string& key) const;

    //! Erases the custom data for the key.
    void   removeCustomData(const std::string& key);


  public:
    //------------------------------------------------------
    // Create:
    //------------------------------------------------------

    //! Creates a Prim definition in the owner Layer, returning a constructed reference.
    //! Convenience function that calls Layer::definePrim().
    static Prim  defineInLayer(const LayerRef& layer,
                               const Path&     path,
                               const Token&    typeName);

    //! Creates a Prim override in the owner Layer, returning a constructed reference.
    //! Convenience function that calls Layer::overridePrim().
    static Prim  overrideInLayer(const LayerRef& layer,
                                 const Path&     path,
                                 const Token&    typeName);

    //! Creates the missing parent prims in a Prim path with a specific type.
    //!
    //! Other define and override prim methods default to creating missing parent
    //! prims as untyped pure overs, which may cause the prim to act as if they
    //! doesn't exist.
    //!
    //! Be careful with this method as it requires a composed source stage to
    //! determine whether a prim is 'missing' from the path or not, and this may
    //! lead to unexpected overhead costs when defining prims.
    static void  createMissingParents(const StageRef& referenceStage,
                                      const LayerRef& layer,
                                      const Path&     path,
                                      const Token&    parentPrimType = GeomTokens.types.Scope);



  public:
    void  setKind(const Token& kind);
    Token getKind() const;

    void setInstanceable(bool instanceable);
    bool isInstanceable() const;


    //------------------------------------------------------
    // Attribute management:
    //------------------------------------------------------

    //! Creates and attaches an intrinsic(non-custom) attribute to this Prim.
    //! If the named Attribute object already exists returns it at its current
    //! value.
    //!
    //! If this Prim has just been defined on a Layer then the new
    //! attribute will be defined on that Layer.
    //!
    //! If this Prim is being edited then the attribute will be created
    //! on the editable Stage and saved to the Stage's current
    //! edit target Layer.
    //!
    //! \a variability_hint is only declaring the *expected* time-variance
    //! of the new attribute. In practice this does not do much as setting
    //! time samples later on is not prohibited if the attribute is declared
    //! here as uniform.
    Attribute  createIntrinsicAttr(const Token&    name,
                                   Value::Type     type,
                                   TimeVariability variability_hint = TimeVariability::Uniform);

    //! Creates and attaches a custom(arbitrary) attribute to this Prim.
    //! If the named Attribute object already exists returns it at its current
    //! value.
    //!
    //! If this Prim has just been defined on a Layer then the new
    //! attribute will be defined on that Layer.
    //!
    //! If this Prim is being edited then the attribute will be created
    //! on the editable Stage and saved to the Stage's current
    //! edit target Layer.
    //!
    //! \a variability_hint is only declaring the *expected* time-variance
    //! of the new attribute. In practice this does not do much as setting
    //! time samples later on is not prohibited if the attribute is declared
    //! here as uniform.
    Attribute  createCustomAttr(const Token&    name,
                                Value::Type     type,
                                TimeVariability variability_hint = TimeVariability::Uniform);

    //! Creates an attribute with the given name, type, temporal variability, and
    //! optionally setting the default value.
    //!
    //! If the named Attribute object already exists the default value will still
    //! be set if provided.
    //!
    //! \a variability_hint is only declaring the *expected* time-variance
    //! of the new attribute. In practice this does not do much as setting
    //! time samples later on is not prohibited if the attribute is declared
    //! here as uniform.
    Attribute  createAttr(const Token&    name,
                          Value::Type     type,
                          bool            is_custom = false,
                          TimeVariability variability_hint = TimeVariability::Varying,
                          const Value&    defaultValue = Value());

    //! Sets an existing attribute's value at a specific time. Returns a valid Attribute
    //! if successful otherwise an invalid one on failure.
    //!
    //! This templated method does not support custom user data types, only the common
    //! ones found in the Value::Type 'Base data types' list.
    //!
    //! Assumes the attrib has already been created and configured for the correct
    //! Value::Type and interpolation rate. Use createAttr() for this prior to calling
    //! setAttr().
    //!
    //! If the attribute does not already exist setAttr() will create it as a custom
    //! uniform attribute using the default Value::Type corresponding to the data type
    //! passed in. For example if setting UVs you would likely want the attribute to be
    //! a TexCoord2fArray Value::Type, but this method will create it as a Float2Array.
    //!
    //! To set array-variant types (FloatArray, Color3fArray, etc) pass in a std::vector.
    //! Many of the common array types are already defined in geo/scene/Array.h or in the
    //! vec/mat/box math types.
    //! For example to set an array of normals:
    //!   Vec3fArray normalsArray;
    //!   prim.setAttr(GeomTokens.primvars.normals, normalsArray, 100.0);
    template <typename T>
    Attribute  setAttr(const Token&         name,
                      const T&              v,
                      const fdk::TimeValue& time = fdk::defaultTimeValue());

    //! If the named attribute exists on the Prim returns it otherwise return an empty (invalid) Attribute object.
    Attribute  getAttr(const Token& name) const;

    //! If the named attribute exists on the Prim fills in the provided
    //! Attribute object and returns true, otherwise returns false.
    bool      getAttr(const Token& name,
                      Attribute&   attr) const;

    //! Returns the list of attributes.
    std::vector<Token> getAttributes() const;

    //! Copy the attribute from another prim at the given time.
    Attribute  copyAttr(const Prim&  fromPrim,
                       const Token& name,
                       fdk::TimeValue    time);

    //! Retrieves any overrides from the ShaderDesc and writes them to the
    //! matching Prim Attributes, optionally skipping any that have
    //! connections defined.
    //!
    //! SchemaProperties tagged as 'usg_local' (a LocalProp) will not have the
    //! 'inputs:' prefix added to the property name so Schema overrides can be
    //! applied to generic attributes.
    //!
    //! Returns false if any underlying error occurred.
    virtual bool  applyAttributeOverrides(const ShaderDesc& schema_desc,
                                          bool              skip_connected =true);


  public:
    //------------------------------------------------------
    // Inter-prim linkages - ie relationships:
    //------------------------------------------------------

    //! Creates and returns an empty relationship
    Relationship  createRelationship(const Token& name, bool custom = false);

    //! If the named relationship exists on the Prim returns it otherwise
    //! returns an empty (invalid) Relationship object.
    Relationship  getRelationship(const Token& name) const;

    //! If the named relationship exists on the Prim fills in the provided
    //! Relationship object and return true, otherwise return false.
    bool          getRelationship(const Token&  name,
                                  Relationship& rel) const;

    //! Fills in the list of relationship names, returning the number of relationship.
    size_t        getRelationships(std::vector<const char*>& relationships) const;

    //! Adds a reference to the prim's reference list.
    void          addReference(const std::string& identifier,
                               const Path&        path,
                               ListPosition       position = ListPosition::PrependBack);

    //! Adds a payload to the prim's reference list.
    void          addPayload(const std::string& identifier,
                             const Path&        path,
                             ListPosition       position = ListPosition::PrependBack);

    //! Adds a variant set to the prim.
    void          addVariantSet(const std::string& variantSetName,
                                ListPosition       position = ListPosition::PrependBack);

    //! Adds a variant to a variant set
    void          addVariant(const std::string& variantSetName, const std::string& variantName);

    //! Sets the variant selection for a variant set
    void          setVariantSelection(const std::string& variantSetName, const std::string& selection);

    //! Returns the list of variant sets authored on the prim
    std::vector<std::string> getVariantSets() const;

    //! Returns the list of variants for a variant set on this prim
    std::vector<std::string> getVariants(const std::string& variantSetName) const;

  public:
    //------------------------------------------------------
    // Child Prims:
    //------------------------------------------------------

    //! Fills in a list of child Prims directly underneath this one (ie does
    //! not include any children of the children) and returns the list size.
    //!
    //! This only functions if Prim is resident on a valid stage.
    size_t        getChildren(std::vector<Prim>& children, bool activeOnly = true) const;

    //! Returns a possible layer which defined this prim. This returns the layer containing the first "def" for the prim.
    LayerRef  getDefiningLayer() const;


  public:
    //! Debugging - prints some key info about the prim to a stream.
    virtual void  print(const char*   prefix,
                        std::ostream& o) const;


  public:
    //------------------------------------------------------
    // For internal use only....
    //------------------------------------------------------

    //!
    Prim::Impl*         impl()       { return _pImpl.get(); }
    const Prim::Impl*   impl() const { return _pImpl.get(); }


  protected:
    std::unique_ptr<Prim::Impl> _pImpl;
  };



  /*----------------------------------*/
  /*        Static operations         */
  /*----------------------------------*/


  //! Print out terse info about the prim to a stream.
  USG_API  std::ostream& operator << (std::ostream&, const Prim&);

} // namespace usg

#endif
