//------------------------------------------------------------------------------
// DDImage/SlrShadingContext.h
//
// Copyright (c) 2021 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef SLR_SHADINGCONTEXT_H
#define SLR_SHADINGCONTEXT_H

#include "DDImage/SlrPrimInfo.h"
#include "DDImage/SlrLightInfo.h"

#include "fdk/math/types.h" // for Mat4dList
#include "fdk/math/Ray.h"

#include "DDImage/ChannelSet.h"
#include "DDImage/Channel3D.h"
#include "DDImage/Box3.h"

#include <cstring>

namespace DD {
  namespace Image {
    class Iop;
    class Pixel;
    class Sampler;
    class TextureFilter;

    // TODO: what about this? Keep it?
    using PixelBox = DD::Image::Box;
  }
}

namespace slr
{
  class SlrRenderContext;
  class SlrThreadContext;
  class SlrScene;
  class SlrVertex;
  class SlrTriangle;
  class SlrSurfaceShader;
  class SlrGeometryShader;
  class SlrVolumeShader;

  // Forward-declare these for slr renderer plugins, they remain abstract in DDImage:
  class SlrRayTestCache;

  // Size of float array in SlrVertex:
  static constexpr const int32_t SlrVertexChans = 28;

  //------------------------------------------------------------------
  /* Provide semi-compatibility with MatrixArray enums.

    MatrixArray map:
     Local space  = Untransformed
     World_space  = Parent object transform applied
     Eye space    = Inverse camera transform applied (camera becomes origin 0,0,0)
     Clip space   = Normalized Device Coordinates(NDC) - camera projection applied
     Screen space = Homogeneous-space scaled to pixel coordinates

    We're not trying to replicate the whole thing since a 5*5*16*double MatrixArray
    is 3200 bytes, and only a subset of the matrices are actually used in practice.
    We'll implement 10 out of the 25:
      LOCAL_TO_WORLD    (OBJECT_MATRIX)
      LOCAL_TO_EYE
      LOCAL_TO_SCREEN
      WORLD_TO_LOCAL    ** inverse LOCAL_TO_WORLD **
      WORLD_TO_EYE      (CAMERA_MATRIX)
      WORLD_TO_SCREEN
      EYE_TO_LOCAL      ** inverse LOCAL_TO_EYE **
      EYE_TO_WORLD      ** inverse CAMERA_MATRIX (which is itself an inverse) **
      EYE_TO_CLIP      (PROJECTION_MATRIX)
      CLIP_TO_SCREEN   (FORMAT_MATRIX)
  */
  enum SlrMatrixDefaults
  {
    OBJECT_MAT4          = 0, // object local-space to world-space
    CAMERA_MAT4          = 1, // world to eye-space - CameraOp::invWorldTransform()
    PROJECTION_MAT4      = 2, // lens linear-projection to ndc-space
    FORMAT_MAT4          = 3, // ndc scale to pixel-space
    //
    LOCAL_TO_WORLD_MAT4  = OBJECT_MAT4,
    LOCAL_TO_EYE_MAT4    = 4,
    LOCAL_TO_SCREEN_MAT4 = 5,
    //
    WORLD_TO_LOCAL_MAT4  = 6, // inverse
    WORLD_TO_EYE_MAT4    = CAMERA_MAT4, // ie CameraOp::invWorldTransform()
    WORLD_TO_SCREEN_MAT4 = 7,
    //
    EYE_TO_LOCAL_MAT4    = 8, // inverse
    EYE_TO_WORLD_MAT4    = 9, // inverse CAMERA_MAT4 (ie CameraOp::worldTransform())
    EYE_TO_NDC_MAT4      = PROJECTION_MAT4,
    //
    NDC_TO_SCREEN_MAT4   = FORMAT_MAT4,
    //
    MAT4_ARRAY_SIZE      = 10
  };


  //! Sets all the entries to identity.
  DDImage_API void resetMatrixArray(fdk::Mat4dList& matrix_array);

  //! Sets the OBJECT_MAT4 entry. Also resizes array to MAT4_ARRAY_SIZE.
  DDImage_API void setObjectMatrix(const fdk::Mat4d& m, fdk::Mat4dList& matrix_array);

  //! Sets the CAMERA_MAT4 entry. Also resizes array to MAT4_ARRAY_SIZE.
  DDImage_API void setCameraMatrix(const fdk::Mat4d& m, fdk::Mat4dList& matrix_array);

  //! Sets the PROJECTION_MAT4 entry. Also resizes array to MAT4_ARRAY_SIZE.
  DDImage_API void setProjectionMatrix(const fdk::Mat4d& m, fdk::Mat4dList& matrix_array);

  //! Sets the FORMAT_MAT4 entry. Also resizes array to MAT4_ARRAY_SIZE.
  DDImage_API void setFormatMatrix(const fdk::Mat4d& m, fdk::Mat4dList& matrix_array);

  /*! Build the forward and inverse combo matrices. Note that not all the
      ones defined in MatrixArray are implemented, just the ones currently
      used by the Slr shading system and shaders.

      Usually you only want to call this once after setting all the various
      base matrices.
  */
  DDImage_API void updateMatrixArray(fdk::Mat4dList& matrix_array);

  //----------------------------------------------------------------------


  /*! Fixed-size array of floating point numbers that are interpolated across a render
      primitive. This currently matches legacy ScanlineRender but can be expanded.

      An SlrShadingContext contains 3 of these: the current value, and the derivatives
      horizontally and vertically.
  */
  class DDImage_API SlrVertex
  {
  public:
    // Order matches the Channels defined in Channel3D.h:
    float       black;  //!<  0 Chan_Black - unused padding
    fdk::Vec4f  Cf;     //!<  1 Chan_Red..Chan_Alpha
    fdk::Vec4f  P;      //!<  5 Chan_Px..Chan_Pw
    fdk::Vec4f  UV;     //!<  9 Chan_UVx..Chan_UVw
    fdk::Vec3f  N;      //!< 13 Chan_Nx..Chan_Nz
    fdk::Vec3f  PL;     //!< 16 Chan_PLx..Chan_PLz
    fdk::Vec3f  PW;     //!< 19 Chan_PWx..Chan_PWz
    fdk::Vec3f  MB;     //!< 22 Chan_MBx..Chan_MBz
    fdk::Vec3f  VEL;    //!< 25 Chan_VELx..Chan_VELz

    //----------------------------------------------------------

    const float* array() const { return &black; }
          float* array()       { return &black; }

    const float& operator [] (int32_t i) const { return array()[i]; }
    float&       operator [] (int32_t i)       { return array()[i]; }

    //----------------------------------------------------------

    float x() const { return P.x; }
    float y() const { return P.y; }
    float z() const { return P.z; }
    float w() const { return P.w; }

    const fdk::Vec4f& rgba() const { return Cf; }
          fdk::Vec4f& rgba()       { return Cf; }
    const fdk::Vec3f& rgb() const { return reinterpret_cast<const fdk::Vec3f&>(Cf); }
          fdk::Vec3f& rgb()       { return reinterpret_cast<      fdk::Vec3f&>(Cf); }

    float r() const { return Cf.x; }
    float g() const { return Cf.y; }
    float b() const { return Cf.z; }
    float a() const { return Cf.w; }
    float opacity() const { return Cf.w; }

    //----------------------------------------------------------

    //! Set all attribs to 0
    void setToZero() { std::memset(array(), 0, sizeof(SlrVertex)); }

    //! Destructive linear interpolate between this and SlrVertex b by t.
    void lerp(const SlrVertex& b, float t);

    //! Sets all attribs to b+delta*t
    void offset(const SlrVertex& b,
                const SlrVertex& delta, float t);

    //! Scale all attribs.
    void scale(float s);

    //! Sets all attribs to b+dx*x+dy*y
    void offset(const SlrVertex& b,
                const SlrVertex& dx, float x,
                const SlrVertex& dy, float y);

    //! Sets all attribs to the midpoint between a and b ((a+b)/2).
    void midpoint(const SlrVertex& a,
                  const SlrVertex& b);


   friend std::ostream& operator << (std::ostream&, const SlrVertex&);

  };

  using SlrVertexArray = std::vector<SlrVertex>;


  //-----------------------------------------------------------------------------


  /*! This class is intended to be passed up a SlrShader tree.
      It's initialized with basic vertex info like the point location,
      a UV coordinate and a normal.  A shader can then modify the
      parameters producing effects like point displacement, UV
      projection, and normal remapping.
  */
  class DDImage_API SlrShadingContext
  {
  public:
    //----------------------------------------------------------

    SlrVertex  v;                   //!< Current values for shader - W-normalized, volatile!
    double     distance;            //!< Distance from last intersection/camera

    //----------------------------------------------------------

    SlrVertex  vP;                  //!< Current values for shader - in homogeneous-space!
    SlrVertex  vdX;                 //!< Change if X of render pixel is increased by 1
    SlrVertex  vdY;                 //!< Change if Y of render pixel is increased by 1

    //----------------------------------------------------------
    int32_t    x, y;                //!< Current output screen coords
    float      sx, sy;              //!< Current output subpixel screen coords
    int32_t    si;                  //!< Current subsample index

    double     time;                //!< Absolute frame time (i.e. 101.0, 155.0, etc)
    double     shutter_offset;      //!< Shutter offset from time0 reference (i.e. -0.5, -0.35, 0.0, +0.5, etc)
    uint32_t   shutter_step;        //!< Global-context shutter-step index for the context's time

    //----------------------------------------------------------

    SlrRenderContext*        slrtx;               //!< For global rendering params, render Op, etc.
    SlrScene*                motion_scene0;       //!< Scene info for motion sample at start of shutter_step
    SlrScene*                motion_scene1;       //!< Scene info for motion sample at end of shutter_step
    SlrPrimInfo*             motion_priminfo0;    //!< Current object SlrPrimInfo for motion sample at start of shutter_step
    SlrPrimInfo*             motion_priminfo1;    //!< Current object SlrPrimInfo for motion sample at end of shutter_step

    const SlrShadingContext* previous_stx;        //!< Previous SlrShadingContext, normally the last surface intersected/shaded
    SlrThreadContext*        sttx;                //!< Per-thread 'scratch-pad' vars to speed up Pixel usage in shaders

    SlrTriangle*             rtriangle;           //!< Current render triangle being evaluated
    SlrSurfaceShader*        surface_shader;      //!< Current surface shader being evaluated
    SlrGeometryShader*       displacement_shader; //!< Current displacement shader being evaluated, if any
    SlrLightShader*          light_shader;        //!< Current light shader being evaluated, if any
    SlrVolumeShader*         atmosphere_shader;   //!< Current atmospheric volume shader being evaluated, if any

    int32_t                   sides_mode;         //!< Which sides to intersect against and shade (use methods to query)
    bool                      texture_sampling;   //!< Whether shaders should sample their textures
    DD::Image::TextureFilter* texture_filter;     //!< Global texture filter to use

    bool                      lighting_enabled;   //!< If true, even if no lights the surface shaders should do illum calcs
    fdk::Vec3f                ambient;            //!< Global ambient setting


  public:
    SlrShadingContext();

    //! Thread index from current SlrThreadContext.
    uint32_t threadIdx() const;

    //----------------------------------------------------------

    SlrScene* motionScene() const { return motion_scene0; }

    SlrScene* motionScene0() const { return motion_scene0; }
    SlrScene* motionScene1() const { return motion_scene1; }

    //----------------------------------------------------------
    // Transforms
    const fdk::Mat4d& matrix(int n) const { return motion_priminfo0->transforms[n]; }

    const fdk::Mat4d& matrix0(int n) const { return motion_priminfo0->transforms[n]; }
    const fdk::Mat4d& matrix1(int n) const { return motion_priminfo1->transforms[n]; }

    //----------------------------------------------------------
    // Ray intersection testing
    SlrRayTestCache*  rayTestCache() const { return _ray_test_cache; };
    void              setRayTestCache(SlrRayTestCache* r) { _ray_test_cache = r; }


    /*! Distance from point \a P to the nearest not occluded polygon edge in screen-space
        for the current primitive.
        All triangles generated from the tessellation of the original SlrPrimInfo
        are tested, so the method checks all polygon edges.
        Returns when the first edge distance is under minDist.
    */
    float getPolygonEdgeDistance(const fdk::Vec4f& P,
                                 float             minDist) const;

    //! Shade both sides of surface (front and back) ignoring surface normal to camera.
    bool  shadeBothSides() const;
    //! Shade only front side of surface.
    bool  shadeFrontOnly() const;
    //! Shade only front side of surface.
    bool  shadeBackOnly()  const;

    void enableBothSidesMode();
    void enableFrontSideMode();
    void enableBackSideMode();


    // Conveniences for testing output pixel coords in shaders
    bool isPixel(int32_t px, int32_t py) const { return (px == x && py == y); }
    bool isPixelX(int32_t px) const { return (px == x); }
    bool isPixelY(int32_t py) const { return (py == y); }

    //----------------------------------------------------------

    //! Current homogeneous W coordinate:
    float                w() const { return vP.P.w; }

    //! Current Z coordinate - W-normalized!
    float                Z() const { return v.P.z; }

    //----------------------------------------------------------
    // Point access convenience methods - W-normalized!:

    const fdk::Vec3f&   PW() const { return v.PW; }
          fdk::Vec3f&   PW()       { return v.PW; }
    const fdk::Vec3f&   PL() const { return v.PL; }
          fdk::Vec3f&   PL()       { return v.PL; }

    //----------------------------------------------------------
    // Standard attribute convenience methods - W-normalized!:

    const fdk::Vec3f&   UV() const { return reinterpret_cast<const fdk::Vec3f&>(v.UV); }
          fdk::Vec3f&   UV()       { return reinterpret_cast<      fdk::Vec3f&>(v.UV); }
    const fdk::Vec3f&    N() const { return v.N; }
          fdk::Vec3f&    N()       { return v.N; }

    const fdk::Vec4f&   Cf() const { return v.Cf; }
          fdk::Vec4f&   Cf()       { return v.Cf; }
    const fdk::Vec4f& rgba() const { return v.rgba(); }
          fdk::Vec4f& rgba()       { return v.rgba(); }
    const fdk::Vec3f&  rgb() const { return v.rgb(); }
          fdk::Vec3f&  rgb()       { return v.rgb(); }
    float                r() const { return v.r(); }
    float                g() const { return v.g(); }
    float                b() const { return v.b(); }
    float                a() const { return v.a(); }
    float          opacity() const { return v.opacity(); }


    //----------------------------------------------------------
    // Texture sampling calls:

    /*! Use the UV and the derivatives of UV to sample the incoming image.
        UV of 0,0 is mapped to the lower-left corner of the image's format,
        and 1,1 is mapped to the upper-right corner.

        For texture sampling it's much faster to provide a DD::Image::Sampler
        rather than a raw Iop so Tile caching is properly leveraged.
    */
    void sample(DD::Image::Sampler* sampler,
                DD::Image::Pixel&   out) const;

    //! Slower version using a raw Iop.
    void sample(DD::Image::Iop*   texture,
                DD::Image::Pixel& out) const;


  protected:
    SlrRayTestCache* _ray_test_cache;   //!< Triangle cache for intersection testing
  };



/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/
/*                   Inline Function Implementations                   */
/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/

inline std::ostream& operator << (std::ostream& o, const SlrVertex& b)
{
  o << "[ ";
  o << "Cf" << b.Cf;
  o << " P" << b.P;
  o << " UV" << b.UV;
  o << " N" << b.N;
  o << " PL" << b.PL;
  o << " PW" << b.PW;
  o << " MB" << b.MB;
  o << " VEL" << b.VEL;
  o << " ]";
  return o;
}

inline void
SlrVertex::lerp(const SlrVertex& b, float t)
{
  if (t < std::numeric_limits<float>::epsilon()) {
    ; // do nothing
  }
  else if (t < 1.0f) {
    const float it = (1.0f - t);
    for (int32_t i = 1; i < SlrVertexChans; ++i) {
      array()[i] = array()[i] * it + b[i] * t;
    }
  }
  else {
    *this = b; // all b
  }
}

inline void
SlrVertex::scale(float s)
{
  if (s != 1.0f) {
    for (int32_t i = 1; i < SlrVertexChans; ++i) {
      array()[i] *= s;
    }
  }
}

inline void
SlrVertex::offset(const SlrVertex& b,
                  const SlrVertex& delta, float t)
{
  for (int32_t i = 1; i < SlrVertexChans; ++i) {
    array()[i] = b[i] + delta[i] * t;
  }
}

inline void
SlrVertex::offset(const SlrVertex& b,
                  const SlrVertex& dx, float x,
                  const SlrVertex& dy, float y)
{
  for (int32_t i = 1; i < SlrVertexChans; ++i) {
    array()[i] = b[i] + dx[i] * x + dy[i] * y;
  }
}

inline void
SlrVertex::midpoint(const SlrVertex& a,
                    const SlrVertex& b)
{
  for (int32_t i = 1; i < SlrVertexChans; ++i) {
    array()[i] = (a[i] + b[i]) * 0.5f;
  }
}


} // namespace slr

#endif
