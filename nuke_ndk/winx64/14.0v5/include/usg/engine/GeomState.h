// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_ENGINE_GEOMSTATE_H
#define USG_ENGINE_GEOMSTATE_H

#include "usg/engine/api.h"
#include "fdk/base/Hash.h"

namespace usg
{

  struct USG_ENGINE_API GeomState
  {
    fdk::Hash defineGeometryState;    //!< Do Layer prim/attribute defines - causes Stage recompose!
    fdk::Hash modifyValuesState;      //!< Do Layer attribute contents modifications (no Stage recompose)
    fdk::Hash editLayerState;         //!< Edit Layer's current state (may cause Stage recompose, may not)
    fdk::Hash editStageState;         //!< Edit Stage's current state (no Stage recompose)

    //! Default ctor leaves all hashes at their default value (~0xLL)
    GeomState() {}
    //! Set all hashes to a specific value.
    GeomState(const fdk::Hash& init);

    //! Return a single hash consisting of all non-default hashes appended together.
    fdk::Hash  hash() const;
    operator fdk::Hash() const { return hash(); }

    bool operator == (const GeomState& b) const;
    bool operator != (const GeomState& b) const { return !(*this == b); }

    bool  isDefineGeometryDefault() const { return defineGeometryState.isDefault(); }
    bool  isModifyValuesDefault()   const { return modifyValuesState.isDefault(); }
    bool  isEditLayerDefault()      const { return editLayerState.isDefault(); }
    bool  isEditStageDefault()      const { return editStageState.isDefault(); }

    //! Are all hashes at the fdk::Hash default value? (this won't work if hashes set to a different initial value)
    bool  isDefault() const;

    //! Set all hashes to a specific value.
    void  set(const fdk::Hash& h);

    //! Copy all the hashes from \a b GeomState.
    void  set(const GeomState& b);

    //! Reset all the hashes to the fdk::Hash default value (~0xLL)
    void  reset();

    //! Append each of \a b GeomState's hashes.
    void  append(const GeomState& b);

    //! Append each of \a b GeomState's hashes if they're not at the default value (~0xLL)
    void  appendIfNotDefault(const GeomState& b);


    friend USG_ENGINE_API std::ostream& operator << (std::ostream&, const GeomState&);
  };


  USG_ENGINE_API std::ostream& operator << (std::ostream&, const GeomState&);


inline
GeomState::GeomState(const fdk::Hash& init) :
  defineGeometryState(init),
  modifyValuesState(init),
  editLayerState(init),
  editStageState(init)
{
  //
}

inline bool
GeomState::operator == (const GeomState& b) const
{
  return (defineGeometryState  == b.defineGeometryState &&
          modifyValuesState == b.modifyValuesState &&
          editLayerState == b.editLayerState &&
          editStageState == b.editStageState);
}

inline bool
GeomState::isDefault() const
{
  return (defineGeometryState.isDefault()  &&
          modifyValuesState.isDefault() &&
          editLayerState.isDefault() &&
          editStageState.isDefault());
}


inline void
GeomState::set(const fdk::Hash& h)
{
  defineGeometryState = h;
  modifyValuesState = h;
  editLayerState = h;
  editStageState = h;
}

inline void
GeomState::set(const GeomState& b)
{
  defineGeometryState = b.defineGeometryState;
  modifyValuesState = b.modifyValuesState;
  editLayerState = b.editLayerState;
  editStageState = b.editStageState;
}

inline void
GeomState::reset()
{
  defineGeometryState.reset();
  modifyValuesState.reset();
  editLayerState.reset();
  editStageState.reset();
}

inline void
GeomState::append(const GeomState& b)
{
  defineGeometryState.append(b.defineGeometryState);
  modifyValuesState.append(b.modifyValuesState);
  editLayerState.append(b.editLayerState);
  editStageState.append(b.editStageState);
}

inline void
GeomState::appendIfNotDefault(const GeomState& b)
{
  defineGeometryState.appendIfNotDefault(b.defineGeometryState);
  modifyValuesState.appendIfNotDefault(b.modifyValuesState);
  editLayerState.appendIfNotDefault(b.editLayerState);
  editStageState.appendIfNotDefault(b.editStageState);
}

inline fdk::Hash
GeomState::hash() const
{
  fdk::Hash hash;
  hash.appendIfNotDefault(defineGeometryState);
  hash.appendIfNotDefault(modifyValuesState);
  hash.appendIfNotDefault(editLayerState);
  hash.appendIfNotDefault(editStageState);
  return hash;
}


}

#endif
