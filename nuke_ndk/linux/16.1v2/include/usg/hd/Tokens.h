// Copyright (c) 2025 The Foundry Visionmongers Ltd. All Rights Reserved.

#pragma once

#include "usg/api.h"
#include "usg/base/Token.h"

namespace usg::hd {
  USG_API const TokenArray& LightPrimTypeTokens();
    struct USG_API HdTokensDef {
      struct {
        Token::Lazy Disabled{ "disabled" };
        Token::Lazy SRGB{ "sRGB" };
        Token::Lazy OpenColorIO{ "openColorIO" };
      } colorCorrection;
    };

    extern USG_API HdTokensDef HdTokens;
} // namespace usg::hd
