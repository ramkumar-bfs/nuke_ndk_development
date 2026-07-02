//------------------------------------------------------------------------------
// ShaderOp.h
//
// Copyright (c) 2021 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef DDIMAGE_SHADEROP_H
#define DDIMAGE_SHADEROP_H

#include "DDImage/MaterialOpI.h"

#include <vector>

namespace DD
{
  namespace Image
  {
    class Iop;


    /*! \class DD::Image::ShaderOp

        Op base class for shader creating nodes.

        Note that unlike the legacy Material class, ShaderOp is explicitly
        *not* an Iop subclass and will not automatically work as a 'texture'
        source for other shaders.

        Connection of ShaderOps to legacy Material class Ops is not allowed! This
        is explicitly to avoid blending 'Shader' nodes with image nodes within the
        same graph branch to better delineate which is a texture source to a shader
        and what is the shader using those textures.

        The previous Material class conflated these concepts leading to confusing
        results when Material nodes were mixed with Iop nodes. For example it did
        not make much sense to have a Diffuse shader followed by a Blur node since
        the Blur is not processing random, spatially-incoherent surface samples like
        a surface shader does. Plus the output of Blur did not affect the output of
        the Diffuse node, and yet that connection was allowed.
    */
    class DDImage_API ShaderOp : public Op,
                                 public MaterialOpI
    {
    public:
      //! Return this ShaderOp cast to the MaterialOpI interface, if it supports translation.
      virtual MaterialOpI*  asMaterialOp() { return nullptr; }


    public:
      ShaderOp(Node*);

      //! Define the material op's shape in the DAG.
      const char*  node_shape() const override;
      //! All material operators default to a orangish color.
      unsigned     node_color() const override;

      /*! Allow ShaderOp or Iop connections *only*.

          Connection to legacy 'Material' class is not allowed! This is done
          explicitly to avoid mixing up 'shader' nodes with 'image' nodes.

          An Iop connection is explicitly a texture source, and a ShaderOp
          connection is explicitly a shader source.

          connectionTypeForNodeInput() is queried to further restrict the connection
          to explicitly Iops or explicitly ShaderOps. If the input is a
          ShaderOp its getOutputType() method is queried to match the connection
          types, and false is returned if they don't.

          Other input Op types are not handled in the base class implementation.
      */
      bool         test_input(int node_input,
                              Op* op) const override;

      /*! Returns nullptr for all inputs, including 0.

          This means any disconnected input will not be connected to any valid Op
          by default, and all Op::input() tests should check for a null pointer result.

          Note that the default output of this Op type is NOT an Iop so do not expect
          the 'Black' default Iop to be automatically connected to a dangling input.
      */
      Op*          default_input(int node_input) const override { return nullptr; }

      //-------------------------------

      //! Return the input Op cast to an Iop, if possible.
      Iop*         textureInput(int32_t node_input) const;


      //-------------------------------

      //! Create the ShaderDescs required to interface to an Iop texture source.
      static usg::ShaderDesc*  createIopInterfaceGraph(Iop*                   input_iop,
                                                       int32_t                output_type,
                                                       const MaterialContext& rtx,
                                                       usg::ShaderDescGroup&  shader_group);

      //! Update the ShaderDescs required to interface to an Iop texture source.
      static void              updateIopInterfaceGraphOverrides(Iop*                   input_iop,
                                                                int32_t                output_type,
                                                                const MaterialContext& rtx,
                                                                usg::ShaderDescGroup&  shader_group);

      //-------------------------------

      //! Create the ShaderDescs required to interface to a CoShader source.
      static usg::ShaderDesc*  createCoShaderInterfaceGraph(ShaderOp*              input_coshader,
                                                            int32_t                output_type,
                                                            const MaterialContext& rtx,
                                                            usg::ShaderDescGroup&  shader_group);

      //! Update the ShaderDescs required to interface to a CoShader source.
      static void              updateCoShaderInterfaceGraphOverrides(ShaderOp*              input_coshader,
                                                                     int32_t                output_type,
                                                                     const MaterialContext& rtx,
                                                                     usg::ShaderDescGroup&  shader_group);

    };


    //------------------------------------------------------------------------


    /*! Temp shader object used during ShaderDesc network creation to inject the
        default surface shader between a co-shader input (non-surface shader) and
        the Material output.

        Calls the static methods on the ShaderOp base class that do this work.
    */
    class DDImage_API CoShaderMaterialInterface : public ShaderOp
    {
    public:
      Op* op;

      static const Description description;
      const char*  Class()     const override { return "CoShaderMaterialInterface"; }
      const char*  node_help() const override { return ""; }

      CoShaderMaterialInterface(Op*);

      MaterialOpI*      asMaterialOp() override { return static_cast<MaterialOpI*>(this); }
      Op*               getOp() override { return this; }
      const char*       getOutputSchema() const override;
      int32_t           getOutputType() const override;

      //! Calls ShaderOp::createIopInterfaceGraph().
      usg::ShaderDesc*  createShaderGraph(int32_t                output_type,
                                          const MaterialContext& rtx,
                                          usg::ShaderDescGroup&  shader_group) override;
      //! Calls ShaderOp::updateIopInterfaceGraphOverrides().
      void              updateShaderGraphOverrides(int32_t                output_type,
                                                   const MaterialContext& rtx,
                                                   usg::ShaderDescGroup&  shader_group) override;
    };


  } // namespace Image
} // namespace DD

#endif
