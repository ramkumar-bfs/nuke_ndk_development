//------------------------------------------------------------------------------
// DDImage/LightMaterialOpI.h
//
// Copyright (c) 2021 The Foundry Visionmongers Ltd.  All Rights Reserved.
//------------------------------------------------------------------------------

#ifndef DDIMAGE_LIGHTMATERIALOP_H
#define DDIMAGE_LIGHTMATERIALOP_H

#include "DDImage/MaterialOpI.h"

#include "usg/geom/Layer.h"


namespace DD {
  namespace Image {

    /*! Interface for abstracting Light usg::Prims and their possible materials,
        usually built from LightOps.

        LightMaterialOpI converts a graph network of Nuke LightOp, AxisOp, and Iop
        Nodes into usg::Prims with the matching Xform parenting structure. At the
        moment (Nuke14-2022) the material and shader graph part is ignored in favor of
        using the direct texture assignment attributes on the Light Prims.
        When light shader graphs are formally supported they will likely be handled
        similar to surface materials and stored under a shared 'light library'
        locale in the scenegraph. This is the reason why this class is a subclass of
        MaterialOpI.

        The biggest topological difference between this and MaterialOpI is that
        a Light usg::Prim is created with possibly a parenting rig added above
        it rather than a single usg::MaterialPrim and multiple usg::ShaderPrims
        underneath.

    */
    class DDImage_API LightMaterialOpI : public MaterialOpI
    {
    public:
      //!
      LightMaterialOpI() : MaterialOpI() {}

      //! The 'shader node' name for LightOps is its full prim path so this method
      //! returns 'LightOp::primFullPath()'. The \a target_renderer arg is ignored.
      std::string  getShaderNodeName(const usg::Token& target_renderer = usg::Token()) override;


      //! Skips any leading 'inputs:' or 'outputs:' prefix in the prop name. This can
      //! commonly happen when the property names are coming from a Light Prim and
      //! not a shader. For example a Schema property named 'inputs:intensity' will
      //! be changed to 'intensity' since light prims don't use the same convention
      //! as shader prims.
      static const char*  conformPropName(const char* prop_name);


      //----------------------------------------------------------------------------
      //
      // Create or update shader descriptions for the wrapped shader class.
      //
      //----------------------------------------------------------------------------


      /*! Define a Light Prim from the wrapped LightOp in the provided GeomLayer,
          setting any attribute overrides.

          'light_path' is the absolute path to the new Light Prim with the new prim
          name at the end.

          Optionally initialize a ShaderDescGroup with all the ShaderDesc comprising
          this light's shader tree and its input connections, and assign the 'output'
          shader entry point for the group.
      */
      void  defineLightPrim(const usg::LayerRef&   define_layer,
                            const usg::Path&       light_prim_path,
                            const MaterialContext& rtx,
                            usg::ShaderDescGroup&  shaderdesc_group);


    protected:
      //----------------------------------------------------------------------------
      //
      // Input property override functions
      //
      //----------------------------------------------------------------------------

      /*! Copy the value from a local Op Knob to the ShaderDesc.
          Translates light Prim property names and trim any leading 'inputs:' or 'outputs:'.
      */
      bool  overrideShaderDescInput(const char*            knob_name,
                                    const MaterialContext& rtx,
                                    usg::ShaderDesc&       shader_desc,
                                    const char*            target_property_name,
                                    usg::Value::Type       target_property_type_hint = usg::Value::Type::InvalidType) override;

      /*! Specialization for Channel-source override.
          Translates light Prim property names and trim any leading 'inputs:' or 'outputs:'.
      */
      bool  overrideShaderDescInput(const Channel&         chan,
                                    const MaterialContext& rtx,
                                    usg::ShaderDesc&       shader_desc,
                                    const char*            target_property_name) override;

      /*! Specialization for ChannelArray-source override.
          Translates light Prim property names and trim any leading 'inputs:' or 'outputs:'.
      */
      bool  overrideShaderDescInput(const Channel*         chan_array,
                                    uint32_t               num_channels,
                                    const MaterialContext& rtx,
                                    usg::ShaderDesc&       shader_desc,
                                    const char*            target_property_name) override;

      /*! Specialization for ChannelSet-source override.
          Translates light Prim property names and trim any leading 'inputs:' or 'outputs:'.
      */
      bool  overrideShaderDescInput(const ChannelSet&      channels,
                                    const MaterialContext& rtx,
                                    usg::ShaderDesc&       shader_desc,
                                    const char*            target_property_name) override;

    };


  } // namespace Image
} // namespace DD

#endif
