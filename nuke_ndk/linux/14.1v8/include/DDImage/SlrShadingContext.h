//------------------------------------------------------------------------------
// DDImage/SlrShadingContext.h
//
// Copyright (c) 2021 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef SLR_SHADINGCONTEXT_H
#define SLR_SHADINGCONTEXT_H

#include "DDImage/SlrLightInfo.h"

#include "fdk/math/types.h" // for Mat4dList
#include "fdk/math/Ray.h"
#include "fdk/base/Hash.h"

#include "ndk/geo/render/RayContext.h"

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
    using RasterBox = DD::Image::Box;
  }
}

namespace slr
{
  class SlrThreadContext;
  class SlrPrimInfo;
  class SlrSurfaceShader;
  class SlrGeometryShader;
  class SlrVolumeShader;

  // Forward-declare these for slr renderer plugins, they remain abstract in DDImage:
  class SlrTriangle;
  class SlrObjectIntersection;

  //------------------------------

  // Specialized types for returning a value and its dx/dy derivatives in a single object.
  // More lightweight than packing them inside a Pixel or Mat4d.
  // Named for the similar OSL class but not using templates so these can contain fdk Vec classes.
  // However they will not be as efficient as the vectorizable data layout in OSL.

  struct Dual2f     { float      val, dx, dy; };
  struct Dual2d     { double     val, dx, dy; };
  struct Dual2Vec2f { fdk::Vec2f val, dx, dy; };
  struct Dual2Vec3f { fdk::Vec3f val, dx, dy; };
  struct Dual2Vec4f { fdk::Vec4f val, dx, dy; };

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


  //-----------------------------------------------------------------------------


  struct DDImage_API SlrShadingParams
  {
    fdk::Vec3f  Ps;           //!< Displaced shading point in common shading space where lighting is queried
    fdk::Vec3f  P;            //!< Geometric point (intersection) in common shading space
    fdk::Vec3f  dPdx;         //!< P differential for dxDir ray direction
    fdk::Vec3f  dPdy;         //!< P differential for dyDir ray direction
    fdk::Vec3f  dPdu;         //!< Parametric-u partial derivative tangent to the surface at P, not normalized
    fdk::Vec3f  dPdv;         //!< Parametric-v partial derivative tangent to the surface at P, not normalized
    fdk::Vec3f  dPdw;         //!< Parametric-w partial derivative tangent to the surface at P, not normalized
    fdk::Vec3f  dPdtime;      //!< P velocity in the current shutter step

    fdk::Vec3f  triUvw;       //!< Barycentric triangle uvw coordinates at P intersection
    uint8_t     edgeMask;     //!< 
    uint8_t     pad1;         //!< 
    uint8_t     pad2;         //!< 
    uint8_t     pad3;         //!< 

    fdk::Vec2f  uv;           //!< Surface 2D parametric coordinates of P
    fdk::Vec2f  dUVdx;        //!< Parametric-uv differential for dxDir ray direction
    fdk::Vec2f  dUVdy;        //!< Parametric-uv differential for dyDir ray direction
    float       uvw;          //!< Surface Z parametric coordinate of P (for volumes)
    float       dUVWdx;       //!< Parametric-w differential for dxDir ray direction (for volumes)
    float       dUVWdy;       //!< Parametric-w differential for dyDir ray direction (for volumes)

    fdk::Vec3f  Ns;           //!< Shading normal, normalized
    fdk::Vec3f  Ng;           //!< Geometric surface normal, normalized
    fdk::Vec3f  Ts;           //!< Shading tangent, normalized

    fdk::Vec2f  st;           //!< Surface texture st(uv) coordinate (w divided out)
    fdk::Vec2f  dStdx;        //!< Texture coord st differential for dxDir ray direction
    fdk::Vec2f  dStdy;        //!< Texture coord st differential for dyDir ray direction
    fdk::Vec2f  dStdu;        //!< Texture coord st partial derivative in parametric-u direction, not normalized
    fdk::Vec2f  dStdv;        //!< Texture coord st partial derivative in parametric-v direction, not normalized

    fdk::Vec4f  Cf;           //!< Interpolated vertex color + opacity

    //----------------------------------------------------------

    const float* array() const { return reinterpret_cast<const float*>(this); }
          float* array()       { return reinterpret_cast<float*>(this); }

    const float& operator [] (int32_t i) const { return array()[i]; }
    float&       operator [] (int32_t i)       { return array()[i]; }

    //----------------------------------------------------------

    float x() const { return P.x; }
    float y() const { return P.y; }
    float z() const { return P.z; }

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

    //! Set all params to 0
    void setToZero() { std::memset(this, 0, sizeof(SlrShadingParams)); }

    friend DDImage_API std::ostream& operator << (std::ostream&, const SlrShadingParams&);

  };
  using SlrShadingParamsList = std::vector<SlrShadingParams>;


  //-----------------------------------------------------------------------------


  /*! This class is intended to be passed up a SlrShader tree.
  */
  class DDImage_API SlrShadingContext
  {
  public:
    ndk::RayContext     srcRtx;              //!< Ray that created this context
    int32_t             depth;               //!< Current ray depth where 0 indicates a camera ray
    float               distance;            //!< Distance of this context's P from srcRtx origin
    uint64_t            primId;              //!< Unique ID of the current intersection renderable/subpart/subtri.
    int32_t             renderableIndex;     //!< PrimInfo index of current intersected object in SlrRenderContext
    //
    SlrShadingParams    params;              //!< Sampled surface parameters
    //
    SlrSurfaceShader*   surfaceShader;       //!< Current surface shader being evaluated
    SlrGeometryShader*  displacementShader;  //!< Current displacement shader being evaluated, if any
    SlrVolumeShader*    atmosphereShader;    //!< Current atmospheric volume shader being evaluated, if any


  public:
    //! Default ctor leaves junk in most vars, sets pointers to null.
    SlrShadingContext();

    //! Copies currentStx and params and updates the srcRtx.
    void  set(const SlrShadingContext& currentStx,
              const ndk::RayContext&   Rtx);

    //! Copies currentStx and params and updates the srcRtx.
    void  set(const SlrShadingContext&         currentStx,
              const fdk::Vec3f&                Rdir,
              double                           tMin,
              double                           tMax,
              const ndk::RayContext::TypeMask& Rtype,
              uint32_t                         sides_mode);

    //! Absolute time of the context
    fdk::TimeValue     time() const { return srcRtx.time; }

    //! SlrThreadContext which 'owns' this shading context.
    SlrThreadContext*  sttx() const { return _sttx; }

    //! Thread index from current SlrThreadContext.
    uint32_t           threadIdx() const;

    //! Previous SlrShadingContext, normally the last surface intersected/shaded.
    const SlrShadingContext* previousStx() const { return _previousStx; }

    //----------------------------------------------------------
    // Global rendering info:

    //! Blending Z order is back-to-front (true) or front-to-back (false)
    bool  blendBackToFront() const;
    //! Shaders should output color layers + alphas.
    bool  enableColorChannels() const;
    //! Shaders should output any data channels (Z, etc)
    bool  enableDataChannels() const;

    //! Should textures sample? This may be disabled for camera rays when shading
    //! a pre-rendered scene with pixel containing the surface albedo.
    bool  textureSamplingEnabled() const { return _textureSampling; }
    void  setTextureSampling(bool enabled) { _textureSampling = enabled; }
    void  enableTextureSampling() { setTextureSampling(true); }
    void  disableTextureSampling() { setTextureSampling(false); }

    //! Texture filter to use.
    DD::Image::TextureFilter* textureFilter() const { return _textureFilter; }

    //! If true, even if no lights the surface shaders should do illum calcs.
    bool  lightingEnabled() const;

    //! If true do direct-illumination calculations.
    bool  directLightingEnabled() const;

    //! If true handle indirect-illumation calculations.
    bool  indirectLightingEnabled() const;

    //! If true do shadow occlusion for direct-illumination.
    bool  shadowingEnabled() const;

    //! If enabled getViewVector() uses camHeroP() instead of camP().
    bool  useHeroCamForSpec() const;

    //----------------------------------------------------------

    //! Render triangle currently being shaded.
    //! This can be tested against the rprim() in previousStx() to prevent self-intersection.
    const SlrTriangle* rprim() const { return _rPrim; }

    //! Local-to-world matrix for current primitive. If null assume xform is identity.
    const fdk::Mat4d* l2w() const { return _l2w; }

    //! World-to-local matrix for current primitive. If null assume xform is identity.
    const fdk::Mat4d* w2l() const { return _w2l; }

    //! Render camera world-to-eye xform (world-space coords to camera local-space)
    const fdk::Mat4d& camWorld2Eye() const;

    //! Render camera eye-to-world xform (camera local-space coords to world-space)
    const fdk::Mat4d& camEye2World() const;

    //! Render camera origin (camera rays originate here).
    //! Same as camEye2World().getTranslation(), but cached for speed.
    const fdk::Vec3d& camP() const;

    //! Hero-view camera origin in world space - for specular copying
    const fdk::Vec3d& camHeroP() const;

    //! List of active lights. If \a lightingEnabled is false an empty list is returned.
    const SlrLightInfoList&  lights() const;

    //----------------------------------------------------------
    // Ray intersection testing

    //! Returns a 'fake' stereo view vector built from the camHeroP, or the negated
    //! ray direction depending on context's stereo mode.
    //! By using the camHeroP camera-ray specular calculations are identical for all
    //! stereo cameras, 'flattening' the specular onto the surface.
    fdk::Vec3d  getViewVector() const;

    //! Shade both sides of surface (front and back) ignoring surface normal to camera.
    bool  shadeBothSides() const;
    //! Shade only front side of surface.
    bool  shadeFrontOnly() const;
    //! Shade only front side of surface.
    bool  shadeBackOnly()  const;

    void  enableBothSidesMode();
    void  enableFrontSideMode();
    void  enableBackSideMode();

    //! Adds light globally to the diffuse (albedo) surface response of surface shaders
    //! which respect this shading global.
    const fdk::Vec3f&  ambientLightingBias() const;

    //----------------------------------------------------------

    //! Returns true if anything is occluding the ray, including transparent objects.
    //! Does not call shaders or return information about the hit object.
    bool  intersect(const ndk::RayContext& Rtx) const;

    //! Intersect the scene with the RayContext \a Rtx, returning true if there was a hit
    //! within \a Rtx's distance range, and filling in \a hitStx with the information
    //! about the hit surface.
    bool  getNearestIntersection(const ndk::RayContext& Rtx,
                                 SlrShadingContext&     hitStx) const;

    /*! Distance from \a P to the nearest not-occluded polygon edge in screen-space
        for the current primitive.
        All triangles generated from the tessellation of the original SlrPrimInfo
        are tested, so the method checks all polygon edges.
        Returns when the first edge distance is under \a minDist.
    */
    float getPolygonEdgeDistance(float minDist) const;

    //----------------------------------------------------------

    //! Current output screen coords
    const fdk::Vec2i&  pxy() const;
    //! Current output subpixel screen coords
    const fdk::Vec2f&  sxy() const;
    //! Current subsample index
    int32_t            si() const;

    // Conveniences for testing output pixel coords in shaders
    bool isDebugPixel() const;
    bool isPixel(int32_t px, int32_t py) const;
    bool isPixelX(int32_t px) const;
    bool isPixelY(int32_t py) const;

    //----------------------------------------------------------

    //! Displaced shading point in common shading space where lighting is queried.
    const fdk::Vec3f&     Ps() const { return params.Ps; }
    //! Geometric point (intersection) in common shading space.
    const fdk::Vec3f&      P() const { return params.P; }

    //! P differential for dyDir ray direction. ('dx' is the ray cone x-axis, not screen-space 'x')
    const fdk::Vec3f&   dPdx() const { return params.dPdx; }
    //! P differential for dyDir ray direction. ('dy' is the ray cone y-axis, not screen-space 'y')
    const fdk::Vec3f&   dPdy() const { return params.dPdy; }

    //! P surface u-space partial derivative, not normalized.
    const fdk::Vec3f&   dPdu() const { return params.dPdu; }
    //! P surface v-space partial derivative, not normalized.
    const fdk::Vec3f&   dPdv() const { return params.dPdv; }

    //! Barycentric triangle uvw coordinates at srcRtx intersection.
    //! For a triangle with corners (p0 p1 p2) w=1 is along edge p0->p1, u=1 is along edge p1->p2,
    //! and v=1 is along edge p0->p2.
    //! If v or w are outside 0..1 then P is outside the bounds of the triangle and w in invalid. This
    //! happens when uvw is calculated for ray offsets dxDir & dyDir. They will always intersect the
    //! triangle plane but can often be outside the triangle's perimeter.
    const fdk::Vec3f& triUvw() const { return params.triUvw; }

    //! Surface 2D parametric coordinates of P
    const fdk::Vec2f      uv() const { return params.uv; }
    //! Parametric-uv differential for dxDir ray direction
    const fdk::Vec2f   dUVdx() const { return params.dUVdx; }
    //! Parametric-uv differential for dyDir ray direction
    const fdk::Vec2f   dUVdy() const { return params.dUVdy; }

    //! Surface Z parametric coordinate of P (for volumes)
    float                uvw() const { return params.uvw; }
    //! Parametric-w differential for dxDir ray direction
    float             dUVWdx() const { return params.dUVWdx; }
    //! Parametric-w differential for dyDir ray direction
    float             dUVWdy() const { return params.dUVWdy; }

    //! Surface texture st(uv) coordinate. st is assumed to have already been divided by any w.
    const fdk::Vec2f&     st() const { return params.st; }

    //! Texture coord st differential for dxDir ray direction. ('dx' is the ray cone x-axis, not screen-space 'x')
    const fdk::Vec2f&  dStdx() const { return params.dStdx; }
    //! Texture coord st differential for dyDir ray direction. ('dy' is the ray cone y-axis, not screen-space 'y')
    const fdk::Vec2f&  dStdy() const { return params.dStdy; }

    //! St surface u-space partial derivative, not normalized.
    const fdk::Vec2f&  dStdu() const { return params.dStdu; }
    //! St surface v-space partial derivative, not normalized.
    const fdk::Vec2f&  dStdv() const { return params.dStdv; }

    //! Shading normal, normalized
    const fdk::Vec3f&      N() const { return params.Ns; }
    //! Geometric surface normal, normalized
    const fdk::Vec3f&     Ng() const { return params.Ng; }
    //! Shading tangent, normalized
    const fdk::Vec3f&      T() const { return params.Ts; }

    //! Interpolated vertex color + opacity.
    const fdk::Vec4f&     Cf() const { return params.Cf; }
    const fdk::Vec4f&   rgba() const { return params.rgba(); }
    const fdk::Vec3f&    rgb() const { return params.rgb(); }
    float                  r() const { return params.r(); }
    float                  g() const { return params.g(); }
    float                  b() const { return params.b(); }
    float                  a() const { return params.a(); }
    float            opacity() const { return params.opacity(); }


    //----------------------------------------------------------
    // Texture sampling calls:

    /*! Use st() and the derivatives of st() to sample the incoming image.
        The st coord 0,0 is mapped to the lower-left corner of the image's format,
        and 1,1 is mapped to the upper-right corner.

        For texture sampling it's much faster to provide a DD::Image::Sampler
        rather than a raw Iop so Tile caching is properly leveraged.
    */
    void sample(DD::Image::Sampler* sampler,
                DD::Image::Pixel&   out) const;
    //! Same as sample(Sampler*) but with overrides for st & derivatives currently in context.
    void sample(DD::Image::Sampler* sampler,
                const Dual2Vec2f&   v,
                DD::Image::Pixel&   out) const;

    //! Slower version using a raw Iop.
    void sample(DD::Image::Iop*   texture,
                DD::Image::Pixel& out) const;
    //! Same as sample(Iop*) but with overrides for st & derivatives currently in context.
    void sample(DD::Image::Iop*   texture,
                const Dual2Vec2f& v,
                DD::Image::Pixel& out) const;


    //------------------------------------------------------------------------
    // Thread-safe scratch Pixels to be used during ray-tracing & shading.
    //
    // Warning - since these are shared variables they should only be
    // used at the end of a shader chain, for example when sampling
    // textures or light values. If passed up a shader graph there's
    // a good chance of conflict with another shader attempting to use
    // the same var.

    //! For sampling texture map Iops - defaults to RGBA
    DD::Image::Pixel&  textureColor() const;
    //! For Connection sampling calls - defaults to RGBA
    DD::Image::Pixel&  bindingColor() const;
    //! For LightShader evaluation - defaults to RGB
    DD::Image::Pixel&  illumColor() const;
    //! For LightShader shadow transmission evaluation - defaults to RGB (no depth!)
    DD::Image::Pixel&  shadowColor() const;
    //! For VolumeShader color evaluation - defaults to RGB
    DD::Image::Pixel&  volumeColor() const;


  protected:
    friend class SlrThreadContext;
    friend class RenderEngine;
    friend class RenderSpan;
    friend class IntersectedSurfaces;
    SlrThreadContext*         _sttx;
    const SlrShadingContext*  _previousStx;     //!< Previous SlrShadingContext, normally the last surface intersected/shaded
    //
    int32_t                   _sidesMode;       //!< Which sides to intersect against and shade (use methods to query)
    bool                      _textureSampling; //!< Whether shaders should sample their textures
    DD::Image::TextureFilter* _textureFilter;   //!< Global texture filter to use
    //
    const SlrObjectIntersection* _If;           //!< Intersection info
    //
    SlrTriangle*              _rPrim;           //!< Current render triangle being evaluated
    fdk::Mat4d*               _l2w;             //!< Local-to-world object matrix for current primitive
    fdk::Mat4d*               _w2l;             //!< World-to-local object matrix for current primitive


  public://protected:
    //----------------------------------------------------------------
    // Deprecated scanline shading data - not used when ray tracing!
    float  _vP[28]; //!< Current values for shader - in homogeneous-space! (deprecated)
    float _vdX[28]; //!< Change if X of render pixel is increased by 1 (deprecated)
    float _vdY[28]; //!< Change if Y of render pixel is increased by 1 (deprecated)
  };

  using SlrShadingContextList = std::vector<SlrShadingContext>;


} // namespace slr

#endif
