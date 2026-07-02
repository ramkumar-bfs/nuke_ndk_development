// Copyright (c) 2023 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_UTILS_H
#define USG_GEOM_UTILS_H

#include "fdk/base/StringTypes.h"
#include "usg/api.h"
#include "usg/base/Value.h"
#include "usg/base/Token.h"
#include "usg/geom/Attribute.h"
#include "usg/geom/Layer.h"
#include "usg/geom/Relationship.h"

#include <set>

namespace usg {

  USG_API const fdk::StringList& KindNames();

  USG_API const fdk::StringList& PurposeNames();

  // Returns a set of the all the registered Concrete Schema
  // Will load any associated plugin to discover the total set available
  // in the current USD session.
  USG_API const fdk::StringList& GetConcreteSchemaTypeNames();
  USG_API const fdk::StringList& GetAPISchemaTypeNames();
  USG_API const fdk::StringList& GetSchemaTfTypeNames();

  USG_API std::vector<usg::Attribute> GetDefaultAttributesForType(const usg::Token& typeName);
  USG_API std::vector<usg::Relationship> GetDefaultRelationshipsForType(const usg::Token& typeName);

  USG_API std::vector<usg::Attribute> GetDefaultAttributesForAPISchema(
    const usg::Token& schemaTypeName);
  USG_API std::vector<usg::Relationship> GetDefaultRelationshipsForAPISchema(
    const usg::Token& schemaTypeName);

  USG_API void StitchLayers(const usg::LayerRef &strongLayer, const usg::LayerRef &weakLayer);
}

#endif
