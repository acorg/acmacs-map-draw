#pragma once

#include <string>
#include <limits>

#include "acmacs-draw/surface.hh"
#include "acmacs-chart/layout.hh"

// ----------------------------------------------------------------------

class Surface;

#include "acmacs-base/global-constructors-push.hh"

const Aspect AspectRegular{1.0};
const Aspect AspectEgg{0.75};
const Aspect AspectNoChange{std::numeric_limits<double>::quiet_NaN()};

const Rotation RotationRegular{0.0};
const Rotation RotationReassortant{0.5};
const Rotation RotationNoChange{std::numeric_limits<double>::quiet_NaN()};

#include "acmacs-base/diagnostics-pop.hh"

// ----------------------------------------------------------------------

class PointStyle
{
 public:
    enum class Shown { Hidden, Shown, NoChange };
    enum class Shape { NoChange, Circle, Box, Triangle };
    enum Empty { Empty };

    inline PointStyle()
        : mShown(Shown::Shown), mShape(Shape::Circle), mFill("green"), mOutline("black"),
          mSize(5), mOutlineWidth(1), mAspect(AspectRegular), mRotation(RotationRegular) {}
    inline PointStyle(enum Empty)
        : mShown(Shown::NoChange), mShape(Shape::NoChange), mFill(ColorNoChange), mOutline(ColorNoChange),
          mSize(Pixels::make_empty()), mOutlineWidth(Pixels::make_empty()), mAspect(AspectNoChange), mRotation(RotationNoChange) {}
    PointStyle& operator = (const PointStyle& aPS);

    void draw(Surface& aSurface, const Coordinates& aCoord);

    inline PointStyle& show(Shown aShown = Shown::Shown) { mShown = aShown; return *this; }
    inline PointStyle& hide() { mShown = Shown::Hidden; return *this; }
    inline PointStyle& shape(Shape aShape) { mShape = aShape; return *this; }
    inline PointStyle& fill(Color c) { mFill = c; return *this; }
    inline PointStyle& outline(Color c) { mOutline = c; return *this; }
    inline PointStyle& size(Pixels aSize) { mSize = aSize; return *this; }
    inline PointStyle& outline_width(Pixels aOutlineWidth) { mOutlineWidth = aOutlineWidth; return *this; }
    inline PointStyle& aspect(Aspect aAspect) { mAspect = aAspect; return *this; }
    inline PointStyle& aspect(double aAspect) { mAspect = aAspect; return *this; }
    inline PointStyle& rotation(Rotation aRotation) { mRotation = aRotation; return *this; }
    inline PointStyle& rotation(double aRotation) { mRotation = aRotation; return *this; }

    inline PointStyle& scale(double aScale) { mSize *= aScale; return *this; }
    inline PointStyle& scale_outline(double aScale) { mOutlineWidth *= aScale; return *this; }

 private:
    Shown mShown;
    Shape mShape;
    Color mFill;
    Color mOutline;
    Pixels mSize;
    Pixels mOutlineWidth;
    Aspect mAspect;
    Rotation mRotation;

}; // class PointStyle

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
