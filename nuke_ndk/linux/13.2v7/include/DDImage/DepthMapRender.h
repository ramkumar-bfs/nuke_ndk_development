// DepthMapRender.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_DepthMapRender_H
#define DDImage_DepthMapRender_H

#include "DDImage_API.h"

#include "Render.h"
#include "CameraOp.h"

namespace DD
{
  namespace Image
  {

    class LightOp;

    //! Depth map rendering for a shadow buffer
    class DDImage_API DepthMapRender : public Render
    {
    private:
      CameraOp *light_cam;  //!< Camera conformed to light's view
      int scene_index;      //!< Which scene index parent renderer is referring to

      Format _out_format;

    protected:
      int width_;       //!< Size width/height in pixels of buffer
      GeoOp* geometry_; //!< Geometry list to render

      /* virtual */ void _request(int x, int y, int r, int t, ChannelMask, int);

      /*! Initialize each sample Scene object and generate it's
          renderable primitives.
       */
      /* virtual */ bool generate_render_primitives();
      /* virtual */ void engine(int y, int x, int r, ChannelMask, Row &);

    public:
      DepthMapRender(Node * node, int width = 1024, int index = 0);
      void set_camera( CameraOp* light ) { light_cam = light; }
      void set_geometry(GeoOp* geometry) { geometry_ = geometry; }
      const GeoOp* geometry() const { return geometry_; }
      int width() const { return width_; }

      /* virtual */ void _validate(bool);

      /* virtual */ CameraOp* render_camera(int sample = 0) { return light_cam; }
      /* virtual */ GeoOp* render_geo(int sample = 0) { return geometry_; }

      /*! Returns the inverse camera matrix for a sample.
       */
      /* virtual */ Matrix4 camera_matrix(int sample = 0);
      /* virtual */ Matrix4 projection_matrix(int sample = 0);

      //  void initialize_span_context(Span& span);

      const char* Class() const { return "DepthMapRender"; }
      const char* node_help() const { return "internal shadow buffer"; }

    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
