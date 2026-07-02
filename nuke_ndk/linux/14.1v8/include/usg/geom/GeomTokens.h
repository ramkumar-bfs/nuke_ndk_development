// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

// This is automatically-generated code.
// You may add custom code between the --(BEGIN CUSTOM CODE)-- and --(END CUSTOM CODE)-- markers.
// Code added anywhere else will be destroyed when the code is generated.

#ifndef USG_GEOM_GEOMTOKENS_H
#define USG_GEOM_GEOMTOKENS_H

#include "usg/api.h"
#include "usg/base/Token.h"

namespace usg {
  struct USG_API GeomTokensDef {
    Token X{"X"};
    Token Y{"Y"};
    Token Z{"Z"};
    Token accelerations{"accelerations"};
    Token all{"all"};
    Token angularVelocities{"angularVelocities"};
    Token axis{"axis"};
    Token basis{"basis"};
    Token bezier{"bezier"};
    Token bilinear{"bilinear"};
    Token boundaries{"boundaries"};
    Token bounds{"bounds"};
    Token box{"box"};
    Token bspline{"bspline"};
    Token cards{"cards"};
    Token catmullClark{"catmullClark"};
    Token catmullRom{"catmullRom"};
    Token clippingPlanes{"clippingPlanes"};
    Token clippingRange{"clippingRange"};
    Token closed{"closed"};
    Token cornerIndices{"cornerIndices"};
    Token cornerSharpnesses{"cornerSharpnesses"};
    Token cornersOnly{"cornersOnly"};
    Token cornersPlus1{"cornersPlus1"};
    Token cornersPlus2{"cornersPlus2"};
    Token creaseIndices{"creaseIndices"};
    Token creaseLengths{"creaseLengths"};
    Token creaseSharpnesses{"creaseSharpnesses"};
    Token cross{"cross"};
    Token cubic{"cubic"};
    Token curveVertexCounts{"curveVertexCounts"};
    Token default_{"default"};
    Token doubleSided{"doubleSided"};
    Token edgeAndCorner{"edgeAndCorner"};
    Token edgeOnly{"edgeOnly"};
    Token elementType{"elementType"};
    Token exposure{"exposure"};
    Token extent{"extent"};
    Token fStop{"fStop"};
    Token face{"face"};
    Token faceVaryingLinearInterpolation{"faceVaryingLinearInterpolation"};
    Token faceVertexCounts{"faceVertexCounts"};
    Token faceVertexIndices{"faceVertexIndices"};
    Token familyName{"familyName"};
    Token focalLength{"focalLength"};
    Token focusDistance{"focusDistance"};
    Token fromTexture{"fromTexture"};
    Token guide{"guide"};
    Token guideVisibility{"guideVisibility"};
    Token height{"height"};
    Token holeIndices{"holeIndices"};
    Token horizontalAperture{"horizontalAperture"};
    Token horizontalApertureOffset{"horizontalApertureOffset"};
    Token ids{"ids"};
    Token indices{"indices"};
    Token inherited{"inherited"};
    Token interpolateBoundary{"interpolateBoundary"};
    Token invisible{"invisible"};
    Token invisibleIds{"invisibleIds"};
    Token knots{"knots"};
    Token left{"left"};
    Token leftHanded{"leftHanded"};
    Token linear{"linear"};
    Token loop{"loop"};
    Token modelApplyDrawMode{"model:applyDrawMode"};
    Token modelCardGeometry{"model:cardGeometry"};
    Token modelCardTextureXNeg{"model:cardTextureXNeg"};
    Token modelCardTextureXPos{"model:cardTextureXPos"};
    Token modelCardTextureYNeg{"model:cardTextureYNeg"};
    Token modelCardTextureYPos{"model:cardTextureYPos"};
    Token modelCardTextureZNeg{"model:cardTextureZNeg"};
    Token modelCardTextureZPos{"model:cardTextureZPos"};
    Token modelDrawMode{"model:drawMode"};
    Token modelDrawModeColor{"model:drawModeColor"};
    Token mono{"mono"};
    Token motionBlurScale{"motion:blurScale"};
    Token motionNonlinearSampleCount{"motion:nonlinearSampleCount"};
    Token motionVelocityScale{"motion:velocityScale"};
    Token none{"none"};
    Token nonperiodic{"nonperiodic"};
    Token normals{"normals"};
    Token open{"open"};
    Token order{"order"};
    Token orientation{"orientation"};
    Token orientations{"orientations"};
    Token origin{"origin"};
    Token orthographic{"orthographic"};
    Token periodic{"periodic"};
    Token perspective{"perspective"};
    Token pinned{"pinned"};
    Token pointWeights{"pointWeights"};
    Token points{"points"};
    Token positions{"positions"};
    Token primvarsDisplayColor{"primvars:displayColor"};
    Token primvarsDisplayOpacity{"primvars:displayOpacity"};
    Token primvarsNormals{"primvars:normals"};
    Token primvarsSt{"primvars:st"};
    Token primvarsSt_0{"primvars:st_0"};
    Token primvarsSt_1{"primvars:st_1"};
    Token primvarsStw{"primvars:stw"};
    Token primvarsStw_0{"primvars:stw_0"};
    Token primvarsStw_1{"primvars:stw_1"};
    Token primvarsUv{"primvars:uv"};
    Token primvarsWidths{"primvars:widths"};
    Token projection{"projection"};
    Token protoIndices{"protoIndices"};
    Token prototypes{"prototypes"};
    Token proxy{"proxy"};
    Token proxyPrim{"proxyPrim"};
    Token proxyVisibility{"proxyVisibility"};
    Token purpose{"purpose"};
    Token radius{"radius"};
    Token ranges{"ranges"};
    Token render{"render"};
    Token renderVisibility{"renderVisibility"};
    Token right{"right"};
    Token rightHanded{"rightHanded"};
    Token scales{"scales"};
    Token shutterClose{"shutter:close"};
    Token shutterOpen{"shutter:open"};
    Token size{"size"};
    Token smooth{"smooth"};
    Token stereoRole{"stereoRole"};
    Token subdivisionScheme{"subdivisionScheme"};
    Token tangents{"tangents"};
    Token triangleSubdivisionRule{"triangleSubdivisionRule"};
    Token trimCurveCounts{"trimCurve:counts"};
    Token trimCurveKnots{"trimCurve:knots"};
    Token trimCurveOrders{"trimCurve:orders"};
    Token trimCurvePoints{"trimCurve:points"};
    Token trimCurveRanges{"trimCurve:ranges"};
    Token trimCurveVertexCounts{"trimCurve:vertexCounts"};
    Token type{"type"};
    Token uForm{"uForm"};
    Token uKnots{"uKnots"};
    Token uOrder{"uOrder"};
    Token uRange{"uRange"};
    Token uVertexCount{"uVertexCount"};
    Token vForm{"vForm"};
    Token vKnots{"vKnots"};
    Token vOrder{"vOrder"};
    Token vRange{"vRange"};
    Token vVertexCount{"vVertexCount"};
    Token velocities{"velocities"};
    Token verticalAperture{"verticalAperture"};
    Token verticalApertureOffset{"verticalApertureOffset"};
    Token visibility{"visibility"};
    Token visible{"visible"};
    Token widths{"widths"};
    Token wrap{"wrap"};
    Token xformOpOrder{"xformOpOrder"};
    struct {
      Token BasisCurves{"BasisCurves"};
      Token Boundable{"Boundable"};
      Token Camera{"Camera"};
      Token Capsule{"Capsule"};
      Token Cone{"Cone"};
      Token Cube{"Cube"};
      Token Curves{"Curves"};
      Token Cylinder{"Cylinder"};
      Token GeomModelAPI{"GeomModelAPI"};
      Token GeomSubset{"GeomSubset"};
      Token Gprim{"Gprim"};
      Token HermiteCurves{"HermiteCurves"};
      Token Imageable{"Imageable"};
      Token Mesh{"Mesh"};
      Token MotionAPI{"MotionAPI"};
      Token NurbsCurves{"NurbsCurves"};
      Token NurbsPatch{"NurbsPatch"};
      Token PointBased{"PointBased"};
      Token PointInstancer{"PointInstancer"};
      Token Points{"Points"};
      Token PrimvarsAPI{"PrimvarsAPI"};
      Token Scope{"Scope"};
      Token Sphere{"Sphere"};
      Token VisibilityAPI{"VisibilityAPI"};
      Token Xform{"Xform"};
      Token XformCommonAPI{"XformCommonAPI"};
      Token Xformable{"Xformable"};
    } types;
    // --(BEGIN CUSTOM CODE)--
    Token colorAov{"color"};
    Token depthAov{"depth"};
    Token cameraDepthAov{"cameraDepth"};
    //
    Token primIdAov{"primId"};
    Token instanceIdAov{"instanceId"};
    Token elementIdAov{"elementId"};
    Token edgeIdAov{"edgeId"};
    Token pointIdAov{"pointId"};
    //
    Token PeyeAov{"Peye"};
    Token NeyeAov{"Neye"};
    Token patchCoordAov{"patchCoord"};
    Token primitiveParamAov{"primitiveParam"};
    Token normalAov{"normal"};

    // Render/shading
    Token usgSchema{"UsgSchema"};
    Token usgShader{"UsgShader"};

    Token universalRenderContext{""};
    Token targetRenderers{"targetRenderers"};
    Token glslfxRenderContext{"glslfx"};
    Token mtlxRenderContext{"mtlx"};
    Token slrRenderContext{"slr"};

    Token universalSourceType{""};
    Token glslfxSourceType{"glslfx"};
    Token customGlslfxSourceType{"customGlslfx"};
    Token slrSourceType{"slr"};

    // Property connectivity
    Token connectability{"connectability"};
    struct {
      Token full{"full"};
      Token interfaceOnly{"interfaceOnly"};
    } connectivity;

    // Attribute interpolation
    Token interpolation{"interpolation"};

    Token constant{"constant"};
    Token uniform{"uniform"};
    Token varying{"varying"};
    Token vertex{"vertex"};
    Token faceVarying{"faceVarying"};

    Token st{"st"};
    Token stw{"stw"};
    Token color{"color"};
    Token opacity{"opacity"};

    struct {
      Token binding{"material:binding"};
    } material;

    // USG-specific tokens
    struct {
      Token owner{"usg:owner"};
      Token modified{"usg:modified"};
      Token displayMaterials{"usg:displayMaterials"};
      Token localProp{"usg:localProp"};
    } usg;
    // --(END CUSTOM CODE)--
  };

  extern USG_API GeomTokensDef GeomTokens;
}
#endif
