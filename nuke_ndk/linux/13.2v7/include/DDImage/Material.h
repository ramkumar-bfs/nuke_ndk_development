// Material.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

// Material class describes how to shade a object

#ifndef DDImage_Material_h
#define DDImage_Material_h

#include "Iop.h"
#include "Black.h"
#include "Vector3.h"
#include "VertexContext.h"

#include <vector>

//Uncomment this to use fast_normalize functions in the shaders:
#define USE_FAST_NORMALIZE 1

namespace DD
{
  namespace Image
  {

    class LightContext;

    // Default Material class is simply to allow connections:
    /*! \class DD::Image::Material

       This subclass of Iop provides some convienence functions so that it
       is easier to shade a 3D geometry object, such as calls to calculate
       the blinn or other lighting models, and the output has been modified
       to produce an image of a sphere lit using this material.

       To make a useful Material, you almost certainly need to override
       these Iop functions:
     * bool Iop::shade_GL(ViewerContext*, GeoInfo&);
     * void Iop::fragment_shader(Scene*, VertexContext*, Pixel& out);

     */
    class DDImage_API Material : public Iop
    {
    public:
      Material(Node*);
      ~Material();

      /*! Define the material op's shape in the DAG. */
      /*virtual*/ const char* node_shape() const;

      /*! All material operators default to a orangish color. */
      /*virtual*/ unsigned node_color() const;

      /*virtual*/ void knobs(Knob_Callback);

      /*! Hash up knobs that can affect the geometric hashes.
          Base class just calls its input.
       */
      virtual void get_geometry_hash(Hash* geo_hashes);

      /*virtual*/ bool set_texturemap(ViewerContext* ctx, bool gl)
      { return input0().set_texturemap(ctx, gl); }
      /*virtual*/ void unset_texturemap(ViewerContext* ctx)
      { input0().unset_texturemap(ctx); }

      /*! Your subclass must implement this. This version produces an error. */
      /*virtual*/ bool shade_GL(ViewerContext*, GeoInfo&);

      /*! Default version does nothing. */
      /*virtual*/ void vertex_shader(VertexContext&);

      /*! Your subclass must implement this. This version produces an error. */
      /*virtual*/ void fragment_shader(const VertexContext&, Pixel& out);
      /*virtual*/ void displacement_shader(const VertexContext& vtx, VArray& out);
      /*virtual*/ float displacement_bound() const;
      /*virtual*/ void blending_shader(const Pixel& in, Pixel& out);
      /*virtual*/ void render_state(GeoInfoRenderState& state);

      /*! Not implemented in the library but you can define it */
      friend std::ostream& operator << (std::ostream&, const Material&);

      /* Default Lambertian diffuse shading */
      static void default_lighting(const VertexContext& vtx, Pixel& out);

      static const Vector4 gDefaultAmbient;
      static const Vector4 gDefaultDiffuse;
      static const Vector4 gDefaultSpecular;
      static const Vector4 gDefaultEmission;

    protected:
      /*virtual*/ void _validate(bool);
      /*virtual*/ void _request(int x, int y, int r, int t, ChannelMask, int count);

      /*! If a Material is used as an Iop, it produces an image of a sphere
         rendered with the shader atop a gray background, with a light at the
         upper-left corner.
       */
      /*virtual*/ void engine(int y, int x, int r, ChannelMask, Row &);

      /*! Call this at the start of fragment shader for Materials that do lighting. */
      void set_blending_shader(const VertexContext& vtx)
      { if (vtx.blending_shader == nullptr)
          ((VertexContext*)&vtx)->blending_shader = this;}

    };

    //! Default solid vertex shader.
    /*! Solid Shader.  Interpolates the foreground vertex color. */
    class DDImage_API SolidShader : public Iop
    {
    public:
      const char* Class() const { return "SolidShader"; }
      const char* node_help() const { return ""; }
      SolidShader(Node*);

      /*virtual*/ bool set_texturemap(ViewerContext* ctx, bool gl);
      /*virtual*/ bool shade_GL(ViewerContext*, GeoInfo&);
      /*virtual*/ void unset_texturemap(ViewerContext*);

      /*! Enable interpolation of Cf channels. */
      /*virtual*/ void vertex_shader(VertexContext&);

      /*! This renders using the foreground color Cf. */
      /*virtual*/ void fragment_shader(const VertexContext&, Pixel& out);

      /*! global solid shader */
      static SolidShader sSolidShader;

    protected:
      /*virtual*/ void _validate(bool);
      /*virtual*/ void _request(int x, int y, int r, int t, ChannelMask, int count);
      /*virtual*/ void engine(int y, int x, int r, ChannelMask, Row &);
    };

    class DDImage_API ColoredShader : public SolidShader
    {
    public:
      const char* Class() const { return "ColoredShader"; }
      const char* node_help() const { return ""; }
      ColoredShader(Node*);      
      bool shade_GL(ViewerContext*, GeoInfo&);
      void unset_texturemap(ViewerContext*);
      void fragment_shader(const VertexContext&, Pixel& out);
    }; 

    //! Default wireframe shader.
    class DDImage_API WireframeShader : public Iop
    {
    public:
      const char* Class() const { return "WireframeShader"; }
      const char* node_help() const { return ""; }
      WireframeShader(Node*);

      bool shade_GL(ViewerContext*, GeoInfo&);
      void vertex_shader(VertexContext& vtx);
      void fragment_shader(const VertexContext&, Pixel& out);

      /*! global wireframe shader */
      static WireframeShader sWireframeShader;

    protected:
      /*virtual*/ void _validate(bool);
      /*virtual*/ void _request(int x, int y, int r, int t, ChannelMask, int count);
      /*virtual*/ void engine(int y, int x, int r, ChannelMask, Row &);
    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
