// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_DeepPixelOp_H
#define DDImage_DeepPixelOp_H

#include "DDImage/DeepFilterOp.h"

namespace DD {
  namespace Image {

    /// DeepPixelOp is a base class that provides similar functionality to the combination of NukeWrapper and PixelOp.
    class DDImage_API DeepPixelOp : public DeepFilterOp
    {
      float _mix;
      
#ifdef NUKE_OBJECT_ID
      int _selectID;
      int _operation;
#endif
      
      bool _limitZ;
      
      double _keyParm[4];

      enum {
        eNone,
        eExcept,
        eOnly,
        eAll
      };
            
    protected:
      float getMix() { return _mix; }
      bool getLimitZ() { return _limitZ; }
      float trapLookup(float v);

      DD::Image::ChannelSet _extraInputChans;
      
      DeepPixelOp(Node* node);

      virtual ~DeepPixelOp();
      
      virtual void in_channels(int, ChannelSet& channels) const;
      
      virtual void getDeepRequests(DD::Image::Box box, const DD::Image::ChannelSet& channels, int count, std::vector<RequestData>& requests);
      
      virtual void processSample(int y,
                                 int x,
                                 const DD::Image::DeepPixel& deepPixel,
                                 size_t sampleNo,
                                 const DD::Image::ChannelSet& channels,
                                 DeepOutPixel& output) const = 0;
      
      virtual void knobs(Knob_Callback f);
      
      virtual int knob_changed(DD::Image::Knob* k);
      
      virtual bool doDeepEngine(DD::Image::Box box, const ChannelSet& channels, DeepOutputPlane& plane);
    };
  }
}

#endif
