// TextureFilter.h
// Copyright (c) 2014 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_TextureFilter_h
#define DDImage_TextureFilter_h

#include "DDImage/Filter.h"

namespace DD
{
  namespace Image
  {
    /*! \class DD::Image::TextureFilter
       As DD::Image::Filter but specific to sampling texture in 3d rendering context.       
     */
     
    class DDImage_API TextureFilter : public Filter
    {
    public:
    
      // Built-in filter types:
      enum { Nearest = Filter::NumFilters, Bilinear, Trilinear, Anisotropic, NumFilters };
      
      TextureFilter(int type = Filter::Cubic, bool enableMipType = false);
            
      void knobs(Knob_Callback, const char* name = "filter", const char* label = nullptr);
      
      // return true if the filter instance support a mipmap filtering type
      bool isMipTypeEnabled() const 
      {
        return _enableMipType;
      }
      
      // return true if the filter needs mip levels
      bool isMipMap() const 
      {
        return IsMipMap(type_);
      }

      void initialize()
      {
        if (!isMipMap()) {
          Filter::initialize();
        }
      }

      int type() const
      {
        return type_;
      }

      void type(int type)
      {
        type_ = type;

        if (!isMipMap()) {
          Filter::initialize();
        }
      }
      
    private:

      static bool IsMipMap(int t) 
      {
        return t >= Nearest && t <= Anisotropic;
      }
    
      bool _enableMipType;      
    };


    /*! Sample a parallelogram of an iop with a mipmap technique.
    */
    void DDImage_API TextureMipSample(const Vector2& center, const Vector2& dU, const Vector2& dV, TextureFilter* filter, Iop** mip, Pixel& out);

  }
}

#endif

// Copyright (c) 2014 The Foundry Visionmongers Ltd.  All Rights Reserved.
