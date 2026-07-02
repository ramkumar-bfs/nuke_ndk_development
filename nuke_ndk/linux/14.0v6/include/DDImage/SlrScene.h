//------------------------------------------------------------------------------
// DDImage/SlrScene.h
//
// Copyright (c) 2021 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef SLR_SCENE_H
#define SLR_SCENE_H

#include "DDImage/SlrPrimInfo.h"
#include "DDImage/SlrLightInfo.h"
#include "DDImage/SlrShadingContext.h"
#include "DDImage/TextureFilter.h"
#include "DDImage/CameraOp.h" // for CameraLensContext


namespace slr
{

  /*! Render-time scene snapshot used by ScanlineRender2 plugin and other render plugins
      on the same architecture, such as DepthMapRender which is used for light shadow maps.

      This structure represents a snapshot of all objects and transforms that are needed
      to render a certain camera view at a certain time.

      Use the next/prev scene methods to access other SlrScenes for motionblur purposes.
      All SlrScenes shared by the sample renderer will share the same stage, object and
      light lists so matching objects and lights between scenes means accessing the same
      one by index in the other SlrScene.
   */

  class DDImage_API SlrScene
  {
  protected:
    fdk::TimeValue  _time;                  //!< Absolute scene time
    double          _shutter_offset;        //!< Time offset relative to shutter open
    int32_t         _motion_sample;         //!< Shutter motion sample this scene represents
    int32_t         _num_motion_samples;    //!< Total number of shutter motion samples
    SlrScene*       _prev_scene;            //!< Previous scene in time to motionblur from
    SlrScene*       _next_scene;            //!< Next scene in time to motionblur to


  public:
    //--------------------------------------------------------

    SlrPrimInfoList           objects;                //!< Geometry objects - references to usg::GeometryPrims
    SlrLightInfoList          lights;                 //!< Lights and their vectors - references to ndk light Prims

    //--------------------------------------------------------

    fdk::Mat4dList            transforms;             //!< Global scene transforms (camera, projection, format)
    const DD::Image::Format*  format;                 //!< Output render format

    fdk::Box3d                object_bbox;            //!< Bounding box of all active, renderable objects
    fdk::Box2f                render_region;          //!< Active screen-space bbox (dataWindow)
    DD::Image::PixelBox       screen_bbox;            //!< Screen-projected version of _object_bbox

    DD::Image::ChannelSet     material_channels;      //!< Intersected output channels of all materials
    DD::Image::ChannelSet     texture_channels;       //!< Intersected input channels of all source textures
    DD::Image::ChannelSet     shadow_channels;        //!< Intersected output channels for all light shadows

    DD::Image::TextureFilter* default_texture_filter; //!< Filter to use if shaders do not specify

    //--------------------------------------------------------
    // Camera
    fdk::Vec3d                      camPW;            //!< Camera origin in world-space
    fdk::Vec3f                      camX;             //!< Camera X vector in world-space, normalized
    fdk::Vec3f                      camY;             //!< Camera Y vector in world-space, normalized
    fdk::Vec3f                      camZ;             //!< Camera Z vector in world-space, normalized
    DD::Image::CameraLensContext    lens_context;     //!< Camera projection context
    DD::Image::LensProjectionFunc*  lens_func;        //!< Camera projection function to use

    //--------------------------------------------------------
    // Illum/Shading

    fdk::Vec3f        ambient;                  //!< Ambient RGB color
    ShadowmapProbe*   shadow_probe_func;        //!< Shadowmap renderer probe function to use


  public:
    SlrScene();
    virtual ~SlrScene() {}

    //! Copy values from source scene.
    void copyInfo(const SlrScene* src);

    //-------------------------------------------------
    // Time / Motionblur / Shutter

    fdk::TimeValue time() const { return _time; }
    double shutterOffset() const { return _shutter_offset; }

    //! Total number of shutter steps. A shutter step is the time interval between 2 motion samples.
    int32_t nShutterSteps() const { return std::max(1, _num_motion_samples - 1); }

    //! Total number of motion samples.
    int32_t nMotionSamples() const { return _num_motion_samples; }

    //! The motion sample index this Scene represents.
    int32_t motionSample() const { return _motion_sample; }

    //-------------------------------------------------

    SlrScene* prevScene() const { return _prev_scene; }
    SlrScene* nextScene() const { return _next_scene; }

    SlrPrimInfo&   object(int n) { return objects[n]; }
    SlrLightInfo&  light(int n)  { return lights[n]; }

    //-------------------------------------------------
    // Scene transform convenience
    const fdk::Mat4d&  matrix(int n)      const { return transforms[n]; }
    const fdk::Mat4d&  cameraMatrix()     const { return transforms[CAMERA_MAT4]; }
    const fdk::Mat4d&  projectionMatrix() const { return transforms[PROJECTION_MAT4]; }
    const fdk::Mat4d&  formatMatrix()     const { return transforms[FORMAT_MAT4]; }

  private:
    SlrScene& operator = (const SlrScene&) = delete;
    SlrScene(const SlrScene&) = delete;

  };


} // namespace slr

#endif
