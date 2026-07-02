// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef STORETYPE_H
#define STORETYPE_H

#include <set>
#include <map>
#include <string>

#include "Channel.h"

namespace DD
{
  namespace Image
  {

    struct ConvolveArray;
    class Format;
    class FormatPair;
    class Matrix4;
    class LookupCurves;

    enum StoreType {
      DoublePtr,
      FloatPtr,
      StringPtr,
      IntPtr,
      UnsignedIntPtr,
      BoolPtr,
      ConvolveArrayPtr,
      ChannelSetPtr,
      ChannelPtr,
      FormatPairPtr,
      MatrixPtr,
      LookupCurvesPtr,
      IntSetPtr,
      IntIntMapPtr,
      IntPairPtr,
      StringVecVecPtr,
      StlStringPtr,
      StlStringIntPairPtr,
      StlStringVecPtr,
      Custom
    };

	typedef std::pair<std::string, int> StlStringIntPair;
  }
}

#endif
