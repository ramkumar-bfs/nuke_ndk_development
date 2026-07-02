// Copyright (c) 2019 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_Sampler_h
#define DDImage_Sampler_h

#include "DDImage/Pixel.h"
#include "DDImage/Iop.h"
#include "DDImage/Filter.h"

namespace DD
{
  namespace Image
  {
    /// Store information about a position to sample from an image. This class can be
    /// used to support a two-pass sampling approach where the first pass populates a
    /// list of SamplePositions and the second pass uses those positions to perform the
    /// actual sampling. The advantage of this approach is that an accurate bounding box
    /// can be calculated for the region we wish to sample from during the first pass. 
    /// This allows us to create a tile of exactly the size we need when the sampler is
    /// created rather than having to default to the entire input request box.
    struct DDImage_API SamplePosition
    {
      SamplePosition(const Vector2& center, const Vector2& dU, const Vector2& dV, int x);
      SamplePosition(float cx, float cy, float w, float h, int x);

      // Calculate the bounds of the filter for the current position. Used for overall
      // bounding box calculations prior to creating a sampler.
      Box filterBounds(const Filter& filter) const;

      float cx;
      float cy;
      float w;
      float h;
      int x; // Position where sampled pixel should go
    };

    /// A class for sampling pixels from an image. It works like a Tile, but with
    /// sample methods. This is generally more efficient than making many calls to
    /// Iop::sample although it may render more than you need from  the input if
    /// you don't sample all of the requested box.
    class DDImage_API Sampler {
    public:
      /// What to do when sampling off the edges of the input
      enum EdgeHandling {
        eEdgeZero,    // Black outside
        eEdgeClamp,   // Repeat pixels at edges
        eEdgeFromIop, // Use Iop's black_outside setting
      };

      // Note: if no filter is specified at construction time then it must be supplied as an argument to any later calls to sample() instead.
      Sampler( Iop* iop, int x, int y, int r, int t, const ChannelSet& channels, class Filter* filter = nullptr, EdgeHandling edges = eEdgeFromIop, InterestRatchet* ir = nullptr, bool loadOndemand = true);
      Sampler( Iop* iop, const Box& box, const ChannelSet& channels, class Filter* filter = nullptr, EdgeHandling edges = eEdgeFromIop, InterestRatchet* ir = nullptr, bool loadOnDemand = true);
      ~Sampler();

      Iop* iop() const;
      const Format& format() const;
      const ChannelSet& channels() const;
      Filter* filter() const;

      float at(int x, int y, Channel z) const;
      void at(int x, int y, Pixel& out);
      void sample(float cx, float cy, float W, float H, class Pixel& out);
      void sample(float cx, float cy, float W, float H, class Filter* filter, class Pixel& out);
      void sample(const Vector2& center, const Vector2& dU, const Vector2& dV, class Pixel& out);
      void sample(const Vector2& center, const Vector2& dU, const Vector2& dV, class Filter* filter, class Pixel& out);

      // This version is used for two-pass sampling and uses the filter that was previously specified when the sampler was constructed.
      void sample(SamplePosition& samplePosition, class Pixel& out);

    private:
      std::unique_ptr<class SamplerImpl> _pImpl;
    };
  }
}

#endif

