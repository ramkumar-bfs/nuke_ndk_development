// NullGeo.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_NullGeo_h
#define DDImage_NullGeo_h

#include "GeoOp.h"

#ifdef _WIN32
  #pragma warning(disable:4251)
#endif

namespace DD
{
  namespace Image
  {

    /*!
       This is a stub GeoOp, intended to be the dead-end in a geometry
       tree.  It contains a single empty cache and does no work.
     */
    class DDImage_API NullGeo : public GeoOp
    {
    protected:

      /*! Do nothing - don't call any inputs. */
      /*virtual*/ void get_geometry_hash();

      /*! Do nothing - don't call any inputs. */
      /*virtual*/ void geometry_engine(Scene&, GeometryList& out);

    public:

      /*! Base class sets inputs to zero. */
      NullGeo(Node * node);
      /*virtual*/ int minimum_inputs() const { return 0; }
      /*virtual*/ int maximum_inputs() const { return 0; }

      /*virtual*/ const char* Class() const;
      /*virtual*/ const char* node_help() const;
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
