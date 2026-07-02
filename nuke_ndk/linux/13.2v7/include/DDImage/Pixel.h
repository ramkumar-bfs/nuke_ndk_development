// Pixel.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Pixel_h
#define DDImage_Pixel_h

#include "ChannelSet.h"
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
      Pixel(ChannelMask c) : _interestRatchet(nullptr), channels(c) { chan[0] = 0; }
      Pixel(Channel c) : _interestRatchet(nullptr), channels(c) { chan[0] = 0; }
      Pixel(const Pixel &b) : _interestRatchet(nullptr), channels(b.channels) { *this = b; }

      void setInterestRatchet(InterestRatchet* interestRatchet)
      {
        _interestRatchet = interestRatchet;
      }

      InterestRatchet* interestRatchet()
      {
        return _interestRatchet;
      }

      void copyInterestRatchet(const Pixel& from)
      {
        _interestRatchet = from._interestRatchet;
      }

      void set(float d) { foreach(z, channels) chan[z] = d; }
      void set(ChannelMask c, float d) { channels = c;
                                         set(d); }
      void erase() { memset(chan + 1, 0, Chan_Last * sizeof(float)); }
      void erase(Channel z) { chan[z] = 0.0f; }
      void erase(ChannelMask mask) { foreach(z, mask) chan[z] = 0.0f; }

      void replace(const Pixel& b, ChannelMask c)
      {
        foreach(z, c) chan[z] = b.chan[z];
      }

      void replace(const Pixel& b)
      {
        foreach(z, b.channels) chan[z] = b.chan[z];
      }

      float& operator [] (Channel z) { return chan[z]; }
      const float& operator [] (Channel z) const { return chan[z]; }
      float* array() { return chan; }

      // Assignment operators:
      Pixel& operator =  (float d)
      {
        foreach(z, channels) chan[z] = d;
        return *this;
      }

      // Multiply operators:
      Pixel  operator *  (float d) const
      {
        Pixel p(channels);
        foreach(z, channels) p.chan[z] = chan[z] * d;
        return p;
      }
      Pixel& operator *= (float d)
      {
        foreach(z, channels) chan[z] *= d;
        return *this;
      }
      Pixel  operator *  (const Pixel& v) const
      {
        Pixel p(channels);
        foreach(z, v.channels) p.chan[z] = chan[z] * v.chan[z];
        return p;
      }
      Pixel& operator *= (const Pixel& v)
      {
        foreach(z, v.channels) chan[z] *= v.chan[z];
        return *this;
      }

      // Divide operators:
      Pixel  operator /  (float D) const
      {
        float d = 1.0f / D;
        Pixel p(channels);
        foreach(z, channels) p.chan[z] = chan[z] * d;
        return p;
      }
      Pixel& operator /= (float D)
      {
        float d = 1.0f / D;
        foreach(z, channels) chan[z] *= d;
        return *this;
      }
      Pixel  operator /  (const Pixel& v) const
      {
        Pixel p(channels);
        foreach(z, v.channels) p.chan[z] = chan[z] / v.chan[z];
        return p;
      }
      Pixel& operator /= (const Pixel& v)
      {
        foreach(z, v.channels) chan[z] /= v.chan[z];
        return *this;
      }

      // Addition operators:
      Pixel  operator +  (float d) const
      {
        Pixel p(channels);
        foreach(z, channels) p.chan[z] = chan[z] + d;
        return p;
      }
      Pixel& operator += (float d)
      {
        foreach(z, channels) chan[z] += d;
        return *this;
      }
      Pixel  operator +  (const Pixel& v) const
      {
        Pixel p(channels);
        foreach(z, v.channels) p.chan[z] = chan[z] + v.chan[z];
        return p;
      }
      Pixel& operator += (const Pixel& v)
      {
        foreach(z, v.channels) chan[z] += v.chan[z];
        return *this;
      }

      // Subtraction operators:
      Pixel  operator -  (float d) const
      {
        Pixel p(channels);
        foreach(z, channels) p.chan[z] = chan[z] - d;
        return p;
      }
      Pixel& operator -= (float d)
      {
        foreach(z, channels) chan[z] -= d;
        return *this;
      }
      Pixel  operator -  (const Pixel& v) const
      {
        Pixel p(channels);
        foreach(z, v.channels) p.chan[z] = chan[z] - v.chan[z];
        return p;
      }
      Pixel& operator -= (const Pixel& v)
      {
        foreach(z, v.channels) chan[z] -= v.chan[z];
        return *this;
      }

      //  bool   operator == (const Pixel& v) const {return x == v.x && y == v.y;}
      //  bool   operator != (const Pixel& v) const {return x != v.x || y != v.y;}

      /*! Destructive linear interpolate between this Pixel b by t. */
      void lerp(const Pixel& b, float t)
      {
        foreach(z, b.channels)
        chan[z] = chan[z] * (1 - t) + b.chan[z] * t;
      }

      void over(const Pixel& A, float a, ChannelMask c)
      {
        if (a < 0.0001f) { foreach(z, c) chan[z] += A.chan[z]; }
        else if (a < 0.9999f) {
          a = 1.0f - a;
          foreach(z, c) chan[z] = chan[z] * a + A.chan[z];
        }
        else { foreach(z, c) chan[z] = A.chan[z]; }
      }

      void under(const Pixel& A, float a, ChannelMask c)
      {
        if (a < 0.0001f)
          foreach(z, c) chan[z] = A.chan[z];
        else if (a < 0.9999f) {
          a = 1.0f - a;
          foreach(z, c) chan[z] += A.chan[z] * a;
        }
        else
          foreach(z, c) chan[z] += A.chan[z];
      }

      void over(const Pixel& A, float a)
      {
        if (a < 0.0001f) {
          for (unsigned z = 1; z <= 4; z++)
            chan[z] += A.chan[z];
        }
        else if (a < 0.9999f) {
          a = 1.0f - a;
          for (unsigned z = 1; z <= 4; z++)
            chan[z] = chan[z] * a + A.chan[z];
        }
        else {
          for (unsigned z = 1; z <= 4; z++)
            chan[z] = A.chan[z];
        }
      }

      void under(const Pixel& A, float a)
      {
        if (a < 0.0001f) {
          for (unsigned z = 1; z <= 4; z++)
            chan[z] = A.chan[z];
        }
        else if (a < 0.9999f) {
          a = 1.0f - a;
          for (unsigned z = 1; z <= 4; z++)
            chan[z] += A.chan[z] * a;
        }
        else {
          for (unsigned z = 1; z <= 4; z++)
            chan[z] += A.chan[z];
        }
      }

    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
// end of Pixel.h
