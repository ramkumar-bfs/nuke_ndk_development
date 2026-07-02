// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef TRANSFORM2DKNOBI_H
#define TRANSFORM2DKNOBI_H

#include "DDImage/Knob.h"
#include "DDImage/Matrix4.h"
#include "DDImage/OutputContext.h"
#include "DDImage/DDImage_API.h"

namespace DD
{
  namespace Image
  {
    // specifies the type of control points the knob contains
    class DDImage_API Transform2d_KnobI
    {
    public:

      enum KNOB_ID
      {
        TRANSLATE,
        ROTATE,
        SCALE,
        // SKEW knob has been deprecated. Please use SKEWX instead
        SKEW,
        CENTER,
        GAUGE,
        SKEWX,
        SKEWY,
        SKEWORDER
      };

      virtual ~Transform2d_KnobI();
      
      virtual DD::Image::Knob* translateKnob() = 0;
          
      virtual DD::Image::Knob* rotateKnob() = 0;
      
      virtual DD::Image::Knob* scaleKnob() = 0;

      // Skew knob has been deprecated. Please use skewXKnob instead
      //virtual DD::Image::Knob* skewKnob() = 0;
      
      virtual DD::Image::Knob* skewXKnob() = 0;

      virtual DD::Image::Knob* skewYKnob() = 0;

      virtual DD::Image::Knob* skewOrderKnob() = 0;

      virtual DD::Image::Knob* centerKnob() = 0;
      
      virtual DD::Image::Knob* gaugeKnob() = 0;

      virtual void setAutoKey( bool autoKey ) = 0;

      /**
       * this should help identify critical moments when a single knob
       * may set its auto key to false in order to perform reset or any other sensitive operation that may trigger auto key
       */
      virtual bool isKnobAutoKeying( KNOB_ID knob ) = 0;
    };    
  }
}

#endif // TRANSFORM2DKNOBI_H
