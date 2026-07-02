// Copyright (c) 2013 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DD_Image_StringKnobI_H
#define DD_Image_StringKnobI_H

#include "DDImage/DDImage_API.h"

namespace DD {
  namespace Image {

    class OutputContext;
    class Knob;

    //! \class DD::Image::StringKnobI
    //! \brief interface class for an array of characters or a string.
    class StringKnobI
    {
    public:
      static const int kNoSelection = -1; //< indicates no selection in callback

      //! callback for when the cursor position changes
      typedef void (CursorPositionChangedCallback)(Knob* knob, int position, int selectionStart, int selectionEnd);

      //! set the cursor to the absolute position and optionally move the anchor also
      virtual void setCursorPosition(int newPosition, bool moveAnchor)
      {
      }

      //! register a callback object to be called when the cursor position changes
      virtual void registerCursorChangedCallback(CursorPositionChangedCallback* cb)
      {
      }

      //! set the default value for the Knob and take ownership if store is true
      virtual void default_value(const char* s, bool store = false)
      {
      }

      virtual ~StringKnobI() {}
    };
  }
}

#endif
