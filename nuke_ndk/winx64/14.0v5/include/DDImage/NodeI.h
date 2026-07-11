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
      //! Return the local Node name, without parent path.
      virtual const char* name() const = 0;

      /*! Return the full path of the node which is the Node's path with parents separated
          using the optional separator character. The default separator is a period '.' which
          is typically used for Nuke expressions.

          However periods can cause problems with some path parsers so underscore '_'
          and colon ':' are often good options.

          All printable (std::isprint) characters are allowed, which include space ' '.

          Note that forward-slash '/' is handled specially and the first character of the
          returned path will have a '/' prefix to form an absolute path.
      */
      virtual std::string fullPath(char separator = '.') const = 0;

      //! Returns the full path of the node, including any parents, separated by periods '.'
      virtual std::string getNodeName() const = 0;

      //! Returns an std::vector of all knobs that can be linked to
      virtual std::vector<LinkableI*> getLinkableKnobs() = 0;

      //! Return the parent node
      virtual NodeI* getParent() = 0;

      /*! Hash of input Node pointers unaffected by Knob values, to identify if Node graph topology
          has changed.
          This hashes the name of the Node, the input indices order and each input's topologyHash(),
          so the topology hash will also reflect the names of all upstream nodes and can be used to
          indicate when those names are changed.
      */
      virtual Hash topologyHash() = 0;

      //! Return whether we are currently in node creation or not
      virtual bool creatingNewNode() const { return false; }

      //! Request a help system update for this Node.
      virtual void checkExternalDocs() const = 0;

      /*! Return the Op hash of a Node at a specific OutputContext if the Node contains one, otherwise
          a default Hash is returned.
          Note that just because the Node contains an Op with that hash does not mean the Op is useable,
          always check the Op is valid and constructed.
      */
      virtual Hash findOpHash(const DD::Image::OutputContext& context) const { return Hash(); }

      /*! Retrieve an Op owned by the Node with a matching hash, or null if none found.
          You must check that the returned Op is useable, valid and constructed.
      */
      virtual Op* findOpByHash(const DD::Image::Hash& hash) const { return nullptr; }

      //! Get the hashes of all Ops owned by this Node.
      virtual std::vector<DD::Image::Hash> opHashes() const { return std::vector<DD::Image::Hash>(); }

      //! Specify a custom QImage icon to decorate this Node, along with an x/y offset and scale.
      virtual void setCustomIcon(const QImage* image, const float scale, const int offsetX, const int offsetY) {}

      //! Observer mechanism to allow objects inheriting DD::Image::KnobChangedObserverI to be notified of knob_changed events
      virtual void registerKnobChangedObserver(DD::Image::KnobChangedObserverI* observer) = 0;
      virtual void unregisterKnobChangedObserver(DD::Image::KnobChangedObserverI* observer) = 0;

    };
  }
}

#endif // DDImage_NodeI_h
