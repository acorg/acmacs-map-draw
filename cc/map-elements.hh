#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>

#include "acmacs-base/size-scale.hh"
#include "acmacs-base/color.hh"
#include "acmacs-base/text-style.hh"
#include "acmacs-base/line.hh"
#include "acmacs-draw/surface.hh"

// ----------------------------------------------------------------------

namespace acmacs::surface { class Surface; }
namespace acmacs::draw { class DrawElements; }
class ChartDraw;

// ----------------------------------------------------------------------

namespace map_elements
{
    class Element;

    class Elements
    {
     public:
        enum Order { BeforePoints, AfterPoints };

        Elements();
        Element& operator[](std::string aKeyword);

        void remove(std::string aKeyword);
        bool exists(std::string aKeyword) const;

        Element& add(std::string_view aKeyword);

        template <typename Elt> Elt& add()
        {
            auto ptr = std::make_unique<Elt>();
            auto& elt = *ptr;
            mElements.push_back(std::move(ptr));
            return elt;
        }

        void draw(acmacs::surface::Surface& aSurface, Order aOrder, const ChartDraw& aChartDraw) const;
        void draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const;

     private:
        std::vector<std::shared_ptr<Element>> mElements;

        void add_basic_elements_v1();
        bool add_v1(std::string_view aKeyword);

    }; // class Elements

// ----------------------------------------------------------------------

    class Element
    {
     public:
        Element(std::string aKeyword, Elements::Order aOrder) : mKeyword(aKeyword), mOrder(aOrder) {}
        Element(const Element&) = default;
        virtual ~Element() = default;

        std::string keyword() const { return mKeyword; }
        Elements::Order order() const { return mOrder; }
        virtual void draw(acmacs::surface::Surface& /* aSurface*/, const ChartDraw& /*aChartDraw*/) const {} // obsolete
        virtual void draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const = 0; // { std::cerr << "WARNING: map_elements::Element::draw " << typeid(this).name() << '\n'; }

     protected:
        virtual acmacs::PointCoordinates subsurface_origin(acmacs::surface::Surface& aSurface, const acmacs::PointCoordinates& aPixelOrigin, const acmacs::Size& aScaledSubsurfaceSize) const;

        void keyword(std::string aKeyword) { mKeyword = aKeyword; }

     private:
        std::string mKeyword;
        Elements::Order mOrder;
    };

} // namespace map_elements


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
