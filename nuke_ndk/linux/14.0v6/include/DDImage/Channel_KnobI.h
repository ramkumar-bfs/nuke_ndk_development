// Copyright (c) 2020 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef DDIMAGE_CHANNEL_KNOBI_H
#define DDIMAGE_CHANNEL_KNOBI_H

namespace DD {
  namespace Image {
    /*! \class DD::Image::Channel_KnobI
     *  \brief The Channel_KnobI class is used to expose the selection handle in DDImage
     */
    class Channel_KnobI
    {
    public:
      Channel_KnobI() = default;
      virtual ~Channel_KnobI() = default;

      enum class SelectionHandling {
        Default,               //!< Channels that are not in the node input get put in the "Other" menu
        AllButNonInputGreyed,  //!< User can select any channel, but those not in the input are greyed out
        AllNormal,             //!< User can select any channel normally
        InputOnly,             //!< Users can only select node input channels
      };

      //! Set the selection handling before the Widget is constructed
      virtual void setSelectionHandling(const SelectionHandling) = 0;
      virtual SelectionHandling selectionHandling() const = 0;
    };
  }
}

#endif
