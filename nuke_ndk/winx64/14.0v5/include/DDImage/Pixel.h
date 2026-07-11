// Pixel.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Pixel_h
#define DDImage_Pixel_h

#include "DDImage/ChannelSet.h"
#include "DDImage/Channel3D.h"
#include "fdk/math/Vec4.h"
#include <string.h>

namespace DD
{
  namespace Image
  {

    class InterestRatchet;

    /*! \class Pixel
       Holds a single pixel's worth of image data.  Contains a ChannelSet
       to indicate active planes.
     */
    class DDImage_API Pixel
    {
      InterestRatchet* _interestRatchet;

    public:
      ChannelSet channels;
      float chan[Chan_Last + 1]; // Needs one extra as chan 0 is the black channel

      Pixel(const ChannelSetInit c) : _interestRatchet(nullptr), channels(c) { chan[0] = 0; }
      Pixel(ChannelMask c)          : _interestRatchet(nullptr), channels(c) { chan[0] = 0; }
      Pixel(Channel c)              : _interestRatchet(nullptr), channels(c) { chan[0] = 0; }
      Pixel(const Pixel &b)         : _interestRatchet(nullptr), channels(b.channels) { *this = b; }

      //-----------------------------------------------------

      float&       operator [] (Channel z)       { return chan[z]; }
      const float& operator [] (Channel z) const { return chan[z]; }
      float* array() { return chan; }

      //-----------------------------------------------------

      InterestRatchet*  interestRatchet() { return _interestRatchet; }
      void              setInterestRatchet(InterestRatchet* interestRatchet) { _interestRatchet = interestRatchet; }
      void              copyInterestRatchet(const Pixel& from) { _interestRatchet = from._interestRatchet; }

      //-----------------------------------------------------

      void set(float d)                { foreach(z, channels) { chan[z] = d; } }
      void set(ChannelMask c, float d) { channels = c; set(d); }
      void erase()                     { memset(chan + 1, 0, Chan_Last * sizeof(float)); }
      void erase(Channel z)            { chan[z] = 0.0f; }
      void erase(ChannelMask mask)     { foreach(z, mask) { chan[z] = 0.0f; } }

      void replace(const Pixel& b, ChannelMask c) { foreach(z, c) { chan[z] = b.chan[z]; } }
      void replace(const Pixel& b)                { foreach(z, b.channels) { chan[z] = b.chan[z]; } }

      //-----------------------------------------------------

      // Assignment operators:
      Pixel& operator =  (float d) { foreach(z, channels) { chan[z] = d; } return *this; }

      // Multiply operators:
      Pixel  operator *  (float d) const;
      Pixel& operator *= (float d);
      Pixel  operator *  (const Pixel& v) const;
      Pixel& operator *= (const Pixel& v);

      // Divide operators:
      Pixel  operator /  (float d) const;
      Pixel& operator /= (float d);
      Pixel  operator /  (const Pixel& v) const;
      Pixel& operator /= (const Pixel& v);

      // Addition operators:
      Pixel  operator +  (float d) const;
      Pixel& operator += (float d);
      Pixel  operator +  (const Pixel& v) const;
      Pixel& operator += (const Pixel& v);
      // Subtraction operators:
      Pixel  operator -  (float d) const;
      Pixel& operator -= (float d);
      Pixel  operator -  (const Pixel& v) const;
      Pixel& operator -= (const Pixel& v);

      //-----------------------------------------------------

      //! Destructive linear interpolate between this Pixel b by t.
      void lerp(const Pixel& b, float t);

      void over(const Pixel& A, float a, ChannelMask c);
      void over(const Pixel& A, float a);

      void under(const Pixel& A, float a, ChannelMask c);
      void under(const Pixel& A, float a);

      //-----------------------------------------------------

      //! Convenience color functions
      void setRGBAToBlack() { rgb().set(0.0f); alpha() = 1.0f; }
      void setRGBAToWhite() { rgba().set(1.0f); }

      void setRGBToBlack()  { rgb().set(0.0f); }
      void setRGBToWhite()  { rgb().set(1.0f); }

      void setOpacityToFull() { opacity() = 1.0f; }
      void setOpacityToNone() { opacity() = 0.0f; }

      //-----------------------------------------------------

      // Read/write vector attribute access convenience methods.
      fdk::Vec4f&        P()       { return reinterpret_cast<      fdk::Vec4f&>(chan[Chan_P_]); }
      const fdk::Vec4f&  P() const { return reinterpret_cast<const fdk::Vec4f&>(chan[Chan_P_]); }
      float&             x()       { return P().x; }
      float              x() const { return P().x; }
      float&             y()       { return P().y; }
      float              y() const { return P().y; }
      float&             z()       { return P().z; }
      float              z() const { return P().z; }
      float&             w()       { return P().w; }
      float              w() const { return P().w; }

      float&             Z()       { return chan[Chan_Z        ]; }
      float              Z() const { return chan[Chan_Z        ]; }
      float&            Zf()       { return chan[Chan_DeepFront]; }
      float             Zf() const { return chan[Chan_DeepFront]; }
      float&            Zb()       { return chan[Chan_DeepBack ]; }
      float             Zb() const { return chan[Chan_DeepBack ]; }

      fdk::Vec3f&           PL()       { return reinterpret_cast<      fdk::Vec3f&>(chan[Chan_PL_ ]); }
      const fdk::Vec3f&     PL() const { return reinterpret_cast<const fdk::Vec3f&>(chan[Chan_PL_ ]); }
      fdk::Vec3f&           PW()       { return reinterpret_cast<      fdk::Vec3f&>(chan[Chan_PW_ ]); }
      const fdk::Vec3f&     PW() const { return reinterpret_cast<const fdk::Vec3f&>(chan[Chan_PW_ ]); }
      fdk::Vec3f&           MB()       { return reinterpret_cast<      fdk::Vec3f&>(chan[Chan_MB_ ]); }
      const fdk::Vec3f&     MB() const { return reinterpret_cast<const fdk::Vec3f&>(chan[Chan_MB_ ]); }
      fdk::Vec4f&           UV()       { return reinterpret_cast<      fdk::Vec4f&>(chan[Chan_UV_ ]); }
      const fdk::Vec4f&     UV() const { return reinterpret_cast<const fdk::Vec4f&>(chan[Chan_UV_ ]); }
      fdk::Vec3f&            N()       { return reinterpret_cast<      fdk::Vec3f&>(chan[Chan_N_  ]); }
      const fdk::Vec3f&      N() const { return reinterpret_cast<const fdk::Vec3f&>(chan[Chan_N_  ]); }
      fdk::Vec3f&          VEL()       { return reinterpret_cast<      fdk::Vec3f&>(chan[Chan_VEL_]); }
      const fdk::Vec3f&    VEL() const { return reinterpret_cast<const fdk::Vec3f&>(chan[Chan_VEL_]); }

      fdk::Vec4f&           Cf()       { return reinterpret_cast<      fdk::Vec4f&>(chan[Chan_Red]); }
      const fdk::Vec4f&     Cf() const { return reinterpret_cast<const fdk::Vec4f&>(chan[Chan_Red]); }
      fdk::Vec4f&         rgba()       { return reinterpret_cast<      fdk::Vec4f&>(chan[Chan_Red]); }
      const fdk::Vec4f&   rgba() const { return reinterpret_cast<const fdk::Vec4f&>(chan[Chan_Red]); }
      fdk::Vec3f&          rgb()       { return reinterpret_cast<      fdk::Vec3f&>(chan[Chan_Red]); }
      const fdk::Vec3f&    rgb() const { return reinterpret_cast<const fdk::Vec3f&>(chan[Chan_Red]); }
      float&                 r()       { return chan[Chan_Red  ]; }
      float                  r() const { return chan[Chan_Red  ]; }
      float&                 g()       { return chan[Chan_Green]; }
      float                  g() const { return chan[Chan_Green]; }
      float&                 b()       { return chan[Chan_Blue ]; }
      float                  b() const { return chan[Chan_Blue ]; }
      float&                 a()       { return chan[Chan_Alpha]; }
      float                  a() const { return chan[Chan_Alpha]; }
      float&               red()       { return chan[Chan_Red  ]; }
      float                red() const { return chan[Chan_Red  ]; }
      float&             green()       { return chan[Chan_Green]; }
      float              green() const { return chan[Chan_Green]; }
      float&              blue()       { return chan[Chan_Blue ]; }
      float               blue() const { return chan[Chan_Blue ]; }
      float&             alpha()       { return chan[Chan_Alpha]; }
      float              alpha() const { return chan[Chan_Alpha]; }

      float&            coverage()       { return chan[Chan_Coverage]; }
      float             coverage() const { return chan[Chan_Coverage]; }
      float&         cutoutAlpha()       { return chan[Chan_Cutout_Alpha]; }
      float          cutoutAlpha() const { return chan[Chan_Cutout_Alpha]; }
      float&         cutoutDepth()       { return chan[Chan_Cutout_Depth]; }
      float          cutoutDepth() const { return chan[Chan_Cutout_Depth]; }

      fdk::Vec3f&          color()       { return   rgb(); }
      const fdk::Vec3f&    color() const { return   rgb(); }
      float&             opacity()       { return alpha(); }
      float              opacity() const { return alpha(); }

      fdk::Vec3f&       position()       { return  PW(); }
      const fdk::Vec3f& position() const { return  PW(); }
      fdk::Vec3f&       velocity()       { return VEL(); }
      const fdk::Vec3f& velocity() const { return VEL(); }
      fdk::Vec3f&         normal()       { return   N(); }
      const fdk::Vec3f&   normal() const { return   N(); }

    };



//------------------------------------------------
inline Pixel
Pixel::operator *  (float d) const
{
  Pixel p(channels);
  foreach(z, channels)
    { p.chan[z] = chan[z] * d; }
  return p;
}
inline Pixel&
Pixel::operator *= (float d)
{
  foreach(z, channels) chan[z] *= d;
  return *this;
}
inline Pixel
Pixel::operator *  (const Pixel& v) const
{
  Pixel p(channels);
  foreach(z, v.channels)
    { p.chan[z] = chan[z] * v.chan[z]; }
  return p;
}
inline Pixel&
Pixel::operator *= (const Pixel& v)
{
  foreach(z, v.channels)
    {chan[z] *= v.chan[z]; }
  return *this;
}
//------------------------------------------------
inline Pixel
Pixel::operator /  (float d) const
{
  const float id = 1.0f / d;
  Pixel p(channels);
  foreach(z, channels)
    { p.chan[z] = chan[z] * id; }
  return p;
}
inline Pixel&
Pixel::operator /= (float d)
{
  const float id = 1.0f / d;
  foreach(z, channels)
    { chan[z] *= id; }
  return *this;
}
inline Pixel
Pixel::operator /  (const Pixel& v) const
{
  Pixel p(channels);
  foreach(z, v.channels)
    { p.chan[z] = chan[z] / v.chan[z]; }
  return p;
}
inline Pixel&
Pixel::operator /= (const Pixel& v)
{
  foreach(z, v.channels)
    { chan[z] /= v.chan[z]; }
  return *this;
}
//------------------------------------------------
inline Pixel
Pixel::operator +  (float d) const
{
  Pixel p(channels);
  foreach(z, channels)
    { p.chan[z] = chan[z] + d; }
  return p;
}
inline Pixel&
Pixel::operator += (float d)
{
  foreach(z, channels)
    { chan[z] += d; }
  return *this;
}
inline Pixel
Pixel::operator +  (const Pixel& v) const
{
  Pixel p(channels);
  foreach(z, v.channels)
    { p.chan[z] = chan[z] + v.chan[z]; }
  return p;
}
inline Pixel&
Pixel::operator += (const Pixel& v)
{
  foreach(z, v.channels)
    { chan[z] += v.chan[z]; }
  return *this;
}
//------------------------------------------------
inline Pixel
Pixel::operator -  (float d) const
{
  Pixel p(channels);
  foreach(z, channels)
    { p.chan[z] = chan[z] - d; }
  return p;
}
inline Pixel&
Pixel::operator -= (float d)
{
  foreach(z, channels)
    { chan[z] -= d; }
  return *this;
}
inline Pixel
Pixel::operator -  (const Pixel& v) const
{
  Pixel p(channels);
  foreach(z, v.channels)
    { p.chan[z] = chan[z] - v.chan[z]; }
  return p;
}
inline Pixel&
Pixel::operator -= (const Pixel& v)
{
  foreach(z, v.channels)
    { chan[z] -= v.chan[z]; }
  return *this;
}
//------------------------------------------------
inline void
Pixel::lerp(const Pixel& b, float t)
{
  foreach(z, b.channels)
    { chan[z] = chan[z] * (1 - t) + b.chan[z] * t; }
}

inline void
Pixel::over(const Pixel& A, float a, ChannelMask c)
{
  if (a < 0.0001f)
  {
    foreach(z, c)
      { chan[z] += A.chan[z]; }
  }
  else if (a < 0.9999f)
  {
    a = 1.0f - a;
    foreach(z, c)
      { chan[z] = chan[z] * a + A.chan[z]; }
  }
  else
  {
    foreach(z, c)
      { chan[z] = A.chan[z]; }
  }
}

inline void
Pixel::under(const Pixel& A, float a, ChannelMask c)
{
  if (a < 0.0001f)
  {
    foreach(z, c)
      { chan[z] = A.chan[z]; }
  }
  else if (a < 0.9999f)
  {
    a = 1.0f - a;
    foreach(z, c)
      { chan[z] += A.chan[z] * a; }
  }
  else
  {
    foreach(z, c)
      { chan[z] += A.chan[z]; }
  }
}

inline void
Pixel::over(const Pixel& A, float a)
{
  if (a < 0.0001f)
  {
    for (unsigned z = 1; z <= 4; z++)
      { chan[z] += A.chan[z]; }
  }
  else if (a < 0.9999f)
  {
    a = 1.0f - a;
    for (unsigned z = 1; z <= 4; z++)
      { chan[z] = chan[z] * a + A.chan[z]; }
  }
  else
  {
    for (unsigned z = 1; z <= 4; z++)
      { chan[z] = A.chan[z]; }
  }
}

inline void
Pixel::under(const Pixel& A, float a)
{
  if (a < 0.0001f)
  {
    for (unsigned z = 1; z <= 4; z++)
      { chan[z] = A.chan[z]; }
  }
  else if (a < 0.9999f)
  {
    a = 1.0f - a;
    for (unsigned z = 1; z <= 4; z++)
      { chan[z] += A.chan[z] * a; }
  }
  else
  {
    for (unsigned z = 1; z <= 4; z++)
      { chan[z] += A.chan[z]; }
  }
}


  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
// end of Pixel.h
