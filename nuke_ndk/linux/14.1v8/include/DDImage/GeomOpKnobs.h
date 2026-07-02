// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef DDIMAGE_GEOMOPKNOBS_H
#define DDIMAGE_GEOMOPKNOBS_H

#include "DDImage/GeomOp.h"

namespace DD
{
  namespace Image
  {

    //! A class which provides UI for the model API.
    //! Create one of these and call the knobs method to make the knobs.
    //! Then call the apply method to author the attributes.
    class DDImage_API ModelKnobs {
    public:
      ModelKnobs() {}
      ModelKnobs(int kind);

      void knobs(Knob_Callback f, GeomOp* op);

      class DDImage_API Binding {
      public:
        Binding(GeomOpEngine& engine);

        void apply(usg::Prim& prim, const fdk::TimeValue& time = fdk::defaultTimeValue());

      private:
        const GeomOpEngine::KnobBinding _kindKnob;
      };

    private:
      int _kind = 0;
    };

    //! A class which provides common UI for Imageable prims.
    //! Create one of these and call the knobs method to make the knobs.
    //! Then call the apply method to author the attributes.
    class DDImage_API ImageableKnobs {
    public:
      ImageableKnobs() {}
      ImageableKnobs(int purpose, int visibility = 0);

      void knobs(Knob_Callback f, GeomOp* op);

      class DDImage_API Binding {
      public:
        Binding(GeomOpEngine& engine);

        void apply(usg::Prim& prim, const fdk::TimeValue& time = fdk::defaultTimeValue());

      private:
        const GeomOpEngine::KnobBinding _purposeKnob;
        const GeomOpEngine::KnobBinding _visibilityKnob;
      };

    private:
      int _purpose = 0;
      int _visibility = 0;
    };

    //! A class which provides common UI for Geometry prims.
    //! Create one of these and call the knobs method to make the knobs.
    //! Then call the apply method to author the attributes.
    class DDImage_API GeometryKnobs {
    public:
      GeometryKnobs() {}
      GeometryKnobs(const fdk::Vec3f& displayColor, float displayOpacity = 1.0f, bool doubleSided = false);

      void knobs(Knob_Callback f, GeomOp* op);

      class DDImage_API Binding {
      public:
        Binding(GeomOpEngine& engine);

        void apply(usg::Prim& prim, const fdk::TimeValue& time = fdk::defaultTimeValue());

      private:
        const GeomOpEngine::KnobBinding _displayColorKnob;
        const GeomOpEngine::KnobBinding _displayOpacityKnob;
        const GeomOpEngine::KnobBinding _doubleSidedKnob;
      };

    private:
      fdk::Vec3f _displayColor{1.0f};
      float _displayOpacity = 1.0f;
      bool _doubleSided = false;
    };

    //! A class which provides common UI for Mesh prims.
    //! Create one of these and call the knobs method to make the knobs.
    //! Then call the apply method to author the attributes.
    class DDImage_API MeshKnobs {
    public:
      MeshKnobs() {}
      MeshKnobs(int subdivisionScheme, int interpolateBoundary = 0);

      void knobs(Knob_Callback f, GeomOp* op);

      class DDImage_API Binding {
      public:
        Binding(GeomOpEngine& engine);

        void apply(usg::Prim& prim, const fdk::TimeValue& time = fdk::defaultTimeValue());

      private:
        const GeomOpEngine::KnobBinding _subdivisionSchemeKnob;
        const GeomOpEngine::KnobBinding _interpolateBoundaryKnob;
      };

    private:
      int _subdivisionScheme = 0;
      int _interpolateBoundary = 0;
    };

  }
}

#endif
