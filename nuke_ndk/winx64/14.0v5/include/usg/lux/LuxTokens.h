// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_LUX_LUXTOKENS_H
#define USG_LUX_LUXTOKENS_H

#include "usg/api.h"

#include "usg/base/Token.h"

namespace usg {
namespace lux {
  struct USG_API LuxTokensDef {
    Token angular{"angular"};
    Token automatic{"automatic"};
    Token consumeAndContinue{"consumeAndContinue"};
    Token consumeAndHalt{"consumeAndHalt"};
    Token cubeMapVerticalCross{"cubeMapVerticalCross"};
    Token extent{"extent"};
    Token filters{"filters"};
    Token geometry{"geometry"};
    Token guideRadius{"guideRadius"};
    Token ignore{"ignore"};
    Token independent{"independent"};
    Token infoId{"info:id"};
    Token infoImplementationSource{"info:implementationSource"};
    Token inputsAngle{"inputs:angle"};
    Token inputsColor{"inputs:color"};
    Token inputsColorTemperature{"inputs:colorTemperature"};
    Token inputsDiffuse{"inputs:diffuse"};
    Token inputsEnableColorTemperature{"inputs:enableColorTemperature"};
    Token inputsExposure{"inputs:exposure"};
    Token inputsHeight{"inputs:height"};
    Token inputsIntensity{"inputs:intensity"};
    Token inputsLength{"inputs:length"};
    Token inputsNormalize{"inputs:normalize"};
    Token inputsRadius{"inputs:radius"};
    Token inputsShadowColor{"inputs:shadow:color"};
    Token inputsShadowDistance{"inputs:shadow:distance"};
    Token inputsShadowEnable{"inputs:shadow:enable"};
    Token inputsShadowFalloff{"inputs:shadow:falloff"};
    Token inputsShadowFalloffGamma{"inputs:shadow:falloffGamma"};
    Token inputsShapingConeAngle{"inputs:shaping:cone:angle"};
    Token inputsShapingConeSoftness{"inputs:shaping:cone:softness"};
    Token inputsShapingFocus{"inputs:shaping:focus"};
    Token inputsShapingFocusTint{"inputs:shaping:focusTint"};
    Token inputsShapingIesAngleScale{"inputs:shaping:ies:angleScale"};
    Token inputsShapingIesFile{"inputs:shaping:ies:file"};
    Token inputsShapingIesNormalize{"inputs:shaping:ies:normalize"};
    Token inputsSpecular{"inputs:specular"};
    Token inputsTextureFile{"inputs:texture:file"};
    Token inputsTextureFormat{"inputs:texture:format"};
    Token inputsWidth{"inputs:width"};
    Token latlong{"latlong"};
    Token lightFilters{"light:filters"};
    Token lightMaterialSyncMode{"light:materialSyncMode"};
    Token lightShaderId{"light:shaderId"};
    Token lightFilterShaderId{"lightFilter:shaderId"};
    Token lightList{"lightList"};
    Token lightListCacheBehavior{"lightList:cacheBehavior"};
    Token materialGlowTintsLight{"materialGlowTintsLight"};
    Token mirroredBall{"mirroredBall"};
    Token noMaterialResponse{"noMaterialResponse"};
    Token portals{"portals"};
    Token proxyPrim{"proxyPrim"};
    Token purpose{"purpose"};
    Token treatAsLine{"treatAsLine"};
    Token treatAsPoint{"treatAsPoint"};
    Token visibility{"visibility"};
    Token xformOpOrder{"xformOpOrder"};
    struct {
      Token BoundableLightBase{"BoundableLightBase"};
      Token CylinderLight{"CylinderLight"};
      Token DiskLight{"DiskLight"};
      Token DistantLight{"DistantLight"};
      Token DomeLight{"DomeLight"};
      Token GeometryLight{"GeometryLight"};
      Token LightAPI{"LightAPI"};
      Token LightFilter{"LightFilter"};
      Token LightListAPI{"LightListAPI"};
      Token ListAPI{"ListAPI"};
      Token MeshLightAPI{"MeshLightAPI"};
      Token NonboundableLightBase{"NonboundableLightBase"};
      Token PluginLight{"PluginLight"};
      Token PluginLightFilter{"PluginLightFilter"};
      Token PortalLight{"PortalLight"};
      Token RectLight{"RectLight"};
      Token ShadowAPI{"ShadowAPI"};
      Token ShapingAPI{"ShapingAPI"};
      Token SphereLight{"SphereLight"};
      Token VolumeLightAPI{"VolumeLightAPI"};
    } types;
    // --(BEGIN CUSTOM CODE)--
    // --(END CUSTOM CODE)--
  };

  extern USG_API LuxTokensDef LuxTokens;
}
}
#endif
