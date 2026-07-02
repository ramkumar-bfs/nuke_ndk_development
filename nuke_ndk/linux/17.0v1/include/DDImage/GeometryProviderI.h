// Copyright (c) 2025 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDIMAGE_GEOMETRYPROVIDERI_H
#define DDIMAGE_GEOMETRYPROVIDERI_H

#include "DDImage/DDImage_API.h"

#include "usg/geom/Layer.h"
#include "usg/geom/Stage.h"
#include "usg/base/ArgSet.h"

namespace DD {
  namespace Image {
    class Op;
    struct OpGraphLocation;


    /*! Interface class to attach to another class, usually an Op, that can provide
        geometry in the form of usg::Layers or a usg::Stage. Subclasses can decide to
        provide both a Layer and a Stage, or just a Layer. If providing both then the
        Layer would commonly be the root layer set in the provided Stage.

        Op examples utilizing this interface are GeomOp, AxisOp (CameraOp, LightOp) and
        ScanlineRender2. 

        The returned Layer and Stage are usually local caches on the attached object as
        downstream consumers typically do not have persistent caches in Nuke due to the
        procedural graph mechanisms. A good example of this is the Viewer which contains
        a Stage reference but relies on each Node to store the geometry data in local
        Layers and provide a Stage when asked.

        However the object could also pass the interface methods up the graph to an input
        to provide geometry if the object itself is not geometry aware. A good use of
        this would be an Iop with a geometry input that wants the Viewer to access the
        geometry passing through the Iop. ScanlineRender2 does this for example.

        Note that code accessing GeometryProvider methods should be aware that it may
        not always be on an Op and should be able to handle null being returned from
        getGeometryProviderOp().
    */
    class DDImage_API GeometryProviderI
    {
    public:
      //------------------------------------------------------
      // Build a filled in usg::Stage:

      //! Build the passed-in stage. If \a geometryProviderOp is null or \a sampleTimes
      //! is empty the stage is emptied.
      //!
      //! The provider Op is typically an input to a Viewer, renderer Iop, or an Op that
      //! wants to sample geometry. Only Ops that can generate geometry (specifically
      //! a Layer or Stage) are supported, which includes GeomOps or any Op with the
      //! GeometryProviderI interface attached.
      //! 
      //! \a sampleTimes is the set of times to be processed/filled for the usg::Stage.
      //! A GeometryProvider should try to fulfill the requested times at a minimum, but
      //! is free to add additional time samples.
      static void  BuildStage(usg::StageRef&           stage,
                              const usg::ArgSet&       requestArgs,
                              const OpGraphLocation&   geometryProviderOp,
                              const fdk::TimeValueSet& sampleTimes);

      //! Same as buildStage(OpSet) but with no request args.
      static void  BuildStage(usg::StageRef&           stage,
                              const OpGraphLocation&   geometryProviderOp,
                              const fdk::TimeValueSet& sampleTimes);

      //! Same as buildStage(OpSet) but with only one sample time.
      static void  BuildStage(usg::StageRef&         stage,
                              const OpGraphLocation& geometryProviderOp,
                              const fdk::TimeValue&  sampleTime);

    public:
      GeometryProviderI() = default;
      virtual ~GeometryProviderI() = default;

      //! Allow subclasses to gain access to sibling functions. Should return 'this'.
      //! Must implement.
      virtual GeometryProviderI*  asGeometryProvider() = 0;


      //--------------------------------------------
      // Op attachment:

      //! If the interface is attached to an Op return it. Should return 'this' if attached
      //! to an Op, otherwise null.
      //! Must implement.
      virtual Op*  getGeometryProviderOp() = 0;

      //! Same as getGeometryProviderOp() but const.
      Op*  getGeometryProviderOp() const { return const_cast<GeometryProviderI*>(this)->getGeometryProviderOp(); }


      //--------------------------------------------
      // Geometry state:

      //! Compose state of output geometry.
      //! Subclass should return a hash which represents the composition state (define geometry)
      //! of the stage it could produce. Multiple calls to this method should return exactly
      //! the same hash as long as a built stage does not need to be recomposed.
      //!
      //! Must implement.
      virtual fdk::Hash  geometryComposeState() = 0;

      //! Edit version state of output layer(s).
      //! Subclass should return a hash which represents the edit version state (modify values)
      //! of the layers it will produce. Multiple calls to this method should return exactly the
      //! same hash as long as the resulting Layer attribute can be value-modified without
      //! changing its definition, which is a composition change and should be reflected in the
      //! geometryComposeState().
      //! When this version hash changes the contents of all attributes in the layer are likely
      //! invalid and any existing time samples cleared out.
      //!
      //! Must implement.
      virtual fdk::Hash  geometryEditVersionState() = 0;

      //! Does the geometry state have contributions from an animating source?
      //! A subclass should either return true if it doesn't know for certain, or determine the
      //! animation state from local parameters and input info.
      //!
      //! This is only a hint to downstream nodes that the per-frame evaluated hash state of an Op
      //! will -likely- vary, it does not indicate that the actual geometry data -does- vary over
      //! time.
      //!
      //! Must implement.
      virtual bool  geometryStateVaries() = 0;


      //--------------------------------------------
      // Layer output:

      //! Build a usg::Layer, which is owned by the GeometryProviderI subclass.
      //! \a sampleTimes is for optional samples that the caller might need explicit time
      //! samples for, but this behavior is dependent on the subclass implementation.
      //!
      //! If \a appendTo is false (the default) then the subclass is assumed to be initializing
      //! its Layer and defining prims and attributes.
      //! If \a appendTo is true then the subclass should only add additional time samples to
      //! the previously-defined attributes and avoid changing the existence of prims or
      //! attributes as this would cause a Stage using the Layer to recompose.
      //!
      //! Must implement.
      virtual usg::LayerRef  buildGeometryLayer(bool                     appendTo = false,
                                                const fdk::TimeValueSet& sampleTimes = fdk::TimeValueSet()) = 0;

      //! Provide a fully populated usg::Layer. \a sampleTimes is for optional sample times
      //! that a consumer might need explicit time samples for, but this behavior is dependent
      //! on the subclass implementation of buildGeometryLayer().
      //!
      //! Default implementation calls buildGeometryLayer() with a local LayerRef and returns it.
      //! This is virtual to allow a subclass to do something more clever.
      virtual usg::LayerRef  getGeometryLayer(const fdk::TimeValueSet& sampleTimes = fdk::TimeValueSet());


      //--------------------------------------------
      // Stage building:

      //! Return true if the Op this is attached to can build a usg::Stage via buildGeometryStage().
      //!
      //! Default is not to provide Stage building.
      virtual bool  canProvideGeometryStage() const { return false; }

      //! Return true if the Op this is attached to can provide geometry for the given purpose.
      //! An example of where this can be helpful is where you don't want to provide cameras/lights 
      //! from the provider unless you require the overlay iconography.
      //! Default is to always provide geometry for all purposes.
      virtual bool  canProvideGeometryFor(const usg::Token& purpose) const { return true; }

      //! Build the provided usg::Stage. \a sampleTimes is for optional sample times that the caller
      //! might want explicit time samples for. However whether those additional times are filled
      //! in is depedent on the subclass implementation of buildGeometryStage().
      //!
      //! Default implementation creates the stage from the layer built from calling
      //! buildGeometryLayer(). It compares the geometryComposeState() against the provided
      //! stage's composeState and if the same the stage won't be created and buildGeometryLayer()
      //! will be called in appendTo=true mode so the layer attribute contents can be modified
      //! without stage recomposition. If newly created the stage is set to LoadAll mode and its
      //! composeState set to the current geometryComposeState().
      virtual void  buildGeometryStage(usg::StageRef&           stage,
                                       const usg::ArgSet&       requestArgs,
                                       const fdk::TimeValueSet& sampleTimes = fdk::TimeValueSet());

      //! Provide a fully composed usg::Stage.
      //!
      //! \a sampleTimes is for the sample times that the caller may want explicit time samples
      //! for. However whether those additional times are filled in is depedent on the subclass
      //! implementation of buildGeometryStage().
      //!
      //! If the stage consumer wants to call the stage building repeatedly with different sample
      //! times then buildGeometryStage() should be used instead.
      //!
      //! Default implementation calls buildGeometryStage() with a local StageRef and returns it.
      //! This is virtual to allow a subclass to do something more clever.
      virtual usg::StageRef  getGeometryStage(const fdk::TimeValueSet& sampleTimes = fdk::TimeValueSet());

    };

  }
}

#endif
