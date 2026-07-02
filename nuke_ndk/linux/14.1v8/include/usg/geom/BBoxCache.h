// Copyright (c) 2023 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_BBOXCACHE_H
#define USG_GEOM_BBOXCACHE_H

#include "usg/geom/Prim.h"
#include "fdk/math/Box3.h"

namespace usg {

  //! A class for caching bbox queries. Create one of these when you need to query a set of bboxes.
  //! This is not threadsafe, so you should create one per thread if multithreading.
  class USG_API BBoxCache {
  public:
    BBoxCache(fdk::TimeValue time,
              bool           useExtentsHint = false,
              bool           ignoreVisibility = false);
    ~BBoxCache();

    //! Returns the prim's untransformed local bbox.
    fdk::Box3d  getUntransformedLocalBound(const Prim& prim);


  private:
    std::unique_ptr<class BBoxCacheImpl> _pImpl;
  };

}

#endif
