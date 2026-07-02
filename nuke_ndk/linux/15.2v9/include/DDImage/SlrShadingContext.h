//------------------------------------------------------------------------------
// DDImage/SlrShadingContext.h
//
// Copyright (c) 2021 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef SLR_SHADINGCONTEXT_H
#define SLR_SHADINGCONTEXT_H

#include "DDImage/SlrPrimInfo.h" // for SlrLightInfoList

#include "ndk/geo/render/RayContext.h"
#include "ndk/base/Channel.h"

#include "DDImage/ChannelSet.h"
#include "DDImage/Pixel.h"
#include "DDImage/Thread.h" // for ThreadId

namespace DD {
  namespace Image {
    class Iop;
    class Pixel;
    class Sampler;
    class TextureFilter;
  }
}

namespace ndk {
  class SamplingRngSet;
}

namespace slr
{
  class SlrThreadContext;
  class SlrIntegratorContext;
  class SlrSurfaceShader;
  class SlrGeometryShader;
  class SlrVolumeShader;
  class SlrRenderObject;

  //-----------------------------------------------------------------------------

  //! Convenience function to remove any leading 'primvars:' notation from string.
  DDImage_API
  std::string  StripPrimvarNotation(const char* name);
  DDImage_API
  std::string  StripPrimvarNotation(const std::string& name);
  DDImage_API
  std::string  StripPrimvarNotation(const usg::Token& name);

  //-----------------------------------------------------------------------------

  //! Used to communicate the name and channel count of layers that shaders are requesting as
  //! primvars or writable shading layers.
  //!
  //! Note when requesting primvars the name should -not- contain the leading 'primars:' of the
  //! source attribute. For example if requesting the displayColor primvar then name should be
  //! 'displayColor' and -not- 'primvars:displayColor'.
  //! if it's more convenient to use the attribute names with the leading 'primvars:' then use
  //! the constructors which will strip off any leading 'primvars:'.
  //!
  //! The channel count -must- be within 1..4, channel counts > 4 or < 1 are ignored.
  //! If 'interp' is true then additional '<name>_Dx' and '<name>_Dy' shading layers are requested
  //! with the same channel count. This is ignored when primvar sample layers are being requested
  //! as the primvar interpolation type is used.
  struct ShadingLayerReq
  {
    usg::Token name;        //!< Requested layer name (if primvar this is -without- the leading 'primvars:')
    int32_t    nChans;      //!< Number of channels, between 1..4. Requests with > 4 chans will be ignored.
    bool       interp;      //!< Does shading layer interpolate across primitives? Ignored for primvar layers.

    //! Request is considered invalid if number of channels requested is > 4.
    bool empty() const { return (name.empty() || nChans < 1 || nChans > 4); }

    explicit operator bool() const { return !empty(); }

    bool operator == (const ShadingLayerReq& b) const { return (name == b.name && nChans == b.nChans); }
    bool operator != (const ShadingLayerReq& b) const { return (name != b.name || nChans != b.nChans); }
    bool operator <  (const ShadingLayerReq& b) const { return (name < b.name); }

    ShadingLayerReq() : nChans(0) {}

    //! Removes any leading 'primvars:' before setting name.
    ShadingLayerReq(const usg::Token& _name, int32_t n, bool _interp = false)
      : name(StripPrimvarNotation(_name.text())), nChans(n), interp(_interp)
    {}

    //! Removes any leading 'primvars:' before setting name.
    ShadingLayerReq(const char* _name, int32_t n, bool _interp = false)
      : name(StripPrimvarNotation(_name)), nChans(n), interp(_interp)
    {}
  };

  std::ostream& operator << (std::ostream&, const ShadingLayerReq&);

  using ShadingLayerReqList = std::vector<ShadingLayerReq>;
  using ShadingLayerReqSet  = std::set<ShadingLayerReq>;

  //-----------------------------------------------------------------------------

  // Built-in parameter channel definitions:

  static constexpr ndk::Channel  Chan_R  = 1; // Same as ndk::Chan_R
  static constexpr ndk::Channel  Chan_G  = 2; // Same as ndk::Chan_G
  static constexpr ndk::Channel  Chan_B  = 3; // Same as ndk::Chan_B
  static constexpr ndk::Channel  Chan_A  = 4; // Same as ndk::Chan_A
  //
  static constexpr ndk::Channel  Chan_Ps      =  5; // Vec3f
  static constexpr ndk::Channel  Chan_P       =  8; // Vec3f
  static constexpr ndk::Channel  Chan_dPdx    = 11; // Vec3f
  static constexpr ndk::Channel  Chan_dPdy    = 14; // Vec3f
  static constexpr ndk::Channel  Chan_dPdu    = 17; // Vec3f
  static constexpr ndk::Channel  Chan_dPdv    = 20; // Vec3f
  static constexpr ndk::Channel  Chan_dPdw    = 23; // Vec3f
  static constexpr ndk::Channel  Chan_dPdtime = 26; // Vec3f
  //
  static constexpr ndk::Channel  Chan_TriUvw = 29; // Vec3f
  //
  static constexpr ndk::Channel  Chan_Ns     = 32; // Vec3f
//static constexpr ndk::Channel  Chan_dNsdy  = 35; // Vec3f
//static constexpr ndk::Channel  Chan_dNsdy  = 38; // Vec3f
  static constexpr ndk::Channel  Chan_Ng     = 35; // Vec3f
  static constexpr ndk::Channel  Chan_Ts     = 38; // Vec3f
  //
  static constexpr ndk::Channel  Chan_UV     = 41; // Vec2f surface parametric-uv
  static constexpr ndk::Channel  Chan_dUVdx  = 43; // Vec2f
  static constexpr ndk::Channel  Chan_dUVdy  = 45; // Vec2f
  //
  static constexpr ndk::Channel  Chan_UVW    = 47; // float parametric-w (for volumes)
  static constexpr ndk::Channel  Chan_dUVWdx = 48; // float
  static constexpr ndk::Channel  Chan_dUVWdy = 49; // float
  //
  static constexpr ndk::Channel  Chan_St     = 50; // Vec2f - texture coord
  static constexpr ndk::Channel  Chan_dStdx  = 52; // Vec2f
  static constexpr ndk::Channel  Chan_dStdy  = 54; // Vec2f
  static constexpr ndk::Channel  Chan_dStdu  = 56; // Vec2f
  static constexpr ndk::Channel  Chan_dStdv  = 58; // Vec2f

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

  //! Channels pointing at the data indices for a Dual2 package.
  struct Dual2Chans { ndk::Channel val, dx, dy; };

  // Built-in parameter Dual2Chans definitions:

  static constexpr Dual2Chans  Dual2Chans_P   = { 8/*Chan_P*/  , 11/*Chan_dPdx*/  , 14/*Chan_dPdy*/  }; // Vec3f
  static constexpr Dual2Chans  Dual2Chans_UV  = {41/*Chan_UV*/ , 43/*Chan_dUVdx*/ , 45/*Chan_dUVdy*/ }; // Vec2f parametric-uv
  static constexpr Dual2Chans  Dual2Chans_UVw = {47/*Chan_UVW*/, 48/*Chan_dUVWdx*/, 49/*Chan_dUVWdy*/}; // float parametric-w (for volumes)
  static constexpr Dual2Chans  Dual2Chans_St  = {50/*Chan_St*/ , 52/*Chan_dStdx*/ , 54/*Chan_dStdy*/ }; // Vec2f texture coord

  //-----------------------------------------------------------------------------


  /*! This class is intended to be passed up a SlrShader tree.
  */
  class DDImage_API SlrShadingContext
  {
  public:
    ndk::RayContext     srcRtx;               //!< Ray that created this context
    //
    int32_t             rayDepth;             //!< Current ray depth where 0 indicates a camera, or primary ray
    float               distance;             //!< Distance of this context's P from srcRtx origin
    //
    SlrSurfaceShader*   surfaceShader;        //!< Current surface shader being evaluated
    SlrGeometryShader*  displacementShader;   //!< Current displacement shader being evaluated, if any
    SlrVolumeShader*    atmosphereShader;     //!< Current atmospheric volume shader being evaluated, if any


  public:
    /* Comparison operators test the distances: */
    bool operator <  (const SlrShadingContext& i) const { return (distance <  i.distance); }
    bool operator <= (const SlrShadingContext& i) const { return (distance <= i.distance); }
    bool operator >  (const SlrShadingContext& i) const { return (distance >  i.distance); }
    bool operator >= (const SlrShadingContext& i) const { return (distance >= i.distance); }
    bool operator == (const SlrShadingContext& i) const { return (::fabs(distance - i.distance) <  std::numeric_limits<float>::epsilon()); }
    bool operator != (const SlrShadingContext& i) const { return (::fabs(distance - i.distance) >= std::numeric_limits<float>::epsilon()); }
    bool operator <  (float v) const { return (distance <  v); }
    bool operator <= (float v) const { return (distance <= v); }
    bool operator >  (float v) const { return (distance >  v); }
    bool operator >= (float v) const { return (distance >= v); }
    bool operator == (float v) const { return (::fabs(distance - v) <  std::numeric_limits<float>::epsilon()); }
    bool operator != (float v) const { return (::fabs(distance - v) >= std::numeric_limits<float>::epsilon()); }


  public:
    //! Is shading context is operating in 'legacy' mode? ie using the old ScanlineRender1 architecture?
    //! In legacy mode certain features such as dynamic primvar retrieval are not supported, so a shader
    //! can test this before deciding to sample primar channels it expects to be filled in.
    //! This method will be deprecated at some point after the legacy system is removed. After the legacy
    //! system is gone it will return 'false' until the method's eventual removal.
    bool  isSlr1LegacyMode() const;

    //! Previous SlrShadingContext, normally the last surface intersected/shaded and one level
    //! up in ray depth.
    const SlrShadingContext*  previousStx() const { return _previousStx; }

    //! Create an editable shading context that's a copy of this one. Use the setAttrib() methods
    //! to change the values on the returned context. The setAttrib() methods will fail on a
    //! non-editable context.
    //! This is primarily used to modify the shading context when passing it to an input shader.
    //! A good example of this is modifying the UV attrib based on the current point location to
    //! perform high quality texture projections.
    SlrShadingContext*  createEditableShadingContextCopy(bool copyShadingLayers = true) const;

    //! Number of assigned shading channels.
    int32_t  nShadingChannels() const;

    //! Set all param channels in the Stx to 0. Does nothing if Stx is not writable.
    void     setShadingParamsToZero();

    // Set a writable shading layer value by Channel index. The Channel index references
    // the first element in the value and the elements are assumed to be packed in order in
    // the array.
    // Shading layer channels are arbitrary data channels shared between shaders through the
    // SlrShadingContext and are separate from the AOV channels the renderer outputs.
    // These methods do nothing if this SlrShadingContext is not writable.
    // Returns true if write is successful.
    // See the PrimAttrib accessor methods for reading primvars.

    bool    setIntShadingLayer(ndk::Channel chan, int32_t value);
    bool  setFloatShadingLayer(ndk::Channel chan, float   value);
    bool  setVec2fShadingLayer(ndk::Channel startChan, const fdk::Vec2f& value);
    bool  setVec3fShadingLayer(ndk::Channel startChan, const fdk::Vec3f& value);
    bool  setVec4fShadingLayer(ndk::Channel startChan, const fdk::Vec4f& value);

    bool      setDual2fShadingLayer(const Dual2Chans& chans, const Dual2f& value);
    bool      setDual2dShadingLayer(const Dual2Chans& chans, const Dual2d& value);
    bool  setDual2Vec2fShadingLayer(const Dual2Chans& chans, const Dual2Vec2f& value);
    bool  setDual2Vec3fShadingLayer(const Dual2Chans& chans, const Dual2Vec3f& value);
    bool  setDual2Vec4fShadingLayer(const Dual2Chans& chans, const Dual2Vec4f& value);


  public:
    //! Absolute time of the context
    fdk::TimeValue  time() const { return srcRtx.time; }

    //! Thread id from current SlrThreadContext.
    DD::Image::ThreadId  threadId() const;

    //! Integrator context that created this shading context.
    const SlrIntegratorContext&  Itx() const;

    //! Sampling random number generator owned by this thread.
    ndk::SamplingRngSet&  samplingRng() const;

    //! Ray id of \a srcRtx
    int32_t   rayId() const { return srcRtx.id; }

    //! Unique ID of the current intersection renderable/subpart/subtri.
    uint64_t  primId() const;

    //! PrimInfo index of current intersected object in SlrRenderContext.
    int32_t   renderableIndex() const;

    //! This context's shading data can be written to.
    bool      isWritable() const;

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
    bool  textureSamplingEnabled() const { return _doTextureSampling; }
    void  setTextureSampling(bool enabled) { _doTextureSampling = enabled; }
    void  enableTextureSampling() { setTextureSampling(true); }
    void  disableTextureSampling() { setTextureSampling(false); }

    //! Texture filter to use. If null the texture will be impulse sampled.
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

    //! Local-to-world matrix for current primitive
    const fdk::Mat4d& l2w() const;

    //! World-to-local matrix for current primitive
    const fdk::Mat4d& w2l() const;

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
    //! Using the camHeroP camera-ray makes specular calculations identical for all
    //! stereo cameras, visually 'flattening' the specular onto the surface.
    fdk::Vec3f  getViewVector() const;

    //! Shade both sides of surface (front and back) ignoring surface normal to camera.
    bool  shadeBothSides() const { return (_sidesMode == ndk::RayContext::SIDES_BOTH);  }
    //! Shade only front side of surface.
    bool  shadeFrontOnly() const { return (_sidesMode == ndk::RayContext::SIDES_FRONT); }
    //! Shade only front side of surface.
    bool  shadeBackOnly()  const { return (_sidesMode == ndk::RayContext::SIDES_BACK);  }

    //! Override sides mode. By default sides mode is copied from surface shader.
    void  setSidesMode(ndk::RayContext::SidesMode mode) { _sidesMode = mode; }

    //! Adds light globally to the diffuse (albedo) surface response of surface shaders
    //! which respect this shading global.
    const fdk::Vec3f&  ambientLightingBias() const;

    //----------------------------------------------------------

    //! Returns true if anything is occluding the ray, including transparent objects.
    //! Does not call shaders or return information about the hit object.
    bool  intersect(const ndk::RayContext& Rtx = ndk::RayContext()) const;

    //! Intersect the scene with the RayContext \a Rtx, returning true if there was a hit
    //! within \a Rtx's distance range, and filling in \a hitStx with the information
    //! about the hit surface.
    bool  getNearestIntersection(const ndk::RayContext& Rtx,
                                 SlrShadingContext&     hitStx) const;

    //! Start a new RayContext bundle making it the current bundle and returning a reference
    //! to it. Note that only one bundle may be active at a time since each bundle must be a
    //! contiguous section of RayContexts in the cache.
    ndk::RayContextBundle&  startNewRtxBundle() const;

    //! Access the current RayContextBundle for this thread which can then be added to. Note
    //! that only one bundle may be current at a time.
    ndk::RayContextBundle&  currentRtxBundle() const;


    /*! Distance from \a P to the nearest not-occluded polygon edge in screen-space
        for the current primitive.
        All triangles generated from the tessellation of the original SlrPrimInfo
        are tested, so the method checks all polygon edges.
        Returns when the first edge distance is under \a minDist.
    */
    float getPolygonEdgeDistance(float minDist) const;

    //----------------------------------------------------------

    //! Current output screen coords
    const fdk::Vec2i&  pixelXY() const;
    //! Current output subpixel coord inside pixelXY
    const fdk::Vec2f&  subpixelXY() const;
    //! Current subsample/camera-ray index
    int32_t            sampleIndex() const;
    int32_t            primaryRayIndex() const { return sampleIndex(); }
    bool               isDebugSample() const;

    // Conveniences for testing output pixel coords in shaders
    bool isDebugPixel() const;
    bool isPixel(int32_t px, int32_t py) const;
    bool isPixelX(int32_t px) const;
    bool isPixelY(int32_t py) const;

    //-------------------------------------------------------
    // Built-in intrinsic surface parameters:

    //! Displaced shading point in common shading space where lighting is queried.
    const fdk::Vec3f&     Ps() const;
    //! Geometric point (intersection) in common shading space.
    const fdk::Vec3f&      P() const;

    //! P differential for dyDir ray direction. ('dx' is the ray cone x-axis, not screen-space 'x')
    const fdk::Vec3f&   dPdx() const;
    //! P differential for dyDir ray direction. ('dy' is the ray cone y-axis, not screen-space 'y')
    const fdk::Vec3f&   dPdy() const;

    //! P surface u-space partial derivative, not normalized.
    const fdk::Vec3f&   dPdu() const;
    //! P surface v-space partial derivative, not normalized.
    const fdk::Vec3f&   dPdv() const;
    //! P surface z-space partial derivative, not normalized. (for volumes)
    const fdk::Vec3f&   dPdw() const;

    //! Barycentric triangle uvw coordinates at srcRtx intersection.
    //! For a triangle with corners (p0 p1 p2) w=1 is along edge p0->p1, u=1 is along edge p1->p2,
    //! and v=1 is along edge p0->p2.
    //! If v or w are outside 0..1 then P is outside the bounds of the triangle and w in invalid. This
    //! happens when uvw is calculated for ray offsets dxDir & dyDir. They will always intersect the
    //! triangle plane but can often be outside the triangle's perimeter.
    const fdk::Vec3f& triUvw() const;

    //! Shading normal, normalized
    const fdk::Vec3f&      N() const;
    //! Geometric surface normal, normalized
    const fdk::Vec3f&     Ng() const;
    //! Shading tangent, normalized
    const fdk::Vec3f&      T() const;

    //! Surface 2D parametric coordinates of P
    //! Note - Parametric UVs are required for dPdu & dPdv and stable bump mapping, etc,
    //! and these are not neccessarily 'texture' UVs. For example texture coords can be projected
    //! onto a surface which can break the parametric-ness of the UVs along the surface. To avoid
    //! this we keep them logically separated to make sure shading operations like bump mapping
    //! are not adversely impacted.
    //! When not projecting texture coords the value of uv() and st() will likely be identical.
    const fdk::Vec2f&     uv() const;
    //! Parametric-uv differential for dxDir ray direction
    const fdk::Vec2f&  dUVdx() const;
    //! Parametric-uv differential for dyDir ray direction
    const fdk::Vec2f&  dUVdy() const;

    //! Surface Z parametric coordinate of P (for volumes)
    float                uvw() const;
    //! Parametric-w differential for dxDir ray direction
    float             dUVWdx() const;
    //! Parametric-w differential for dyDir ray direction
    float             dUVWdy() const;

    //-------------------------------------------------------
    // Built-in shading layers:

    //! Texture st(uv) coordinate (with w divided out)
    const fdk::Vec2f&     st() const;
    //! Texture coord st differential for dxDir ray direction
    const fdk::Vec2f&  dStdx() const;
    //! Texture coord st differential for dyDir ray direction
    const fdk::Vec2f&  dStdy() const;
    //! Texture coord st partial derivative in parametric-u direction, not normalized
    const fdk::Vec2f&  dStdu() const;
    //! Texture coord st partial derivative in parametric-u direction, not normalized
    const fdk::Vec2f&  dStdv() const;

    //! Interpolated vertex color + opacity.
    const fdk::Vec4f&     Cf() const;
    const fdk::Vec4f&   rgba() const;
    const fdk::Vec3f&    rgb() const;
    float                  r() const;
    float                  g() const;
    float                  b() const;
    float                  a() const;
    float            opacity() const;

    //----------------------------------------------------------

    // Retrieve a primitive attribute (primvar) value by Channel index. The Channel index references
    // the first element in the value and the elements are assumed to be packed in order in the array.
    // Primitive attributes are retrieved from the primitive once a surface is hit and the
    // SlrShadingContext is filled in.
    // These values are usually read only since a shader does not have access to the raw primitive
    // data, but if required they can be set via the setShadingLayer() methods using the primvar's
    // channel indices. Warning, writing to primvar channels can lead to unpredictable results!

    int32_t               getIntPrimAttrib(ndk::Channel chan,      int32_t def_val) const;
    float               getFloatPrimAttrib(ndk::Channel chan,      float   def_val) const;
    double             getDoublePrimAttrib(ndk::Channel startChan, double  def_val) const;
    const fdk::Vec2f&   getVec2fPrimAttrib(ndk::Channel startChan, const fdk::Vec2f& def_val) const;
    const fdk::Vec3f&   getVec3fPrimAttrib(ndk::Channel startChan, const fdk::Vec3f& def_val) const;
    const fdk::Vec4f&   getVec4fPrimAttrib(ndk::Channel startChan, const fdk::Vec4f& def_val) const;

    Dual2f          getDual2fPrimAttrib(const Dual2Chans& chans, float  def_val) const;
    Dual2d          getDual2dPrimAttrib(const Dual2Chans& chans, double def_val) const;
    Dual2Vec2f  getDual2Vec2fPrimAttrib(const Dual2Chans& chans, const fdk::Vec2f& def_val) const;
    Dual2Vec3f  getDual2Vec3fPrimAttrib(const Dual2Chans& chans, const fdk::Vec3f& def_val) const;
    Dual2Vec4f  getDual2Vec4fPrimAttrib(const Dual2Chans& chans, const fdk::Vec4f& def_val) const;

    //----------------------------------------------------------

    // Retrieve a shading layer value by Channel index. The Channel index references
    // the first element in the value and the elements are assumed to be packed in order in
    // the array.
    // Shading layer channels are arbitrary data channels shared between shaders through the
    // SlrShadingContext and are separate from the AOV channels the renderer outputs.
    // See the PrimAttrib accessor methods for reading geometry primvars.

    int32_t               getIntShadingLayer(ndk::Channel chan,      int32_t def_val) const;
    float               getFloatShadingLayer(ndk::Channel chan,      float   def_val) const;
    double             getDoubleShadingLayer(ndk::Channel startChan, double  def_val) const;
    const fdk::Vec2f&   getVec2fShadingLayer(ndk::Channel startChan, const fdk::Vec2f& def_val) const;
    const fdk::Vec3f&   getVec3fShadingLayer(ndk::Channel startChan, const fdk::Vec3f& def_val) const;
    const fdk::Vec4f&   getVec4fShadingLayer(ndk::Channel startChan, const fdk::Vec4f& def_val) const;

    Dual2f                 getDual2fShadingLayer(const Dual2Chans& chans, float  def_val) const;
    Dual2d                 getDual2dShadingLayer(const Dual2Chans& chans, double def_val) const;
    Dual2Vec2f         getDual2Vec2fShadingLayer(const Dual2Chans& chans, const fdk::Vec2f& def_val) const;
    Dual2Vec3f         getDual2Vec3fShadingLayer(const Dual2Chans& chans, const fdk::Vec3f& def_val) const;
    Dual2Vec4f         getDual2Vec4fShadingLayer(const Dual2Chans& chans, const fdk::Vec4f& def_val) const;


#if 0
//----------------------------------------------------------------
// Primvars:
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
//----------------------------------------------------------------
#endif

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


  public:
    //! Access to the owner thread context. For internal use only.
    SlrThreadContext*  sttx() const { return _sttx; }

    //!
    void printParamData(std::ostream&) const;

  private:
    friend class SlrThreadContext;
    friend class SlrIntegratorContext;
    friend class ShadingDataCache;
    friend class RenderEngine;
    friend class RenderSpan; // (deprecated, remove)
    friend class IntersectedSurfaces; // (deprecated, remove)
    friend DDImage_API std::ostream& operator << (std::ostream&, const SlrShadingContext&);
    SlrShadingContext() = default;

  protected:
    SlrThreadContext*           _sttx;              //!< Owner of the shading data
    const SlrShadingContext*    _previousStx;       //!< Previous SlrShadingContext, normally the last surface intersected/shaded
    class ShadingDataCache*     _dataCache;         //!< Shading data storage
    int32_t                     _dataCacheIndex;    //!< Index of shading data in cache
    ndk::RayContext::SidesMode  _sidesMode;         //!< Which sides to intersect against and shade (use methods to query)
    bool                        _doTextureSampling; //!< Whether shaders should sample their textures
    DD::Image::TextureFilter*   _textureFilter;     //!< Global texture filter to use. May be null!
    SlrRenderObject*            _oIf;               //!< Object intersection info

  public://protected:
    //----------------------------------------------------------------
    // Deprecated scanline shading data - not used when ray tracing!
    float  _vP[28]; //!< Current values for shader - in homogeneous-space! (deprecated)
    float _vdX[28]; //!< Change if X of render pixel is increased by 1 (deprecated)
    float _vdY[28]; //!< Change if Y of render pixel is increased by 1 (deprecated)
    SlrRenderObject* rPrim() const { return _oIf; }
  };

  using SlrShadingContextList = std::vector<SlrShadingContext>;

  //-----------------------------------------------------------------------------


  class DDImage_API StxBundle
  {
  public:
    StxBundle(SlrThreadContext* sttx, int32_t bundleIndex);

    //! Bundle index in global StxBundle list.
    int32_t  index() const { return _index; }

    //! Starting SlrShadingContext index in global list.
    int32_t  startIndex() const { return _start; }
    int32_t  endIndex() const { return (_start + _size); }
    int32_t  size() const { return _size; }

    SlrShadingContext&        operator [] (int i);
    const SlrShadingContext&  operator [] (int i) const;

    //! Returns the first shading context in the bundle. Caution - if this bundle is empty the returned
    //! shading context may be from a previous bundle!
    SlrShadingContext&        front();
    //! Returns the last shading context in the bundle. Caution - if this bundle is empty the returned
    //! shading context may be from a previous bundle!
    SlrShadingContext&        back();

    SlrShadingContext*        array();
    const SlrShadingContext*  array() const;

    //! Iterator support
    typedef SlrShadingContextList::iterator       iterator;
    typedef SlrShadingContextList::const_iterator const_iterator;

    iterator        begin();
    const_iterator  begin() const;
    iterator        end();
    const_iterator  end() const;

    //! Sort the shading contexts from near to far distance where the first in the list is nearest.
    void sort();

    //! Sort the shading contexts from far to near distance where the first in the list is farthest.
    void sortFarToNear();

    //! Sets size of bundle to 0 and resizes list to start of bundle (retaining contents of prior bundles.)
    void clear();


  private:
    friend class SlrThreadContext;
    SlrThreadContext*  _sttx;     //!< Owner of the shading data
    int32_t            _index;    //!< Index of this bundle in parent bundle list
    int32_t            _start;    //!< Absolute starting index in shading data cache list
    int32_t            _size;     //!< Absolute inclusive-end index (loop is index=start; index < end)
  };

  using StxBundleList = std::vector<StxBundle>;


  //-----------------------------------------------------------------------------

  inline std::ostream& operator << (std::ostream& o, const Dual2f& b)
  {
    o << "(v" << b.val << " dx" << b.dx << " dy" << b.dy << ")";
    return o;
  }
  inline std::ostream& operator << (std::ostream& o, const Dual2d& b)
  {
    o << "(v" << b.val << " dx" << b.dx << " dy" << b.dy << ")";
    return o;
  }
  inline std::ostream& operator << (std::ostream& o, const Dual2Vec2f& b)
  {
    o << "(v" << b.val << " dx" << b.dx << " dy" << b.dy << ")";
    return o;
  }
  inline std::ostream& operator << (std::ostream& o, const Dual2Vec3f& b)
  {
    o << "(v" << b.val << " dx" << b.dx << " dy" << b.dy << ")";
    return o;
  }
  inline std::ostream& operator << (std::ostream& o, const Dual2Vec4f& b)
  {
    o << "(v" << b.val << " dx" << b.dx << " dy" << b.dy << ")";
    return o;
  }
  inline std::ostream& operator << (std::ostream& o, const Dual2Chans& b)
  {
    o << "(v=" << (int)b.val << " dx=" << (int)b.dx << " dy=" << (int)b.dy << ")";
    return o;
  }

  inline std::ostream& operator << (std::ostream& o, const ShadingLayerReq& b)
  {
    if (!b.empty()) {
      o << b.name << "[" << b.nChans << "]";
    }
    else {
      o << b.name << "<empty>";
    }
    return o;
  }

  inline std::ostream& operator << (std::ostream& o, const StxBundle& b)
  {
    o << "[";
    for (const SlrShadingContext& stx : b) {
      o << " " << stx.distance;
    }
    o << " ]";
    return o;
  }


} // namespace slr


#endif
