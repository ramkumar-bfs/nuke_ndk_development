// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_GEOM_TOKENS_H
#define USG_GEOM_TOKENS_H

#include "usg/api.h"

#include "usg/base/Token.h"

namespace usg {
  struct USG_API TokensDef {
    Token excludes{"excludes"};
    Token expansionRule{"expansionRule"};
    Token explicitOnly{"explicitOnly"};
    Token expandPrims{"expandPrims"};
    Token expandPrimsAndProperties{"expandPrimsAndProperties"};
    Token includeRoot{"includeRoot"};
    Token includes{"includes"};
    struct {
      Token APISchemaBase{"APISchemaBase"};
      Token ClipsAPI{"ClipsAPI"};
      Token CollectionAPI{"CollectionAPI"};
      Token ModelAPI{"ModelAPI"};
      Token Typed{"Typed"};
    } types;
    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  };

  extern USG_API TokensDef Tokens;
}

#endif
