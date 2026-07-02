// OutputContext.h
// Copyright (c) 2013 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef OutputContext_H
#define OutputContext_H

#include "DDImage/DDImage_API.h"
#include "DDImage/ProxyContext.h"
#include "math.h"
#include "DDImage/Box.h"
#include "DDImage/Deprecation.h"

#include <Build/fnBuild.h>
#include <string>
#include <vector>

class Viewer_Window;
class Node;

namespace DD
{
  namespace Image
  {
    class Hash;
    class Op;

    /*! \class DD::Image::OutputContext

       This is a block of data that is passed *up* the Op tree. The context
       modifies the values that are stored by knobs. An Op can further use
       information from it (such as the frame number) to modify it's
       behavior, by implementing Op::setOutputContext(). And an Op can
       modify the context that is used by each input, by implementing
       Op::inputContext().

     */
    class DDImage_API OutputContext : public ProxyContext
    {
    public:

      /*!
        Preferences which can be used to influence how files are read by Readers.
        eScanlineAlways - upstream Reader should always read frames line by line.
        ePreferScanline - prefer line-by-line reads, but allow other nodes upstream to override this preference.
        ePreferAllLines - prefer to read the whole of the requested area at once, but allow other nodes upstream to override this preference.
      */
      enum LineReadPreference
      {
        eScanlineAlways,
        ePreferScanline,
        ePreferAllLines
      };

      typedef ProxyContext::ProxySetting ProxySetting;

      OutputContext() :
        frame_(0), view_(0),
        lineReadPreference_(ePreferScanline)
      {}

      // TODO : move to pImpl
      static const OutputContext default_;

      static const OutputContext& defaultContext()
      {
        return default_;
      }

      double frame() const { return frame_; }
      void setFrame(double v) { frame_ = v; }
      int view() const { return view_; }
      void setView(int v) { view_ = v; }
      void view(int v) { if (v == -1 || v == 0)
                           view_ = 1;
                         else
                           view_ = v;}                           // deprecated

      /*!
        By default readers read frames line by line (ePreferScanline). Some
        formats support reading the entire frame in one go.
        Setting the lineReadPreference can be used to switch between the two
        modes for these formats.
      */
      void setLineReadPreference(LineReadPreference linePreference);
      LineReadPreference lineReadPreference() const { return lineReadPreference_; }

      /*!
         Add the contents of this to the hash, thus producing a different hash for any
         different setting of OutputContext.
       */
      void append(Hash&) const;

      bool operator==(const OutputContext& other) const
      {
        return frame_ == other.frame_ &&
               view_  == other.view_ &&
               lineReadPreference_ == other.lineReadPreference_ &&
               ProxyContext::isEqualTo(other);
      }

      bool operator!=(const OutputContext& other) const
      {
        return !(*this == other);
      }

      bool operator<(const OutputContext& other) const
      {
        if (frame_ < other.frame_) {
          return true;
        }

        if (frame_ > other.frame_) {
          return false;
        }

        if (view_ < other.view_) {
          return true;
        }

        if (view_ > other.view_) {
          return false;
        }

        if (lineReadPreference_ < other.lineReadPreference_) {
          return true;
        }

        if (lineReadPreference_ > other.lineReadPreference_) {
          return false;
        }

        if (ProxyContext::isLessThan(other)) {
          return true;
        }

        return false;
      }

      bool operator>(const OutputContext& other) const
      {
        return !((*this)<other);
      }

      static const std::string& viewname(int v);
      static const std::string& viewshort(int v);

      /**
       * how many views exist.  View 0 is always an extra view called 'default'.  If there
       * is one view, this is will be view 1.
       */
      static int viewcount();

      /**
       * whether or not there are more than one actual view (not counting the hidden 'default' view)
       */
      static bool MultiView();

      const std::string& viewname() const
      {
        return viewname(view_);
      }

      static int lookup_view(const std::string& viewname);

      // s is a newline-separated list of view names.
      static void set_viewnames(const std::string& s);

      static void (* create_view)(const std::string& s);

      const Format* to_proxy_rel(const Format& from, Format& to, const OutputContext& rel) const
      {
        return ProxyContext::to_proxy_rel(from, to, rel);
      }

      bool sameProxy(const DD::Image::OutputContext& other) const
      {
        return ProxyContext::isEqualTo(other);
      }

      // Merge with lookup_view
      //! If a view exists with name \a viewName on \a op's root node then that
      //! view's index is returned, otherwise -1 is returned.
      static int (*viewIndex)(const std::string& viewName, Op* op);

      // Merge with create_view
      //! Creates a new view with name \a viewName and \a color on \a op's root
      //! node.
      static void (*createView)(const std::string& viewName, const std::string& color, Op* op);

      // Merge with viewname
      //! If a view exists at \a index on \a op's root node then that view's
      //! name is returned, otherwise an empty string is returned.
      static const std::string& (*viewName)(int index, Op* op);

      // Merge with viewshort
      //! If a view exists at \a index on \a op's root node then that view's
      //! short name is returned, otherwise an empty string is returned.
      static std::string (*viewShortName)(int index, Op* op);

      // Merge with set_viewnames
      //! Sets the \a op's root node to have view's named \a viewNames.
      static void (*setViewNames)(const std::vector<std::string>& viewNames, Op* op);

      // Merge with viewcount
      //! Returns the number of views on the \a op's root node.
      static int (*viewCount)(Op* op);

      // Merge with MultiView
      //! Returns true if \a op's root node has more than one view.
      static bool (*hasMultipleViews)(Op* op);

    private:

      double frame_;   //< the frame id for which the current context applies, may not match what is being shown in the Viewer
      int view_;
      LineReadPreference lineReadPreference_;

      friend DDImage_API std::ostream& operator << (std::ostream& o, const OutputContext&);

    };
  }
}

#endif // OutputContext_H

