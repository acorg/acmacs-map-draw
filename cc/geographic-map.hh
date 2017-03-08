#pragma once

#include <string>
class Surface;

// ----------------------------------------------------------------------

class GeographicMapDraw
{
 public:
    inline GeographicMapDraw() = default;

    void prepare();
    void draw(Surface& aSurface);
    void draw(std::string aFilename);

}; // class GeographicMapDraw

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
