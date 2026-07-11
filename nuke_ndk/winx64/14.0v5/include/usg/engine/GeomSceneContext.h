// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_ENGINE_GEOMSCENECONTEXT_H
#define USG_ENGINE_GEOMSCENECONTEXT_H

#include "usg/engine/api.h"
#include "usg/engine/GeomState.h"

#include "fdk/base/Time.h"

#include "usg/base/ArgSet.h"


namespace usg
{

  /*! Context for GeomEngine getScenegraph(), processScenegraph() to fill the time
      samples the scene consumer wants.

      This class also contains an arbitrary set of scene-global key/value attributes
      that can help guide or define behaviors. Some are scoping parameters set by
      downstream GeomEngines to be passed upstream to guide the creation of prims or
      materials.
  */
  class USG_ENGINE_API GeomSceneContext
  {
  public:
    //! Default ctor creates a default TimeValue.
    GeomSceneContext() { _processTimes.insert(fdk::defaultTimeValue()); }
    //! Sets the process times.
    GeomSceneContext(const fdk::TimeValueSet& times) : _processTimes(times) {}
    //! Sets the process times and the scope vars.
    GeomSceneContext(const fdk::TimeValueSet& times,
                     const ArgSet&            scope_vars) : _processTimes(times), _requestScopeVars(scope_vars) {}
    //! Copies from second context, and either replaces or adds to the process times.
    GeomSceneContext(const GeomSceneContext&  b,
                     const fdk::TimeValueSet& times,
                     bool                     append_times = false);

    //-------------------------------------------------------------

    //! Additional request context scope vars for upstream GeomEngines to use.
    const ArgSet&             requestScopeVars() const { return _requestScopeVars; }

    //! Overwrites the current request context scope vars.
    void                      setRequestScopeVars(const ArgSet& vars) { _requestScopeVars = vars; }

    //-------------------------------------------------------------

    //! Returns the set of time values to process at.
    const fdk::TimeValueSet&  processTimes() const { return _processTimes; }

    //! Returns the number of times to process.
    size_t                    numProcessTimes() const { return _processTimes.size(); }


    friend USG_ENGINE_API std::ostream& operator << (std::ostream&, const GeomSceneContext&);


  protected:
    friend class GeomEngine; // So GeomEngine can directly manipulate _processTimes

    ArgSet             _requestScopeVars;   //!< Provides additional context for upstream GeomEngines
    fdk::TimeValueSet  _processTimes;       //!< Times to process the scene at
  };


  USG_ENGINE_API std::ostream& operator << (std::ostream&, const GeomSceneContext&);

} // namespace usg

#endif
