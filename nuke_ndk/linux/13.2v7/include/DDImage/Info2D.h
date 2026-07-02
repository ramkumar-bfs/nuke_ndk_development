#ifndef DDImage_Info2D_h
#define DDImage_Info2D_h

#include "DDImage_API.h"

#include "DDImage/GeneralInfo.h"
#include "DDImage/Format.h"
#include "DDImage/Box.h"

namespace DD
{
  namespace Image
  {

      /*! \class DD::Image::Info2D

         This structure contains some of the information that Iop::_validate() and DeepOp::_validate()
         must calculate. Because this information is often copied unchanged
         or with tiny variations from the input operator, it is put into
         this structure so that it can be copied with a single assignment.

         This is a subclass of Box and that provides x,y,r,t
         values that describe the bounding box around all the interesting
         pixels (that is, the region of definition). It also provides a merge
         and intersect operators that are useful when _validate is combining
         the info from two inputs.
       */
    class DDImage_API Info2D : public GeneralInfo, public Box
    {
    public:
      /** constructor
       */
      Info2D();

      void setFormats(const FormatPair& formats)
      {
        _format = formats.format();
        _fullSizeFormat = formats.fullSizeFormat();
      }

      FormatPair formats() const
      {
        return FormatPair(_format, _fullSizeFormat);
      }

      /*! \fn const Format& DD::Image::IopInfo::format() const
        
        The image size/format. Operators that care (such as file writers)
        should use this as the size of the image. It can also be used
        as a boundary for an operator that draws data that logically should
        extend to infinity.
        
        The format also has a name and pixel aspect ratio that may be
        useful. Note that Nuke will have already taken the pixel aspect
        ratio into account when storing WH_knob or other dimensional
        knobs.
        
        This is also set by the constructor and can be referred to to
        initialize knob values. For instance if you want an x value to
        default to half way across the image, set it to
        (format().x()+format.r())/2. This format is a \e guess and it
        may be different when the user hooks up the node or finally
        stores values in the knobs.
      */
      const Format& format() const { return *_format; }
      
      /*! \fn void Info2D::format(const Format& f)
        Sets info_.format() to the given format. Some operators expect
        this to tell them how to size their output. Most operators will
        overwrite this in their _validate() method.
      */
      void format(const Format& v) { _format = &v; }
      
      /*! \fn const Format& DD::Image::Info2D::full_size_format() const
        
        If the output of this operator is a proxy for a full size image,
        this is the full size. Nuke assumes all coordinates are given in
        the full size and scales them to the format() when storing them
        into the knob locations. Because Nuke does the scaling most
        operators can ignore this, it is occasionally useful for
        undoing the scaling to get the user's original numbers.
        
        If this is not a proxy then this should point at the same location
        as format().
      */
      const Format& full_size_format() const { return *_fullSizeFormat; }
      
      /*! \fn void Info2D::full_size_format(const Format& f)
        Sets info_.full_size_format() to the given format. Some operators
        expect this to tell them how to size their output. Most operators
        will overwrite this in their _validate() method.
      */
      void full_size_format(const Format& v) { _fullSizeFormat = &v; }
            
      const DD::Image::Box& box() const
      {
        return *this;
      }

      DD::Image::Box& box()
      {
        return *this;
      }

      void setBox(const DD::Image::Box& box)
      {
        set(box);
      }

    private:
      const Format* _format;
      const Format* _fullSizeFormat;
    };

    class DDImage_API Info2DProvider : public GeneralInfoProvider
    {
    public:
      virtual ~Info2DProvider();

      virtual const Info2D& info2D() const = 0;
    };

  }
}

#endif
