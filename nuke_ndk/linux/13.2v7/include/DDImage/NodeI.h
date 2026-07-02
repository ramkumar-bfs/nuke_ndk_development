// Copyright (c) 2011 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_NodeI_h
#define DDImage_NodeI_h

#include "DDImage/DDImage_API.h"
#include "DDImage/KnobChangedObserverI.h"

#include "DDImage/Hash.h"
#include "DDImage/OutputContext.h"

#include <vector>
#include <string>

class QImage;

namespace DD
{
  namespace Image
  {
    class LinkableI;
    
    class DDImage_API NodeI
    {
    public:
      virtual ~NodeI();
       
    public:
      //! returns an std::vector of all knobs that can be linked to
      virtual std::vector<LinkableI*> getLinkableKnobs() = 0;
      
      //! returns the parent node
      virtual NodeI* getParent() = 0;
      
      //! returns the full name of the node
      virtual std::string getNodeName() const = 0;

      //! return whether we are currently in node creation or not
      virtual bool creatingNewNode() const
      {
        return false;
      }

      //! requests a help system update for this Node.
      virtual void checkExternalDocs() const = 0;

      //! returns the "local" hash of a Node in a given output context. The local hash differs from the Node's complete hash, in that it excludes the hashes of an Op's inputs from the calculation.
      virtual Hash findOpLocalHash(const DD::Image::OutputContext& context) { return Hash(); }

      //! Specify a custom QImage icon to decorate this Node, along with an x/y offset and scale.
      virtual void setCustomIcon(const QImage* image, const float scale, const int offsetX, const int offsetY) {}

      //! Observer mechanism to allow objects inheriting DD::Image::KnobChangedObserverI to be notified of knob_changed events
      virtual void registerKnobChangedObserver(DD::Image::KnobChangedObserverI* observer) = 0;
      virtual void unregisterKnobChangedObserver(DD::Image::KnobChangedObserverI* observer) = 0;

    };
  }
}

#endif // DDImage_NodeI_h
