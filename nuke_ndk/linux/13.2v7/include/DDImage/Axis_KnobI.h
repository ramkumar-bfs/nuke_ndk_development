// Copyright (c) 2010 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DD_Image_Axis_KnobI_H
#define DD_Image_Axis_KnobI_H

#include "DDImage/Vector3.h"
#include "DDImage/Matrix4.h"

namespace DD
{

  namespace Image
  {
    class OutputContext;

    class DDImage_API Axis_KnobI
    {
    public:
      enum TransformOrder { 
        eSRT = 0, 
        eSTR, 
        eRST, 
        eRTS, 
        eTSR, 
        eTRS 
      };

      enum RotationOrder { 
        eXYZ = 0, 
        eXZY, 
        eYXZ, 
        eYZX, 
        eZXY, 
        eZYX 
      };

      enum Transformation {
        eTranslate = 0,
        eRotate,
        eScale,
        eUniformScale,
        eSkew,
        ePivot,
        ePivotRotate
      };

      Axis_KnobI() { }

      virtual ~Axis_KnobI() { }

      virtual void setTranslate(const Vector3& translation) = 0;
      virtual void setRotate(const Vector3& rotate) = 0;
      virtual void setScale(const Vector3& scale) = 0;
      virtual void setPivot(const Vector3& pivot_translation) = 0;
      virtual void setPivotRotate(const Vector3& pivot_rotate) = 0;
      virtual void setSkew(const Vector3& skew) = 0;
      virtual void setUniformScale(double uniformScale) = 0;
      virtual void setRotationOrder(RotationOrder order) = 0;
      virtual void setTransformOrder(TransformOrder order) = 0;

      //! @brief Forces AxisKnob to always be drawn in node select mode
      //! independently on the current selection mode on the viewer.
      //! @param value true to enable, false otherwise.
      virtual void setAxisVisibleForAllSelectionModes(bool visible) = 0;

      virtual Vector3 getTranslate(const OutputContext& context) const = 0;
      virtual Vector3 getRotate(const OutputContext& context) const = 0;
      virtual Vector3 getScale(const OutputContext& context) const = 0;
      virtual Vector3 getPivot(const OutputContext& context) const = 0;
      virtual Vector3 getPivotRotate(const OutputContext& context) const = 0;
      virtual Vector3 getSkew(const OutputContext& context) const = 0;
      virtual double getUniformScale(const OutputContext& context) const = 0;
      virtual RotationOrder getRotationOrder(const OutputContext& context) const = 0;
      virtual TransformOrder getTransformOrder(const OutputContext& context) const = 0;

      //! @brief Returns true if this AxisKnob instance is always drawn in
      //! independently on the current selection mode on the viewer.
      //! @return true if enabled, false otherwise.
      virtual bool isAxisVisibleForAllSelectionModes() const = 0;

      virtual bool hasExpression(Transformation knob) const = 0;

      virtual const DD::Image::Matrix4& matrix(const DD::Image::OutputContext&) const = 0;

      virtual void hide() = 0;
      virtual void show() = 0;

      static const TransformOrder kDefaultTransformOrder = eSRT;
      static const RotationOrder kDefaultRotationOrder = eZXY;
    };

  }
}

#endif
