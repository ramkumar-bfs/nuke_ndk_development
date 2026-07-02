// Copyright (c) 2023 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef NDK_GEO_RENDER_SAMPLINGRNG_H
#define NDK_GEO_RENDER_SAMPLINGRNG_H

#include "ndk/api.h"

#include "fdk/math/types.h"

#include <vector>

namespace ndk {


  /*! \class SamplingRng
  */
  class NDK_API SamplingRng
  {
  public:
    //!
    SamplingRng(int32_t seed = 27) { setSeed(seed); }

    //! Assign a new seed value to the Rng.
    void setSeed(int32_t seed);
    //! Change the Rng seed to a new pseudo-random value, and returning it.
    int32_t scrambleSeed();

    //! Return the current permutation value for the current seed.
    int32_t  value() const { return _val; }
    //! Change the permutation value and scramble the seed.
    int32_t  newValue();


  public:
    //! Create a set of jittered, well-stratified 1D samples, changing the Rng seed.
    void  generate1dSamples(int32_t n, fdk::FloatList& samples);

    //! Create a set of multi-jittered, well-stratified 2D samples, changing the Rng seed.
    //! The values are distributed in the range -1...+1 in both x & y.
    void  generate2dSamples(int32_t w, int32_t h, fdk::Vec2fList& samples);


  private:
    int32_t _seed, _val;
    int32_t _table[32];
  };

  using SamplingRngList = std::vector<SamplingRng>;


  //--------------------------------------------------------------------------


  /*! \class SamplingRngSet
  */
  class NDK_API SamplingRngSet
  {
  public:
    //!
    SamplingRngSet(int32_t nPnts = 0) : _rngs(nPnts) {}

    int32_t  size() const { return _rngs.size(); }
    void  resize(int32_t nPnts) { _rngs.resize(nPnts); }

    //! SamplingRng for index i
    SamplingRng&  rng(int32_t i) { return _rngs[i]; }

    SamplingRng&       operator [] (int i)       { return _rngs[i]; }
    const SamplingRng& operator [] (int i) const { return _rngs[i]; }


  protected:
    SamplingRngList  _rngs;   //!< List of SamplingRngs, usually 1 per sample point

  };


} // namespace ndk

#endif
