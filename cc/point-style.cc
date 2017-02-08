#include "point-style.hh"

// ----------------------------------------------------------------------

PointStyle& PointStyle::operator = (const PointStyle& aPS)
{
    if (aPS.mShown != Shown::NoChange)
        mShown = aPS.mShown;
    if (aPS.mShape != Shape::NoChange)
        mShape = aPS.mShape;
    if (!aPS.mFill.empty())
        mFill = aPS.mFill;
    if (!aPS.mOutline.empty())
        mOutline = aPS.mOutline;
    if (!aPS.mSize.empty())
        mSize = aPS.mSize;
    if (!aPS.mOutlineWidth.empty())
        mOutlineWidth = aPS.mOutlineWidth;
    if (!aPS.mAspect.empty())
        mAspect = aPS.mAspect;
    if (!aPS.mRotation.empty())
        mRotation = aPS.mRotation;

    return *this;

} // PointStyle::operator =

// ----------------------------------------------------------------------

void PointStyle::draw(Surface& aSurface, const Coordinates& aCoord)
{
    if (mShown == Shown::Shown && !aCoord.empty()) {
        switch (mShape) {
          case Shape::NoChange:
              throw std::runtime_error("Invalid point shape NoChange");
          case Shape::Circle:
              aSurface.circle_filled(aCoord, mSize, mAspect, mRotation, mOutline, mOutlineWidth, mFill);
              break;
          case Shape::Box:
              aSurface.square_filled(aCoord, mSize, mAspect, mRotation, mOutline, mOutlineWidth, mFill);
              break;
          case Shape::Triangle:
              aSurface.triangle_filled(aCoord, mSize, mAspect, mRotation, mOutline, mOutlineWidth, mFill);
              break;
        }
    }
    else if (mShown == Shown::NoChange)
        throw std::runtime_error("Invalid shown value NoChange");

} // PointStyle::draw

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
