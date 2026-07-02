// Copyright (c) 2018 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DD_Image_ColorKnobI_H
#define DD_Image_ColorKnobI_H

#include "DDImage/DDImage_API.h"

namespace DD {
  namespace Image {

    class DDImage_API ColorKnobI
    {
    public:
      virtual ~ColorKnobI();

      virtual void setPanelDefaultDroppedState(bool v) = 0;
    };

  }
}

#endif /* DD_Image_ColorKnobI_H */
