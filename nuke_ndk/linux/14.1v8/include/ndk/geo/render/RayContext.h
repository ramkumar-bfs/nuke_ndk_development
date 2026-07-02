// Copyright (c) 2023 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef NDK_GEO_RENDER_RAYCONTEXT_H
#define NDK_GEO_RENDER_RAYCONTEXT_H

#include "ndk/api.h"

#include "fdk/math/Vec3.h"
#include "fdk/math/Mat4.h"
#include "fdk/base/Time.h"

#include <vector>

namespace ndk {

//-------------------------------------------------------------------------


  //! Hit point information about an intersected surface/object.
  //! If distance is <= 0.0 or inf then no intersection occurred.
  template<class T>
  struct NDK_API RayHitPoint
  {
    fdk::Vec3<T> PW;        //!< Hit point location
    fdk::Vec3f   Ng;        //!< Geometric normal at hit point
    T            distance;  //!< Distance to hit point - 0.0 (or inf) means a miss (no hit)
    fdk::Vec2f   uv;        //!< UV coordinate at hit point
    fdk::Vec2f   duv;       //!< UV ray radius
    //
    uint64_t     id;        //!< Unique Id from the hit geometry (avoid self-intersections)
  };

  using RayHitPointf = RayHitPoint<float>;
  using RayHitPointd = RayHitPoint<double>;
  using RayHitPointfList = std::vector<RayHitPointf>;
  using RayHitPointdList = std::vector<RayHitPointd>;


  //----------------------------------------------------------------------


  /*! \class RayContext

      \a dxDir & \a dyDir differential direction vectors define a cone around
      the ray's direction vector.

      The difference in angle between direction and dxDir is the cone's
      horizontal radius, and the angle between direction and dyDir is the
      cone's vertical radius.

  */
  class NDK_API RayContext
  {
    public:
      using TypeMask = uint8_t;
      using SidesMode = uint8_t;

      // These are public for ease of access:
      fdk::Vec3f      origin;     //!< Ray origin
      fdk::Vec3f      direction;  //!< Ray direction
      double          minT;       //!< The minimum distance for the ray
      double          maxT;       //!< The maximum distance for the ray
      //
      fdk::Vec3f      dxDir;      //!< Ray spread X-direction normal.
      fdk::Vec3f      dyDir;      //!< Ray spread Y-direction normal.
      //
      fdk::TimeValue  time;       //!< The current time for the ray
      TypeMask        typeMask;   //!< Type of ray
      SidesMode       sidesMode;  //!< 0=both, 1=front, 2=back

      //----------------------------------------------------------------------------
      // Ray types, defined as uints vs. enums so they can be OR'd & AND'd together:
      //
      static constexpr TypeMask       cameraPath() { return 0x01; }   //!< Ray coming from camera (to camera actually...)
      static constexpr TypeMask       shadowPath() { return 0x02; }   //!< Ray from surface to light
      static constexpr TypeMask   reflectionPath() { return 0x04; }   //!< Ray reflected off a surface
      static constexpr TypeMask transmissionPath() { return 0x08; }   //!< Ray refracted or transmitted through a surface
      //
      static constexpr TypeMask      diffusePath() { return 0x10; }   //!< A hint about the last-hit surface this ray will contribute to
      static constexpr TypeMask       glossyPath() { return 0x20; }   //!< A hint about the last-hit surface this ray will contribute to

      // Commonly-used combos:
      static constexpr TypeMask   diffuseReflectionPath() { return (diffusePath() |   reflectionPath()); }
      static constexpr TypeMask diffuseTransmissionPath() { return (diffusePath() | transmissionPath()); }
      static constexpr TypeMask    glossyReflectionPath() { return (glossyPath()  |   reflectionPath()); }
      static constexpr TypeMask  glossyTransmissionPath() { return (glossyPath()  | transmissionPath()); }


      // TypeMask convenience methods:
      bool         isCameraPath() const { return (typeMask &       cameraPath()); }
      bool         isShadowPath() const { return (typeMask &       shadowPath()); }
      bool      isReflectedPath() const { return (typeMask &   reflectionPath()); }
      bool    isTransmittedPath() const { return (typeMask & transmissionPath()); }
      //
      bool isDiffuseContributor() const { return (typeMask &      diffusePath()); }
      bool  isGlossyContributor() const { return (typeMask &       glossyPath()); }


    public:
      //! Default constructor leaves everything uninitialized
      RayContext() {}

      RayContext(const fdk::Vec3f& _origin,
                 const fdk::Vec3f& _dir,
                 TypeMask          _type,
                 fdk::TimeValue    _time,
                 double            _minDist,
                 double            _maxDist = std::numeric_limits<double>::infinity());

      //! Ray inverse direction vector (reciprocal)
      const fdk::Vec3f& invDir() const { return _inv_dir; }

      //! Get the point along ray direction at distance \a t.
      fdk::Vec3d  getPositionAt(double t) const { return origin + (direction * t); }

      //! Sets the ray context and updates slopes.
      void  set(const fdk::Vec3f& _origin,
                const fdk::Vec3f& _dir,
                TypeMask          _type,
                fdk::TimeValue    _time,
                double            _minDist,
                double            _maxDist);

      //! Sets direction vector and updates slopes.
      void  setDirection(const fdk::Vec3f& dir) { direction = dir; updateSlopes(); }

      //! Sets direction vector and distances and updates slopes.
      void  setDirAndDistance(const fdk::Vec3f& dir,
                              double            min,
                              double            max) { direction = dir; minT = min; maxT = max; updateSlopes(); }

      //! Initialize the differentials from two rotation angles (in radians).
      void  setXYDirsFromAngle(float  radian_rX,
                               float  radian_rY);


      //! Recalcs the slope test speedup vars - do this after any change to ray direction.
      void  updateSlopes();
      bool  isSlopePositive(int32_t i) const { return (_slope_positive & (0x01 << i)); }
      bool  isXSlopePositive() const { return (_slope_positive & 0x01); }
      bool  isYSlopePositive() const { return (_slope_positive & 0x02); }
      bool  isZSlopePositive() const { return (_slope_positive & 0x04); }

      //! Transform the ray origin and direction in place by a matrix and updates slopes.
      void  transform(const fdk::Mat4d& xform);

      //! Transform the ray origin and direction in place by two matrices, interpolated at t, and updates slopes.
      void  transform(const fdk::Mat4d& m0,
                      const fdk::Mat4d& m1,
                      float             t);


    protected:
      fdk::Vec3f  _inv_dir;           //!< Direction reciprocal, for AABB intersection test speedups
      uint8_t     _slope_positive;    //!< xyz positive slope indicator - true if slope is positive for an axis
      uint8_t     pad1;

  };



  /*----------------------------------*/
  /*        Static operations         */
  /*----------------------------------*/


  //! Print out components to a stream.
  template<typename T>
  std::ostream& operator << (std::ostream& o, const RayContext& r);


/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/
/*                   Inline Function Implementations                   */
/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/


inline std::ostream& operator << (std::ostream& o, const RayContext& rtx)
{
  //o.setf(ios::fixed, ios::floatfield);
  //o.precision(7);
  o << rtx.origin << ",dir" << rtx.direction;
  o << "(" << rtx.minT << ".." << rtx.maxT << ")";
  o << ",dx" << rtx.dxDir << ",dy" << rtx.dyDir;
  //o << rtx.invDir();
  //o << "/[" << rtx.isXSlopePositive() << " " << rtx.isYSlopePositive() << " " << rtx.isZSlopePositive() << "]";
  return o;
}

inline void
RayContext::updateSlopes()
{
  _inv_dir.set(1.0f/direction.x,
               1.0f/direction.y,
               1.0f/direction.z);
  _slope_positive = 0x00;
  if (_inv_dir.x < 0.0f) { _slope_positive |= 0x01; }
  if (_inv_dir.y < 0.0f) { _slope_positive |= 0x02; }
  if (_inv_dir.z < 0.0f) { _slope_positive |= 0x04; }
}

inline
RayContext::RayContext(const fdk::Vec3f& _origin,
                       const fdk::Vec3f& _dir,
                       TypeMask          _type,
                       fdk::TimeValue    _time,
                       double            _minDist,
                       double            _maxDist) :
  origin(_origin),
  direction(_dir),
  minT(_minDist),
  maxT(_maxDist),
  dxDir(0.0),
  dyDir(0.0),
  time(_time),
  typeMask(_type),
  sidesMode(0/*both*/)
{
  updateSlopes();
}

inline void
RayContext::set(const fdk::Vec3f& _origin,
                const fdk::Vec3f& _dir,
                TypeMask          _type,
                fdk::TimeValue    _time,
                double            _minDist,
                double            _maxDist)
{
  origin = _origin;
  direction = _dir;
  minT = _minDist;
  maxT = _maxDist;
  dxDir.set(0.0);
  dyDir.set(0.0);
  time = _time;
  typeMask = _type;
  sidesMode = 0/*both*/;
  updateSlopes();
}

inline void
RayContext::transform(const fdk::Mat4d& xform)
{
  origin = xform.transform(origin);
  direction = xform.vecTransform(direction);
  updateSlopes();
}

// Transform by two matrices, interpolated at t.
inline void
RayContext::transform(const fdk::Mat4d& m0,
                      const fdk::Mat4d& m1,
                      float             t)
{
  origin = m0.transform(origin).interpolateTo(m1.transform(origin), t);
  direction = m0.vecTransform(direction).interpolateTo(m1.vecTransform(direction), t);
  updateSlopes();
}

inline void
RayContext::setXYDirsFromAngle(float  radian_rX,
                               float  radian_rY)
{
  radian_rX = std::fabs(radian_rX);
  if (radian_rX > std::numeric_limits<float>::epsilon()) {
    if (radian_rX > M_PI) {
      radian_rX = M_PI;
    }
    const float dx = std::sin(radian_rX);
    const float dz = std::cos(radian_rX);
    dxDir.set(dx, 0.0f, dz);
    dxDir.orientAroundNormal(direction);
  }
  else {
    dxDir = direction;
  }

  radian_rY = std::fabs(radian_rY);
  if (radian_rY > std::numeric_limits<float>::epsilon()) {
    if (radian_rY > M_PI) {
      radian_rY = M_PI;
    }
    const float dy = std::sin(radian_rY);
    const float dz = std::cos(radian_rY);
    dyDir.set(0.0f, dy, dz);
    dyDir.orientAroundNormal(direction);
  }
  else {
    dyDir = direction;
  }
}


} // namespace ndk

#endif
