// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FDK_MATH_MATH_H
#define FDK_MATH_MATH_H

#include "fdk/api.h"

#include "fdk/math/half.h" // for Vec2,Vec3,etc templates

#include <cmath> // For M_PI_2, etc
#include <type_traits>
#include <vector>

#ifdef _WIN32

  #ifndef M_PI_2
    #define M_PI_2    1.57079632679489661923
  #endif
  #ifndef M_2_PI
    #define M_2_PI    0.63661977236758134308
  #endif

#endif

namespace fdk {

  //! Convert degrees to radians.
  template <typename T>
  inline T     radians(T degrees) { return degrees * T(M_PI_2 / 90); }

  //! Convert radians to degrees.
  template <typename T>
  inline T     degrees(T radians) { return radians * T(90 * M_2_PI); }



  //! Returns true if val1 is nearly val2, within the threshold range.
  template <typename T>
  inline bool  isNearly(T val1, T val2, T threshold =std::numeric_limits<T>::epsilon()) { return (::fabs(val1 - val2) <= threshold); }

  //! Returns true if value is nearly 0, within the threshold range.
  template <typename T>
  inline bool  isNearlyZero(T v, T threshold =std::numeric_limits<T>::epsilon()) { return (::fabs(v) <= threshold); }

  //! Returns true if value is nearly 1, within the threshold range.
  template <typename T>
  inline bool  isNearlyOne(T v, T threshold =std::numeric_limits<T>::epsilon()) { return (::fabs((T)1 - v) <= threshold); }

  template <typename T>
  constexpr bool is_arithmetic = std::is_arithmetic<T>::value || std::is_same<fdk::half, typename std::remove_cv<T>::type>::value;

  template <typename T>
  constexpr bool is_floating_point = std::is_floating_point<T>::value || std::is_same<fdk::half, typename std::remove_cv<T>::type>::value;

}

#endif
