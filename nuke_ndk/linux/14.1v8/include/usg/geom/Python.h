// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_PYTHON_H
#define USG_GEOM_PYTHON_H

#include <Python.h>

#include "usg/geom/Layer.h"
#include "usg/geom/Stage.h"

namespace usg
{
  //! Returns the native Python object for a wrapped stage (a pxr.Usd.Stage).
  //! If USD is configured wthout Python bindings, this will return nullptr.
  USG_API PyObject* getPythonObject(const StageRef& stage);

  //! Returns the native Python object for a wrapped layer (an pxr.Sdf.Layer).
  //! If USD is configured wthout Python bindings, this will return nullptr.
  USG_API PyObject* getPythonObject(const LayerRef& layer);
}

#endif
