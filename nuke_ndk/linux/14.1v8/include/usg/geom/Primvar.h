// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_PRIMVAR_H
#define USG_GEOM_PRIMVAR_H

#include "usg/api.h"

#include "usg/base/ArrayTypes.h"

#include "usg/geom/Attribute.h"
#include "usg/geom/Prim.h"

namespace usg {

  //! A class representing a primvar attribute. This has extra methods for dealing with indices, flattening values and so on.
  class USG_API Primvar {
  public:
    Primvar() = delete;
    Primvar(const Prim& prim, const Token& name);

    //! Returns true if the primvar is valid
    bool isValid() const;
    explicit operator bool() const { return isValid(); }

    //! Returns the primvar attribute
    Attribute& attribute() { return _attr; }

    //! Returns the primvar indices attribute if it exists
    Attribute& indicesAttribute() { return _indicesAttr; }

    //! Returns the interpolation of the attribute
    Token getInterpolation() const { return _attr.getInterpolation(); }

    //! Computes the flattened primver attributes from the attribute and its indices. If there are no indices, just returns the straight values.
    template<class T> bool computeFlattened(Array<T>& flattenedValues, fdk::TimeValue time = fdk::defaultTimeValue()) const;

  protected:
    Prim _prim;
    Attribute _attr;
    Attribute _indicesAttr;
  };


  template<class T> inline bool Primvar::computeFlattened(Array<T>& flattenedValues, fdk::TimeValue time) const
  {
    if (_attr) {
      flattenedValues.clear();
      if (_indicesAttr) {
        IntArray indices;
        Array<T> values;
        _indicesAttr.getValue(indices, time);
        _attr.getValue(values, time);
        flattenedValues.resize(indices.size());
        for (size_t i = 0; i < indices.size(); ++i) {
          flattenedValues[i] = values[indices[i]];
        }
      }
      else {
        _attr.getValue(flattenedValues, time);
      }
      return true;
    }
    return false;
  }

}

#endif
