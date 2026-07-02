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
    int32_t      sampleIndex;   //!< Index of the sample creating this intersection
    //
    fdk::Vec3<T> P;             //!< Hit point location
    fdk::Vec3f   Ng;            //!< Geometric normal at hit point
    T            distance;      //!< Distance to hit point - 0.0 (or inf) means a miss (no hit)
    float        u, v, w;       //!< Barycentric triangle uvw coordinates at intersection P
    fdk::Vec2f   duv;           //!< uv ray radius
    //
    uint64_t     id;            //!< Unique Id from the hit geometry (avoid self-intersections)
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

      //! Intersection testing sides mode
      enum SidesMode : uint8_t {
        SIDES_BOTH  = 0,
        SIDES_FRONT = 1,
        SIDES_BACK  = 2
      };

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
      int32_t         id;         //!< Id of ray, usually the sample index

      //----------------------------------------------------------------------------
      // Ray types, defined as uints vs. enums so they can be OR'd & AND'd together:
      static constexpr TypeMask        emptyPath() { return 0x00; }
      static constexpr TypeMask       cameraPath() { return 0x01; } //!< Ray emitted from a camera
      static constexpr TypeMask       shadowPath() { return 0x02; } //!< Ray from surface to light to determine shadow transmission
      static constexpr TypeMask   reflectionPath() { return 0x04; } //!< Ray reflected off a surface
      static constexpr TypeMask transmissionPath() { return 0x08; } //!< Ray refracted or transmitted through a surface
      //
      static constexpr TypeMask      diffusePath() { return 0x10; } //!< Path is a diffuse reflection off a surface
      static constexpr TypeMask       glossyPath() { return 0x20; } //!< Path is a glossy reflection off a surface
      //
      static constexpr TypeMask         typeBits() { return 0x0f; }
      static constexpr TypeMask         pathBits() { return 0xf0; }
      static constexpr TypeMask         allPaths() { return 0xff; }

      // Commonly-used ray type combos:
      static constexpr TypeMask   diffuseReflectionPath() { return (diffusePath() |   reflectionPath()); }
      static constexpr TypeMask diffuseTransmissionPath() { return (diffusePath() | transmissionPath()); }
      static constexpr TypeMask    glossyReflectionPath() { return (glossyPath()  |   reflectionPath()); }
      static constexpr TypeMask  glossyTransmissionPath() { return (glossyPath()  | transmissionPath()); }

      // TypeMask convenience methods:
      TypeMask          rayType() const { return (typeMask & typeBits()); }
      TypeMask         pathType() const { return (typeMask & pathBits()); }
      //
      bool         isCameraPath() const { return (rayType() ==       cameraPath()); }
      bool         isShadowPath() const { return (rayType() ==       shadowPath()); }
      bool      isReflectedPath() const { return (rayType() ==   reflectionPath()); }
      bool    isTransmittedPath() const { return (rayType() == transmissionPath()); }
      //
      bool isDiffuseContributor() const { return (pathType() & diffusePath()); }
      bool  isGlossyContributor() const { return (pathType() &  glossyPath()); }


    public:
      //! Default constructor leaves everything uninitialized
      RayContext() {}

      RayContext(const fdk::Vec3f& _origin,
                 const fdk::Vec3f& _dir,
                 TypeMask          _type,
                 fdk::TimeValue    _time,
                 int32_t           _id,
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
                int32_t           _id,
                double            _minDist,
                double            _maxDist);

      //! Sets origin, direction vector and updates slopes.
      void  setOriginAndDirection(const fdk::Vec3f& _origin,
                                  const fdk::Vec3f& _dir);

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

      //! Reverse the direction of the ray, which is just negation and update of slopes.
      void  invert();

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

  using RayContextList         = std::vector<RayContext>;
  using RayContextPtrList      = std::vector<RayContext*>;
  using RayContextConstPtrList = std::vector<const RayContext*>;


  //----------------------------------------------------------------------


  struct RayContextBundle
  {
    RayContextBundle() {}
    RayContextBundle(RayContextList* list,
                     int32_t         startIndex) : _list(list), _start(startIndex), _size(0) {}

    int32_t  startIndex() const { return _start; }
    int32_t  endIndex() const { return (_start + _size); }
    int32_t  size() const { return _size; }

    //! Add another RayContext to the bundle, returning a reference to it and incrementing
    //! the current bundle count. This method assumes the bundle currently 'owns' the
    //! RayContextList by a single thread and is always appending to the end of the list.
    RayContext&  add(const RayContext& rtx = RayContext()); // copy
    RayContext&  add(const RayContext&& rtx); // move

          RayContext&  operator [] (int i)       { return (*_list)[_start + i]; }
    const RayContext&  operator [] (int i) const { return (*_list)[_start + i]; }

    //! Returns the first RayContext in the bundle. Caution - if this bundle is empty the returned
    //! RayContext may be from a previous bundle!
    RayContext&        front() { return (*_list)[_start]; }
    //! Returns the last RayContext in the bundle. Caution - if this bundle is empty the returned
    //! RayContext may be from a previous bundle!
    RayContext&        back() { return _list->back(); }

    RayContext*        array() { return (_list->data() + _start); }
    const RayContext*  array() const { return (_list->data() + _start); }

    //! Iterator support
    typedef RayContextList::iterator       iterator;
    typedef RayContextList::const_iterator const_iterator;

    const_iterator  begin() const { return _list->begin(); }
    const_iterator  end()   const { return _list->end(); }

    //! Sets size of bundle to 0 and resizes list to start of bundle (retaining contents of prior bundles.)
    void clear() { _list->resize(_start); _size = 0; }


  private:
    RayContextList* _list;      //!< The list of RayContexts this bundle is adding to
    int32_t         _start;     //!< Absolute starting index cache list
    int32_t         _size;      //!< Absolute inclusive-end index (loop is index=start; index < end)
  };

  using RayContextBundleList = std::vector<RayContextBundle>;



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
                       int32_t           _id,
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
  sidesMode(SIDES_BOTH),
  id(_id)
{
  updateSlopes();
}

inline void
RayContext::set(const fdk::Vec3f& _origin,
                const fdk::Vec3f& _dir,
                TypeMask          _type,
                fdk::TimeValue    _time,
                int32_t           _id,
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
  id = _id;
  typeMask = _type;
  sidesMode = SIDES_BOTH;
  updateSlopes();
}

inline void
RayContext::setOriginAndDirection(const fdk::Vec3f& _origin,
                                  const fdk::Vec3f& _dir)
{
  origin = _origin;
  direction = _dir;
  updateSlopes();
}

inline void
RayContext::invert()
{
  direction.negate();
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

//------------------------------------------------------------

inline RayContext&
RayContextBundle::add(const RayContext& rtx)
{
  _list->push_back(rtx); ++_size;
  return _list->back();
}

inline RayContext&
RayContextBundle::add(const RayContext&& rtx)
{
  _list->emplace_back(std::move(rtx)); ++_size;
  return _list->back();
}

} // namespace ndk

#endif
